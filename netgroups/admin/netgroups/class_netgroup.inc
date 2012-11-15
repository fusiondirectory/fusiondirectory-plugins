<?php

/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * and  compatible with GOsa 2.6 (http://www.gosa-project.org)
 * Copyright (C) 2011 Alejandro Escanero Blanco (aescanero@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

class netgroup extends plugin {
    /* Group attributes */

    var $cn = "";
    var $description = "";

    var $netgroups = array();
    var $netgroups_used_by_some = array();
    var $triples = array();
    var $triples_used_by_some = array();
    var $allusers = array();
    var $users_used_by_some = array();

    var $nisNetgroupTriple = array();
    var $memberNisNetgroup = array();

    /* Helpers */
    var $base = "";
    var $members = array();
    var $users = array();

    var $orig_dn = "";
    var $orig_cn = "";
    var $orig_base = "";
    var $userSelect = FALSE;
    var $dialog;
    var $OnlyShowFirstEntries = 200;
    var $dnMapping = array();
    var $view_logged = FALSE;
    var $allowNetgroupsWithSameNameInOtherSubtrees = true;
    var $baseSelector;

    /* attribute list for save action */
    var $attributes = array("cn", "description", "nisNetgroupTriple", "memberNisNetgroup");
    var $objectclasses = array("top", "nisNetgroup");
    var $CopyPasteVars = array("");
    var $multiple_support = TRUE;


    /* Constructor */

    function netgroup(&$config, $dn= NULL) {

        plugin::plugin($config, $dn);

        $this->orig_dn = $dn;
        $this->orig_cn = $this->cn;

        /* Get triples list and put it in $this->triples */
        //TODO: domain part left to next versions
        if (isset($this->attrs['nisNetgroupTriple'][0])) {
            $tmp = array();
            for ($i = 0; $i < $this->attrs['nisNetgroupTriple']['count']; $i++) {
                if (preg_match("/^\((\S+),\-?,(\S*)\)$/", $this->attrs['nisNetgroupTriple'][$i], $matches)) {
                    //Removing host.domain entries
                    if(strpos($matches[1],".")===false){
                        $tmp[$matches[1]] = $this->attrs['nisNetgroupTriple'][$i];
                    }
                }
                if (preg_match("/^\(\-?,(\S+),(\S*)\)$/", $this->attrs['nisNetgroupTriple'][$i], $matches)) {
                    $tmp[$matches[1]] = $this->attrs['nisNetgroupTriple'][$i];
                }
            }
            $this->triples = $tmp;
            ksort($this->triples);
        }

        /* Get netgroups list  and put it in $this->netgroups */
        if (isset($this->attrs['memberNisNetgroup'][0])) {
            $tmp = array();
            for ($i = 0; $i < $this->attrs['memberNisNetgroup']['count']; $i++) {
                $tmp[$this->attrs['memberNisNetgroup'][$i]] = $this->attrs['memberNisNetgroup'][$i];
            }
            $this->netgroups = $tmp;
            ksort($this->netgroups);
        }


        /* Get global filter config */
        if (!session::is_set("nngufilter")) {
            $ui = get_userinfo();
            $base = get_base_from_people($ui->dn);
            $nngufilter = array("dselect" => $base,
                "regex" => "*");
            session::set("nngufilter", $nngufilter);
        }
        $nngufilter = session::get('nngufilter');
        $nngufilter['SubSearchGroup'] = false;
        session::set('nngufilter', $nngufilter);

        if ($this->dn == "new") {
            if (session::is_set('CurrentMainBase')) {
                $this->base = session::get('CurrentMainBase');
            } else {
                $ui = get_userinfo();
                $this->base = dn2base($ui->dn);
            }
        } else {

            /* Get object base */
            $this->base = preg_replace("/^[^,]+," . preg_quote(get_ou("netgroupRDN"), '/') . "/i", "", $this->dn);
        }
        $this->orig_base = $this->base;

        $this->accessTo = array();
        if (isset($this->attrs['accessTo'])) {
            for ($i = 0; $i < $this->attrs['accessTo']['count']; $i++) {
                $tmp = $this->attrs['accessTo'][$i];
                $this->accessTo[$tmp] = $tmp;
            }
        }

        /* Get global filter config */
        if (!session::is_set("sysfilter")) {
            $ui = get_userinfo();
            $base = get_base_from_people($ui->dn);
            $sysfilter = array("depselect" => $base,
                "regex" => "*");
            session::set("sysfilter", $sysfilter);
        }

        /* This is always an account */
        $this->is_account = TRUE;

        /* Instanciate base selector */
        $this->baseSelector = new baseSelector($this->get_allowed_bases(), $this->base);
        $this->baseSelector->setSubmitButton(false);
        $this->baseSelector->setHeight(300);
        $this->baseSelector->update(true);

        $this->reload(TRUE);
    }

    function execute() {
        /* Call parent execute */
        plugin::execute();

        /* Log view */
        if ($this->is_account && !$this->view_logged) {
            $this->view_logged = TRUE;
            new log("view", "netgroups/" . get_class($this), $this->dn);
        }

        /* Do we represent a valid netgroup? */
        if (!$this->is_account && $this->parent === NULL) {
            $display = "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>" . msgPool::noValidExtension() . "</b>";
            return ($display);
        }

        /* Delete user from netgroup */
        if (isset($_POST['del_users']) && isset($_POST['members']) && preg_match("/w/", $this->getacl("triples"))) {
            foreach ($_POST['members'] as $value) {
                unset($this->members["$value"]);
                $this->removeUser($value);
            }
            $this->reload();
        }

        /* Add objects? */
        if (isset($_POST["edit_membership"]) && preg_match("/w/", $this->getacl("triples"))) {
            $this->userSelect = new tripleSelect($this->config, get_userinfo());
            $this->dialog = TRUE;
        }

        /* Add objects finished? */
        if (isset($_POST["add_triples_cancel"])) {
            $this->userSelect = NULL;
            $this->dialog = FALSE;
        }

        /* Add to netgroup */
        if (isset($_POST['add_triples_finish']) && $this->userSelect) {

            /* Get all the dn from userSelect */
            $users = $this->userSelect->detectPostActions();
            if (isset($users['targets'])) {
                $headpage = $this->userSelect->getHeadpage();
                foreach ($users['targets'] as $dn) {
                    $attrs = $headpage->getEntry($dn);
                    $value = "";
                    $objectClass = "posixAccount";
                    for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
                        if ($attrs['objectClass'][$i] == "GOhard") {
                            $value = $attrs['cn'][0];
                            $objectClass = $attrs['objectClass'][$i];
                        } else if (($attrs['objectClass'][$i] == "nisNetgroup") || ($attrs['objectClass'][$i] == "goServer") || ($attrs['objectClass'][$i] == "gotoWorkstation") || ($attrs['objectClass'][$i] == "GOhard") || ($attrs['objectClass'][$i] == "ieee802Device")) {
                            $value = $attrs['cn'][0];
                            $objectClass = $attrs['objectClass'][$i];
                            break;
                        } else if (($attrs['objectClass'][$i] == "posixAccount")) {
                            $value = $attrs['uid'][0];
                            break;
                        }
                    }
                    $this->addUser($value, $objectClass,$dn);
//TODO: Check the next line
//                    $this->members["$value"] = $this->allusers[$value];
                    $this->reload();
                }
            }
            $this->userSelect = NULL;
            $this->dialog = FALSE;
        }

        /* Dialog handling */
        if (is_object($this->dialog)) {
            /* Must be called before save_object */
            $this->dialog->save_object();

            if ($this->dialog->isClosed()) {
                $this->dialog = false;
            } elseif ($this->dialog->isSelected()) {

                /* Check if selected base is valid */
                $tmp = $this->get_allowed_bases();
                if (isset($tmp[$this->dialog->isSelected()])) {
                    $this->base = $this->dialog->isSelected();
                }
                $this->dialog = false;
            } else {
                return($this->dialog->execute());
            }
        }

        $smarty = get_smarty();
        $smarty->assign("usePrototype", "true");

        /* Manage object add dialog */
        if ($this->userSelect) {
            return($this->userSelect->execute());
        }

        /* Create base acls */
        $smarty->assign("base", $this->baseSelector->render());

        /* Members and users */
        $smarty->assign("members", $this->members);

        /* Fields */
        foreach (array("cn", "description") as $val) {
            $smarty->assign("$val", $this->$val);
        }

        $tmp = $this->plInfo();
        foreach ($tmp['plProvidedAcls'] as $name => $translation) {
            $smarty->assign($name . "ACL", $this->getacl($name));
        }

        if ($this->acl_is_writeable("base")) {
            $smarty->assign("baseSelect", true);
        } else {
            $smarty->assign("baseSelect", false);
        }

        /* Show main page */
        $smarty->assign("alphabet", generate_alphabet(10));
        $smarty->assign("search_image", get_template_path('images/lists/search.png'));
        $smarty->assign("launchimage", get_template_path('images/lists/action.png'));
        $smarty->assign("tree_image", get_template_path('images/lists/search-subtree.png'));
        $smarty->assign("deplist", $this->config->idepartments);

        /* Multiple edit handling */
        $smarty->assign("multiple_support", $this->multiple_support_active);

/*        $tmp_members=array();
        $tmp_members_used_by_some=array();

        foreach ($this->triples_used_by_some as $key => $value) {
            $tmp_members_used_by_some[$key]=$value;
        }
        foreach ($this->netgroups_used_by_some as $key => $value) {
            $tmp_members_used_by_some[$key]=$value;
        }
  */
        $smarty->assign("memberCn_All", $this->allusers);
        $smarty->assign("memberCn_Some", $this->users_used_by_some);

        foreach ($this->attributes as $val) {
            if (in_array($val, $this->multi_boxes)) {
                $smarty->assign("use_" . $val, TRUE);
            } else {
                $smarty->assign("use_" . $val, FALSE);
            }
        }
        foreach (array("base") as $val) {
            if (in_array($val, $this->multi_boxes)) {
                $smarty->assign("use_" . $val, TRUE);
            } else {
                $smarty->assign("use_" . $val, FALSE);
            }
        }

        return($smarty->fetch(get_template_path('generic.tpl', TRUE)));
    }

    function addUser($uid, $objectClass, $dn) {
        /* In mutliple edit we have to handle two arrays.
         *  triples               : Containing users used in all netgroups
         *  triples_used_by_some  : Those which are not used in all netgroups
         * So we have to remove the given $uid from the ..used_by_some array first.
         */
        if ($this->multiple_support_active) {
            if (isset($this->netgroups_used_by_some[$uid])) {
                unset($this->netgroups_used_by_some[$uid]);
                unset($this->dnMapping[$uid]);
                unset($this->users_used_by_some[$uid]);
            }
            if (isset($this->triples_used_by_some[$uid])) {
                unset($this->triples_used_by_some[$uid]);
                unset($this->dnMapping[$uid]);
                unset($this->users_used_by_some[$uid]);
            }
        }

        /* Ensure that the requested object is known to the netgroup class
         */
        if (!isset($this->dnMapping[$uid])) {
            $ldap = $this->config->get_ldap_link();
            $ldap->cd($this->config->current['BASE']);
            $typesearch = "uid";
            if ($objectClass == "nisNetgroup" || $objectClass == "GOhard" || $objectClass == "ieee802Device" || $objectClass == "goServer" || $objectClass == "gotoWorkstation")
                $typesearch = "cn";
            $ldap->cat($dn, array("dn", $typesearch, "sn", "givenName", "description"));
            if ($ldap->count()==0) {
                msg_dialog::display(_("Error"),
                                sprintf(_("Add CN/UID '%s' to NIS netgroup '%s' failed: cannot find object!"),
                                        $uid, $this->cn),
                                ERROR_DIALOG);
                return;
            } else {
                while ($attrs = $ldap->fetch()) {
                    $this->dnMapping[$attrs[$typesearch][0]] = $attrs['dn'];
                    $this->members[$attrs[$typesearch][0]] = $this->createResultName($attrs, $objectClass);
                    $this->allusers[$attrs[$typesearch][0]] = $this->createResultName($attrs, $objectClass);
                }
            }
        }

        if ($objectClass == "GOhard" || $objectClass == "ieee802Device" || $objectClass == "goServer" || $objectClass == "gotoWorkstation") {
            $this->triples[$uid] = "(" . $uid . ",,)";
        } else if ($objectClass == "posixAccount") {
            $this->triples[$uid] = "(," . $uid . ",)";
        } else {
            $this->netgroups[$uid] = $uid;
        }
    }

    function removeUser($uid) {
        $temp = array();
        if (isset($this->triples[$uid])) {
            unset($this->triples[$uid]);
            unset($this->dnMapping[$uid]);
            unset($this->allusers[$uid]);
        }
        if (isset($this->netgroups[$uid])) {
            unset($this->netgroups[$uid]);
            unset($this->dnMapping[$uid]);
            unset($this->allusers[$uid]);
        }

        /* We have two array contianing netgroup members in multiple edit.
         *  this->triples             : Netgroups used by all currently edited netgroups
         *  this->triples_used_by_some: Used by some
         * So we have to remove the specified uid from both arrays.
         */
        if ($this->multiple_support_active) {
            if (isset($this->triples_used_by_some[$uid])) {
                unset($this->triples_used_by_some[$uid]);
                unset($this->dnMapping[$uid]);
                unset($this->users_used_by_some[$uid]);
            }
            if (isset($this->netgroups_used_by_some[$uid])) {
                unset($this->netgroups_used_by_some[$uid]);
                unset($this->dnMapping[$uid]);
                unset($this->users_used_by_some[$uid]);
            }
        }
    }

    /* Reload data and put in the list */

    function reload($silent = FALSE) {

        /* Fix regex string */
        $nngufilter = session::get("nngufilter");
        $regex = normalizeLdap($nngufilter['regex']);
        $MaxUser = $this->OnlyShowFirstEntries;

/*        if ($this->multiple_support_active) {
            $this->allusers=array();
            $this->users_used_by_some=array();
        }
  */
        /* Prepare ldap link */
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($nngufilter['dselect']);

        /* Create a filter with all the triples that no appear in $this->members */
        $triplesfilter = "";
        if ($this->config->get_cfg_value("ldapFilterNestingLimit") == "" ||
                count($this->triples) < $this->config->get_cfg_value("ldapFilterNestingLimit")) {
            foreach (array_keys($this->triples) as $value) {
                if (!isset($this->members[$value])) {
                    $triplesfilter .= "(|(cn=" . normalizeLdap($value) . ")(uid=" . normalizeLdap($value) . "))";
                }
            }
        }

        /* Create a filter with all the netgroups that no appear in $this->members */
        $netgroupfilter = "";
        if ($this->config->get_cfg_value("ldapFilterNestingLimit") == "" ||
                count($this->netgroups) < $this->config->get_cfg_value("ldapFilterNestingLimit")) {
            foreach (array_keys($this->netgroups) as $value) {
                if (!isset($this->members[$value])) {
                    $netgroupfilter .= "(cn=" . normalizeLdap($value) . ")";
                }
            }
        }

        /* Search in all the tree for the component of this triple */
        if (!empty($triplesfilter)) {
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(&(|(objectClass=posixAccount)(objectClass=GOhard)(objectClass=ieee802Device))(|" . $triplesfilter . "))", array("dn", "cn", "uid", "sn", "givenName", "description", "objectClass"));
            while ($attrs = $ldap->fetch()) {
                $selector = "uid";
                $objectClass = "posixAccount";
                for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
                    if ($attrs['objectClass'][$i] == "goServer") {
                        $selector = "cn";
                        $objectClass = "goServer";
                        break;
                    }
                    if ($attrs['objectClass'][$i] == "gotoWorkstation") {
                        $selector = "cn";
                        $objectClass = "gotoWorkstation";
                        break;
                    }
                    if ($attrs['objectClass'][$i] == "GOhard") {
                        $selector = "cn";
                        $objectClass = "GOhard";
                    }
                    if ($attrs['objectClass'][$i] == "ieee802Device") {
                        $selector = "cn";
                        $objectClass = "ieee802Device";
                        break;
                    }
                }

                $value = $attrs[$selector][0];
                $this->dnMapping[$value] = $attrs['dn'];
                $this->members[$value] = $this->createResultName($attrs, $objectClass);
                $this->allusers[$value] = $this->createResultName($attrs, $objectClass);
            }
        }

        if ($this->multiple_support_active) {
            $triplesfilter_used_by_some = "";
            if ($this->config->get_cfg_value("ldapFilterNestingLimit") == "" ||
                    count($this->triples_used_by_some) < $this->config->get_cfg_value("ldapFilterNestingLimit")) {
                foreach (array_keys($this->triples_used_by_some) as $value) {
                    if (!isset($this->members[$value])) {
                        $triplesfilter_used_by_some .= "(|(cn=" . normalizeLdap($value) . ")(uid=" . normalizeLdap($value) . "))";
                    }
                }
            }

            /* Search in all the tree for the component of this triple */
            if (!empty($triplesfilter_used_by_some)) {
                $ldap->cd($this->config->current['BASE']);
                $ldap->search("(&(|(objectClass=posixAccount)(objectClass=GOhard)(objectClass=ieee802Device))(|" . $triplesfilter_used_by_some . "))", array("dn", "cn", "uid", "sn", "givenName", "description", "objectClass"));
                while ($attrs = $ldap->fetch()) {
                    $selector = "uid";
                    $objectClass = "posixAccount";
                    for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
                        if ($attrs['objectClass'][$i] == "goServer") {
                            $selector = "cn";
                            $objectClass = "goServer";
                            break;
                        }
                        if ($attrs['objectClass'][$i] == "gotoWorkstation") {
                            $selector = "cn";
                            $objectClass = "gotoWorkstation";
                            break;
                        }
                        if ($attrs['objectClass'][$i] == "GOhard") {
                            $selector = "cn";
                            $objectClass = "GOhard";
                        }
                        if ($attrs['objectClass'][$i] == "ieee802Device") {
                            $selector = "cn";
                            $objectClass = "ieee802Device";
                            break;
                        }
                    }

                    $value = $attrs[$selector][0];
                    $this->dnMapping[$value] = $attrs['dn'];
                    $this->members[$value] = $this->createResultName($attrs, $objectClass);
                    $this->users_used_by_some[$value] = $this->createResultName($attrs, $objectClass);
                }

            }

            /* Create a filter with all the netgroups that no appear in $this->members */
            $netgroupfilter_used_by_some = "";
            if ($this->config->get_cfg_value("ldapFilterNestingLimit") == "" ||
                    count($this->netgroups_used_by_some) < $this->config->get_cfg_value("ldapFilterNestingLimit")) {
                foreach (array_keys($this->netgroups_used_by_some) as $value) {
                    if (!isset($this->members[$value])) {
                        $netgroupfilter_used_by_some .= "(cn=" . normalizeLdap($value) . ")";
                    }
                }
            }

            /* Search in all the tree for the component of this netgroup */
            if (!empty($netgroupfilter_used_by_some)) {
                $ldap->cd($this->config->current['BASE']);
                $ldap->search("(&(objectClass=nisNetgroup)(|" . $netgroupfilter_used_by_some . "))", array("dn", "cn", "description"));
                while ($attrs = $ldap->fetch()) {
                    $selector = "cn";
                    $value = $attrs[$selector][0];
                    $this->dnMapping[$value] = $attrs['dn'];
                    $this->members[$value] = $this->createResultName($attrs, "nisNetgroup");
                    $this->users_used_by_some[$value] = $this->createResultName($attrs, "nisNetgroup");
                }
            }

        }

        /* Search in all the tree for the component of this netgroup */
        if (!empty($netgroupfilter)) {
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(&(objectClass=nisNetgroup)(|" . $netgroupfilter . "))", array("dn", "cn", "description"));
            while ($attrs = $ldap->fetch()) {
                $selector = "cn";
                $value = $attrs[$selector][0];
                $this->dnMapping[$value] = $attrs['dn'];
                $this->members[$value] = $this->createResultName($attrs, "nisNetgroup");
                $this->allusers[$value] = $this->createResultName($attrs, "nisNetgroup");
            }
        }


        /* check if all uids or cn are in the ldap tree */
        if ($this->config->get_cfg_value("ldapFilterNestingLimit") == "" ||
                count($this->triples) < $this->config->get_cfg_value("ldapFilterNestingLimit") ||
                count($this->netgroups) < $this->config->get_cfg_value("ldapFilterNestingLimit")) {
            foreach (array_keys(array_merge($this->triples, $this->netgroups)) as $value) {
                if (!isset($this->members[$value])) {
                    $this->members[$value] = _("! unknown UID/CN") . " [" . $value . "]";
                }
            }
        } else {
            foreach (array_keys(array_merge($this->triples, $this->netgroups)) as $value) {
                $this->members[$value] = $value;
            }
        }

        /* Sort lists */
        natcasesort($this->members);
        reset($this->members);
    }

    /* Create display name, this was used so often that it is excluded into a seperate function */

    function createResultName($attrs, $objectClass) {
        $description = "";
        if (isset($attrs["description"][0]))
            $description = $attrs["description"][0] . ", ";
        if ($objectClass == "posixAccount" && isset($attrs["givenName"][0]) && isset($attrs["sn"][0])) {
            $ret = _("User") . ": " . $attrs["sn"][0] . ", " . $attrs["givenName"][0] . " [" . $attrs["uid"][0] . "]";
        } else if ($objectClass == "posixAccount") {
            $ret = _("User") . ": " . $description . "[" . $attrs["uid"][0] . "]";
        } else if ($objectClass == "goServer") {
            $ret = _("Server") . ": " . $description . "[" . $attrs["cn"][0] . "]";
        } else if ($objectClass == "gotoWorkstation") {
            $ret = _("Workstation") . ": " . $description . "[" . $attrs["cn"][0] . "]";
        } else if ($objectClass == "GOhard") {
            $ret = _("Host") . ": " . $description . "[" . $attrs["cn"][0] . "]";
        } else if ($objectClass == "ieee802Device") {
            $ret = _("Network Device") . ": " . $description . "[" . $attrs["cn"][0] . "]";
        } else {
            $ret = _("NIS Netgroup") . ": " . $description . "[" . $attrs["cn"][0] . "]";
        }
        return($ret);
    }

    function remove_from_parent() {
        plugin::remove_from_parent();

        $ldap = $this->config->get_ldap_link();
        $ldap->rmdir($this->dn);
        if (!$ldap->success()) {
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
        }

        new log("remove", "netgroups/" . get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());

        /* Delete references to object netgroups */
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(&(objectClass=gosaGroupOfNames)(memberNisNetgroup=" . LDAP::prepare4filter($this->dn) . "))", array("cn"));
        while ($ldap->fetch()) {
            $og = new ogroup($this->config, $ldap->getDN());
            unset($og->member[$this->dn]);
            $og->save();
        }

        /* Remove ACL dependencies too,
         */
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(&(objectClass=gosaAcl)(gosaAclEntry=*" . base64_encode($this->dn) . "*))", array("gosaAclEntry", "dn"));
        while ($attrs = $ldap->fetch()) {
            $acl = new acl($this->config, $this->parent, $attrs['dn']);
            foreach ($acl->gosaAclEntry as $id => $entry) {
                foreach ($entry['members'] as $m_id => $member) {
                    if ($m_id == "G:" . $this->dn || $m_id == "U:" . $this->dn) {
                        unset($acl->gosaAclEntry[$id]['members'][$m_id]);
                        fusiondirectory_log("modify", "netgroups/acl", $attrs['dn'], array(), sprintf("Removed acl for %s on object %s.", $this->dn, $attrs['dn']));
                    }
                }
            }
            $acl->save();
        }

        /* Remove ACL dependencies, too */
        acl::remove_acl_for($this->dn);

        /* Send signal to the world that we've done */
        $this->handle_post_events("remove");
    }

    /* Save data to object */

    function save_object() {
        /* Save additional values for possible next step */
        if (isset($_POST['nisnetgroupedit'])) {

            /* Create a base backup and reset the
              base directly after calling plugin::save_object();
              Base will be set seperatly a few lines below */
            $base_tmp = $this->base;
            plugin::save_object();
            $this->base = $base_tmp;

            /* Refresh base */
            if ($this->acl_is_moveable($this->base)) {
                if (!$this->baseSelector->update()) {
                    msg_dialog::display(_("Error"), msgPool::permMove(), ERROR_DIALOG);
                }
                if ($this->base != $this->baseSelector->getBase()) {
                    $this->base = $this->baseSelector->getBase();
                    $this->is_modified = TRUE;
                }
            }
        }
    }

    /* Save to LDAP */

    function save() {

        plugin::save();

        $objectclasses = array();
        foreach ($this->attrs['objectClass'] as $key => $class) {
            $objectclasses[] = $this->attrs['objectClass'][$key];
        }
        $this->attrs['objectClass'] = $objectclasses;

        $ldap = $this->config->get_ldap_link();

        /* Take members array */
        if (count($this->triples)) {
            $tmp_triples=$this->triples;
            $ldap->cd($this->config->current['BASE']);
            foreach ($this->triples as $key => $value) {
                if (preg_match("/^\((\S+),\-?,(\S*)\)$/", $value, $matches)) {
                    $ldap->search("(&(objectClass=dNSZone)(relativeDomainName=".$matches[1]."))", array("zoneName", "aRecord"));
                    while ($attrs = $ldap->fetch()) {
                        if(isset($attrs['aRecord']) and isset($attrs['zoneName'])){
                            $tmp_triples[$matches[1] . "." . $attrs['zoneName'][0]] = "(" . $matches[1] . "." . $attrs['zoneName'][0] . ",,)";
                        }
                    }
                }
            }
            $this->attrs['nisNetgroupTriple'] = array_values(array_unique($tmp_triples));
        }

        if (count($this->netgroups)) {
            $this->attrs['memberNisNetgroup'] = array_values(array_unique($this->netgroups));
        }

        /* New accounts need proper 'dn', propagate it to remaining objects */
        if ($this->dn == 'new') {
            $this->dn = 'cn=' . $this->cn . ',' . get_ou("netgroupRDN") . $this->base;
        }

//?    $this->attrs['member'][]= $this->dn;

        /* Save data. Using 'modify' implies that the entry is already present, use 'add' for
          new entries. So do a check first... */
        $ldap->cat($this->dn, array('dn'));
        if ($ldap->fetch()) {
            /* Modify needs array() to remove values :-( */
            if (!count($this->triples)) {
                $this->attrs['nisNetgroupTriple'] = array();
            }
            if (!count($this->netgroups)) {
                $this->attrs['memberNisNetgroup'] = array();
            }
            $mode = "modify";
        } else {
            $mode = "add";
            $ldap->cd($this->config->current['BASE']);
            $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
        }

        /* Write back to ldap */
        $ldap->cd($this->dn);
        $this->cleanup();
        $ldap->$mode($this->attrs);

        /* Remove ACL dependencies too,
         */
        if ($this->dn != $this->orig_dn && $this->orig_dn != "new") {
            $tmp = new acl($this->config, $this->parent, $this->dn);
            $tmp->update_acl_membership($this->orig_dn, $this->dn);
        }

        if ($this->initially_was_account) {
            new log("modify", "netgroups/" . get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());
        } else {
            new log("create", "netgroups/" . get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());
        }

        $ret = 0;
        if (!$ldap->success()) {
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
            $ret = 1;
        }

        /* Post that we've done */
        $this->handle_post_events($mode);

        return ($ret);
    }

    function check() {
        /* Call common method to give check the hook */
        $message = plugin::check();

        /* Permissions for that base? */
        if ($this->base != "") {
            $new_dn = 'cn=' . $this->cn . ',' . get_ou("netgroupRDN") . $this->base;
        } else {
            $new_dn = $this->dn;
        }

        /* must: cn */
        if ($this->cn == "" && $this->acl_is_writeable("cn")) {
            $message[] = msgPool::required(_("Name"));
        }

        // Check if a wrong base was supplied
        if (!$this->baseSelector->checkLastBaseUpdate()) {
            $message[] = msgPool::check_base();
            ;
        }

        /* Check for valid input */
        if (!tests::is_uid($this->cn)) {
            if (strict_uid_mode ()) {
                $message[] = msgPool::invalid(_("Name"), $this->cn, "/[a-z0-9_-]/");
            } else {
                $message[] = msgPool::invalid(_("Name"), $this->cn, "/[a-z0-9_-]/i");
            }
        }

        // Check if a wrong base was supplied
        if (!$this->baseSelector->checkLastBaseUpdate()) {
            $message[] = msgPool::check_base();
            ;
        }

        /* Check netgroup loops and if user is in any netgroup */

        $nisnetmap = array();
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(objectClass=nisNetgroup)", array("cn", "memberNisNetgroup", "nisNetgroupTriple"));
        while ($attrs = $ldap->fetch()) {
            $nisnetmap[$attrs['cn'][0]] = array();
            $nisnetmap[$attrs['cn'][0]]['memberNisNetgroup'] = array();
            $nisnetmap[$attrs['cn'][0]]['nisNetgroupTriple'] = array();
            if(isset($attrs['memberNisNetgroup'])){
                foreach ($attrs['memberNisNetgroup'] as $val) {
                    $nisnetmap[$attrs['cn'][0]]['memberNisNetgroup'][] = $val;
                }
            }
            if(isset($attrs['nisNetgroupTriple'])){
                foreach ($attrs['nisNetgroupTriple'] as $val) {
                    $nisnetmap[$attrs['cn'][0]]['nisNetgroupTriple'][] = $val;
                }
            }
        }

        $checkmsg = $this->checkNisNetgroup($nisnetmap);
        if (count($checkmsg) != 0)
            $message[] = $checkmsg[0];


        /* Check if we are allowed to create or move this object
         */
        if (!$this->orig_dn == "new" ||
                $this->orig_base != $this->base ||
                $this->cn != $this->orig_cn) {

            if ($this->orig_dn == "new" && !$this->acl_is_createable($this->base)) {
                $message[] = msgPool::permCreate();
            } elseif ($this->orig_dn != "new" && !$this->acl_is_moveable($this->base)) {
                $message[] = msgPool::permMove();
            }
        }

        return ($message);
    }

    function checkNisNetgroup($nisnetmap, $timeline=FALSE, $checknng=FALSE) {
        if ($timeline == FALSE) {
            $timeline = array();
            foreach ($this->netgroups as $key => $value) {
                $timeline[] = $key;
                foreach ($nisnetmap[$key]['memberNisNetgroup'] as $val) {
                    $result = $this->checkNisNetgroup($nisnetmap, $timeline, $nisnetmap[$val]);
                    if (count($result) != 0) {
                        return($result);
                    }
                }
            }
        } else {
            foreach ($nisnetmap[$val]['memberNisNetgroup'] as $key) {
                if (array_search($key, $timeline) !== FALSE) {
                    $result = array();
                    $result[] = _("NIS Netgroup Error") . "<b>" . _("Error: There are a loop with NIS Netgroups") . " [" . " " . $key .
                            "] " . "<br>" .
                            _("IMPORTANT: Check the members of NIS Netgroup") . "</b>";
                    return($result);
                }
                $timeline[] = $key;
                $result = $this->checkNisNetgroup($nisnetmap, $timeline, $nisnetmap[$val]);
                if (count($result) != 0) {
                    return($result);
                }
            }
        }
        return(array());
    }

//TODO: check ACL!
    /* Return plugin informations for acl handling  */
    static function plInfo() {
      return array(
        "plShortName"   => _("Generic"),
        "plDescription" => _("Generic NIS Netgroup settings"),
        "plSelfModify"  => FALSE,
        "plCategory"    => array("netgroups" => array("objectClass" => "nisNetgroup", "description" => _("NIS Netgroups"))),
        "plObjectType"  => array("netgroup" => array("name" => _("NIS Netgroup"), "filter" => "objectClass=nisNetgroup")),

        "plProvidedAcls" => array(
          "cn"          => _("Name"),
          "description" => _("Description"),
          "base"        => _("Base"),
          "memberCn"    => _("NIS Netgroup members")
        )
      );
    }

    function multiple_save_object() {
        if (isset($_POST['group_mulitple_edit'])) {

            /* Create a base backup and reset the
              base directly after calling plugin::save_object();
              Base will be set seperatly a few lines below */
            $base_tmp = $this->base;
            plugin::multiple_save_object();
            plugin::save_object();
            $this->base = $base_tmp;

            foreach (array("base") as $attr) {
                if (isset($_POST['use_' . $attr])) {
                    $this->multi_boxes[] = $attr;
                }
            }

            /* Refresh base */
            if ($this->acl_is_moveable($this->base)) {
                if (!$this->baseSelector->update()) {
                    msg_dialog::display(_("Error"), msgPool::permMove(), ERROR_DIALOG);
                }
                if ($this->base != $this->baseSelector->getBase()) {
                    $this->base = $this->baseSelector->getBase();
                    $this->is_modified = TRUE;
                }
            }
        }
    }

    function get_multi_edit_values() {
        $ret = plugin::get_multi_edit_values();

        foreach (array("base") as $attr) {
            if (in_array($attr, $this->multi_boxes)) {
                $ret[$attr] = $this->$attr;
            }
        }

        $ret['memberCn'] = $this->allusers;
        $ret['memberCn_used_by_some'] = $this->users_used_by_some;
        $ret['triples'] = $this->triples;
        $ret['triples_used_by_some'] = $this->triples_used_by_some;
        $ret['netgroups'] = $this->netgroups;
        $ret['netgroups_used_by_some'] = $this->netgroups_used_by_some;
        return($ret);
    }

    function multiple_execute() {
        return($this->execute());
    }

    /* Initialize plugin with given atribute arrays
     */

    function init_multiple_support($attrs, $all) {
        plugin::init_multiple_support($attrs, $all);
        $this->members=array();
        $this->allusers=array();
        $this->users_used_by_some=array();
        $this->netgroups = array();
        $this->netgroups_used_by_some = array();
        if (isset($attrs['memberNisNetgroup'])) {
            for ($i = 0; $i < $attrs['memberNisNetgroup']['count']; $i++) {
                $this->netgroups[$attrs['memberNisNetgroup'][$i]] = $attrs['memberNisNetgroup'][$i];
            }
            ksort($this->netgroups);
        }

        if (isset($all['memberNisNetgroup'])) {
            for ($i = 0; $i < $all['memberNisNetgroup']['count']; $i++) {
                if (!in_array($all['memberNisNetgroup'][$i], $this->netgroups)) {
                    $this->netgroups_used_by_some[$all['memberNisNetgroup'][$i]] = $all['memberNisNetgroup'][$i];
                }
            }
            ksort($this->netgroups_used_by_some);
        }

        $this->triples = array();
        $this->triples_used_by_some = array();
        if (isset($attrs['nisNetgroupTriple'])) {
            for ($i = 0; $i < $attrs['nisNetgroupTriple']['count']; $i++) {
                if (preg_match("/^\((\S+),\-?,(\S*)\)$/", $attrs['nisNetgroupTriple'][$i], $matches)) {
                    //Removing host.domain entries
                    if(strpos($matches[1],".")===false){
                        $this->triples[$matches[1]] = $attrs['nisNetgroupTriple'][$i];
                    }
                }
                if (preg_match("/^\(\-?,(\S+),(\S*)\)$/", $attrs['nisNetgroupTriple'][$i], $matches)) {
                    $this->triples[$matches[1]] = $attrs['nisNetgroupTriple'][$i];
                }
            }
            ksort($this->triples);
        }

        if (isset($all['nisNetgroupTriple'])) {
            for ($i = 0; $i < $all['nisNetgroupTriple']['count']; $i++) {
                if (!in_array($all['nisNetgroupTriple'][$i], $this->triples)) {
                    if (preg_match("/^\((\S+),\-?,(\S*)\)$/", $all['nisNetgroupTriple'][$i], $matches)) {
                        //Removing host.domain entries
                        if(strpos($matches[1],".")===false){
                            $this->triples_used_by_some[$matches[1]] = $all['nisNetgroupTriple'][$i];
                        }
                    }
                    if (preg_match("/^\(\-?,(\S+),(\S*)\)$/", $all['nisNetgroupTriple'][$i], $matches)) {
                        $this->triples_used_by_some[$matches[1]] = $all['nisNetgroupTriple'][$i];
                    }
                }
            }
            ksort($this->triples_used_by_some);
        }

        $this->reload();

    }

    function set_multi_edit_values($attrs) {
        $triples = array();
        $netgroups = array();

        /* Update netgroupMembership, keep optinal netgroup */
        foreach ($attrs['triples_used_by_some'] as $uid => $value) {
            if (in_array($uid, array_keys($this->triples))) {
                $triples[$uid] = $value;
            }
        }

        foreach ($attrs['netgroups_used_by_some'] as $uid => $value) {
            if (in_array($uid, array_keys($this->netgroups))) {
                $netgroups[$uid] = $value;
            }
        }

        /* Update netgroupMembership, add forced netgroups */
        foreach ($attrs['triples'] as $uid => $value) {
            $triples[$uid] = $value;
        }
        foreach ($attrs['netgroups'] as $uid => $value) {
            $netgroups[$uid] = $value;
        }
        plugin::set_multi_edit_values($attrs);
        $this->triples = $triples;
        $this->netgroups = $netgroups;
        //$this->memberCn = $users;
    }

}

?>
