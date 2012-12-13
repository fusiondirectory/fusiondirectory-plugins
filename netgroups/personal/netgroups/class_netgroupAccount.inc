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



class netgroupAccount extends plugin {
    /* Definitions */

    var $plHeadline = "NIS Netgroup";
    var $plDescription = "Choose NIS Netgroups for the user";

    /* Plugin specific values */
    var $view_logged = FALSE;

    /* attribute list for save action */
    var $CopyPasteVars = array();
    var $attributes = array();
    var $objectclasses = array("whatever");
    var $uid = "";
    var $members = array();
    var $netgroups = array();
    var $userSelect = FALSE;
    var $multiple_support = TRUE;
    var $is_account = TRUE;
    var $dn = "";
    var $attrs = array();

    var $netgroups_all = array();
    var $netgroups_some = array();

    function netgroupAccount(&$config, $dn= NULL) {
        $this->config = $config;

        /* Load bases attributes */
        plugin::plugin($config, $dn);

        /* Setting uid to default */
        if (isset($this->attrs['uid'][0])) {
            $this->uid = $this->attrs['uid'][0];
        }

        if ($this->dn != "new") {
            /* Check netgroup triples */
            $ldap = $this->config->get_ldap_link();
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(objectClass=nisNetgroup)", array("cn", "nisNetgroupTriple"));
            while ($attrs = $ldap->fetch()) {
                if (isset($attrs['nisNetgroupTriple'])) {
                    foreach ($attrs['nisNetgroupTriple'] as $val) {
                        if (preg_match("/^\(\-?," . $this->uid . ",(\S*)\)$/", $val, $matches)) {
                            $desc=""; if(isset($attrs['description'])) $desc=$attrs['description'];
                            $this->members[$attrs['cn'][0]] = $attrs['cn'][0]."[".$desc."]";
                            $this->netgroups[$attrs['cn'][0]] = $attrs['cn'][0];
                        }
                    }
                }
            }
        }
    }

    function init_multiple_support($attrs, $all) {
        plugin::init_multiple_support($attrs, $all);
        $multiuid=$all["uid"];
        $netgroup_map=array();
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(objectClass=nisNetgroup)", array("cn", "description","nisNetgroupTriple"));
        while ($attrs = $ldap->fetch()) {
            if (isset($attrs['nisNetgroupTriple'])) {
                foreach ($attrs['nisNetgroupTriple'] as $value) {
                    if (preg_match("/^\(\-?,(\S+),(\S*)\)$/", $value, $matches)) {
                        if (in_array($matches[1], $multiuid)) {
                            if(!isset($netgroup_map[$attrs['cn'][0]])) $netgroup_map[$attrs['cn'][0]]=array();
                            $netgroup_map[$attrs['cn'][0]][]=$matches[1];
                        }
                    }
                }
                if(isset($netgroup_map[$attrs['cn'][0]])){
                    $desc=""; if(isset($attrs['description'])) $desc=$attrs['description'];
                    //$this->members[$attrs['cn'][0]] = $attrs['cn'][0]."[".$desc."]";
                    if(count(array_unique($netgroup_map[$attrs['cn'][0]]))==$multiuid["count"]){
                        $this->netgroups_all[$attrs['cn'][0]] = $attrs['cn'][0];
                    }else{
                        $this->netgroups_some[$attrs['cn'][0]] = $attrs['cn'][0];
                    }
                }
            }
        }

    }

    function execute($isCopyPaste = false) {
        /* Call parent execute */
        plugin::execute();
        $display = "";

        /* Log view */
        if ($this->is_account && !$this->view_logged) {
            $this->view_logged = TRUE;
            new log("view", "users/" . get_class($this), $this->dn);
        }

        if (isset($_POST['del_users']) && isset($_POST['members']) && preg_match("/w/", $this->getacl("triples"))) {
            foreach ($_POST['members'] as $value) {
                unset($this->members["$value"]);
            }
        }

        /* Add objects? */
        if (isset($_POST["edit_membership"]) && preg_match("/w/", $this->getacl("netgroups"))) {
            $this->userSelect = new memberNisnetgroupSelect($this->config, get_userinfo());
            $this->dialog = TRUE;
        }

        /* Add objects finished? */
        if (isset($_POST["add_memberNisnetgroups_cancel"])) {
            $this->userSelect = NULL;
            $this->dialog = FALSE;
        }

        /* Add to netgroup */
        if (isset($_POST['add_memberNisnetgroups_finish']) && $this->userSelect) {

            /* Get all the dn from userSelect */
            $users = $this->userSelect->detectPostActions();
            if (isset($users['targets'])) {
                $headpage = $this->userSelect->getHeadpage();
                foreach ($users['targets'] as $dn) {
                    $attrs = $headpage->getEntry($dn);
                    $desc=""; if(isset($attrs['description'])) $desc=$attrs['description'];
                    $this->members[$attrs['cn'][0]] = $attrs['cn'][0]."[".$desc."]";
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

        /* Show main page */
        $smarty = get_smarty();

        $smarty->assign("usePrototype", "true");

        /* Manage object add dialog */
        if ($this->userSelect) {
            return($this->userSelect->execute());
        }

        /* Members and users */
        $smarty->assign("members", $this->members);

        $tmp = $this->plInfo();
        foreach ($tmp['plProvidedAcls'] as $name => $translation) {
            $smarty->assign($name . "ACL", $this->getacl($name));
        }

        $smarty->assign("memberCn_All", $this->netgroups_all);
        $smarty->assign("memberCn_Some", $this->netgroups_some);

        $display.= $smarty->fetch(get_template_path('netgroupAccount.tpl', TRUE, dirname(__FILE__)));
        return($display);
    }

    function save_object() {
        /* Save additional values for possible next step */
        if (isset($_POST['nisnetgroupedit'])) {

            /* Create a base backup and reset the
              base directly after calling plugin::save_object();
              Base will be set seperatly a few lines below */
            plugin::save_object();
        }
    }

    /* Save data to LDAP, depending on is_account we save or delete */

    function save() {

        /* Call parents save to prepare $this->attrs */
        plugin::save();
        $ldap = $this->config->get_ldap_link();
        $attrs=array();
        $attrs['objectClass'] = array("top","nisNetgroup");

        foreach(array_keys($this->members) as $val){
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(&(objectClass=nisNetgroup)(cn=".$val."))", array("dn", "cn", "nisNetgroupTriple","memberNisNetgroups"));
            $attrs['nisNetgroupTriple']=array();
            $attrs['memberNisNetgroup']=array();
            while ($sattrs = $ldap->fetch()) {
                $triples=array();
                $netgroups=array();

                if (isset($sattrs['nisNetgroupTriple'])) {
                    for ($i=0;$i<$sattrs['nisNetgroupTriple']['count'];$i++){
                        $triples[]=$sattrs['nisNetgroupTriple'][$i];
                    }
                }
                $triples[]='(,'.$this->uid.',)';
                $attrs['nisNetgroupTriple']=array_values(array_unique($triples));

                if (isset($sattrs['memberNisNetgroup'])) {
                    for ($i=0;$i<$sattrs['memberNisNetgroup']['count'];$i++){
                        $netgroups[]=$sattrs['memberNisNetgroup'][$i];
                    }
                }
                $attrs['memberNisNetgroup']=array_values(array_unique($netgroups));
                $ldap->cd($sattrs['dn']);
                $ldap->modify($attrs);
            }
        }

        $nomembers=array_diff(array_keys($this->netgroups),array_keys($this->members));
        foreach($nomembers as $val){
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(&(objectClass=nisNetgroup)(cn=".$val."))", array("dn", "cn", "nisNetgroupTriple","memberNisNetgroups"));
            $attrs['nisNetgroupTriple']=array();
            $attrs['memberNisNetgroup']=array();
            while ($sattrs = $ldap->fetch()) {
                $triples=array();
                $netgroups=array();

                $count=true;
                if (isset($sattrs['nisNetgroupTriple'])) {
                    for ($i=0;$i<$sattrs['nisNetgroupTriple']['count'];$i++){
                        if (preg_match("/^\(\-?," . $this->uid . ",(\S*)\)$/", $sattrs['nisNetgroupTriple'][$i], $matches)===FALSE) {
                            $triples[]=$sattrs['nisNetgroupTriple'][$i];
                        }
                    }
                }

                $attrs['nisNetgroupTriple']=array_values(array_unique($triples));

                if (isset($sattrs['memberNisNetgroup'])) {
                    for ($i=0;$i<$sattrs['memberNisNetgroup']['count'];$i++){
                        $netgroups[]=$sattrs['memberNisNetgroup'][$i];
                    }
                }
                $attrs['memberNisNetgroup']=array_values(array_unique($netgroups));
                $ldap->cd($sattrs['dn']);
                $ldap->modify($attrs);
            }
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
        return ($ret);
    }

    function get_multi_edit_values() {
        $ret = plugin::get_multi_edit_values();

        foreach (array("base") as $attr) {
            if (in_array($attr, $this->multi_boxes)) {
                $ret[$attr] = $this->$attr;
            }
        }

        $ret['memberCn'] = $this->netgroups_all;
        $ret['memberCn_used_by_some'] = $this->netgroups_some;
        $ret['netgroups_all'] = $this->netgroups_all;
        $ret['netgroups_some'] = $this->netgroups_some;
        return($ret);
    }

    function set_multi_edit_values($attrs) {
        $netgroups = array();

        foreach ($attrs['netgroups_some'] as $uid => $value) {
            if (in_array($uid, array_keys($this->netgroups))) {
                $netgroups[$uid] = $uid;
            }
        }

        foreach ($attrs['netgroups_all'] as $uid => $value) {
            $netgroups[$uid] = $uid;
        }
        plugin::set_multi_edit_values($attrs);
        //$this->netgroups = $netgroups;
        $this->members = $netgroups;
        //$this->memberCn = $users;
    }


    function check() {
        /* Call common method to give check the hook */
        $message = plugin::check();
        return ($message);
    }

    function multiple_check() {
        $message = plugin::multiple_check();
        return($message);
    }

    /* Adapt from template, using 'dn' */
    function adapt_from_template($dn, $skip= array()) {
        //TODO
    }

    function multiple_execute() {
        /*TODO:
         * - Coger todos los uid/cn que correspondan de $this->multi_attrs_all
         * - Buscar netgroups con todos los uid/cn
         * - Buscar netgroups con alguno de los uid/cn
         * - Quitar los todos de loa algunos
         * - El execute debe de estar aquí.
         * - agregar get_multi_edit_values() y set_multi_edit_values() para el save()
        */
        return($this->execute());
    }

    static function plInfo() {
      return array(
        "plDescription"   => _("NIS Netgroup member"),
        "plSelfModify"    => FALSE,
        "plPriority"      => 2,
        "plCategory"      => array("users"),
        "plObjectType"    => array("user"),
        "plProvidedAcls"  => array(
            "memberCn" => _("NIS Netgroup members")
        )
      );
    }

}

?>
