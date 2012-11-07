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



class netgroupSystem extends plugin {
    /* Definitions */

    var $plHeadline = "NIS Netgroup";
    var $plDescription = "Choose NIS Netgroups for the host";

    /* Plugin specific values */
    var $view_logged = FALSE;

    /* attribute list for save action */
    var $CopyPasteVars = array();
    var $attributes = array();
    var $objectclasses = array("whatever");
    var $cn = "";
    var $members = array();
    var $netgroups = array();
    var $userSelect = FALSE;
    var $multiple_support = TRUE;
    var $is_account = TRUE;
    var $dn = "";
    var $attrs = array();

    function netgroupSystem(&$config, $dn= NULL) {
        $this->config = $config;

        /* Load bases attributes */
        plugin::plugin($config, $dn);

        /* Setting uid to default */
        if (isset($this->attrs['cn'][0])) {
            $this->cn = $this->attrs['cn'][0];
        }

        if ($this->dn != "new") {
            /* Check netgroup triples */
            $ldap = $this->config->get_ldap_link();
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(objectClass=nisNetgroup)", array("cn", "nisNetgroupTriple"));
            while ($attrs = $ldap->fetch()) {
                if (isset($attrs['nisNetgroupTriple'])) {
                    foreach ($attrs['nisNetgroupTriple'] as $val) {
                        if (preg_match("/^\(" . $this->cn . ",\-?,(\S*)\)$/", $val, $matches)) {
                            $desc=""; if(isset($attrs['description'])){
                                if(is_array($attrs['description'])) $desc=$attrs['description'][0];
                                else $desc=$attrs['description'];
                            }
                            if(isset($attrs['description'])) $desc=$attrs['description'];
                            $this->members[$attrs['cn'][0]] = $attrs['cn'][0]."[".$desc."]";
                            $this->netgroups[$attrs['cn'][0]] = $attrs['cn'][0];
                        }
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
            new log("view", "system/" . get_class($this), $this->dn);
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
                    $desc=""; if(isset($attrs['description'])){
                        if(is_array($attrs['description'])) $desc=$attrs['description'][0];
                        else $desc=$attrs['description'];
                    }
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

        /* Multiple edit handling */
        $smarty->assign("multiple_support", $this->multiple_support_active);


        $tmp = $this->plInfo();
        foreach ($tmp['plProvidedAcls'] as $name => $translation) {
            $smarty->assign($name . "ACL", $this->getacl($name));
        }

        $display.= $smarty->fetch(get_template_path('netgroupSystem.tpl', TRUE, dirname(__FILE__)));
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
        if (strlen($this->cn)==0) {
            $this->cn=$ldap->getCn($this->dn);
        }

        $attrs=array();
        $attrs['objectClass'] = array("top","nisNetgroup");

        foreach(array_keys($this->members) as $val){

            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(&(objectClass=nisNetgroup)(cn=".$val."))", array("dn", "cn", "nisNetgroupTriple"));
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
                $triples[]='('.$this->cn.',,)';
                $ldap2 = $this->config->get_ldap_link();
                $ldap2->cd($this->config->current['BASE']);
                $ldap2->search("(&(objectClass=dNSZone)(relativeDomainName=".$this->cn."))", array("zoneName", "aRecord"));
                while ($attrs2 = $ldap2->fetch()) {
                    if(isset($attrs2['aRecord'])  and isset($attrs2['zoneName'])){
                        $triples[] = "(" . $this->cn . "." . $attrs2['zoneName'][0] . ",,)";
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

        $nomembers=array_diff(array_keys($this->netgroups),array_keys($this->members));
        $ldap = $this->config->get_ldap_link();
        foreach($nomembers as $val){
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(&(objectClass=nisNetgroup)(cn=".$val."))", array("dn", "cn", "nisNetgroupTriple"));
            $attrs['nisNetgroupTriple']=array();
            $attrs['memberNisNetgroup']=array();
            while ($sattrs = $ldap->fetch()) {
                $triples=array();
                $netgroups=array();

                if (isset($sattrs['nisNetgroupTriple'])) {
                    for ($i=0;$i<$sattrs['nisNetgroupTriple']['count'];$i++){
                        if (preg_match("/^\(" . $this->cn . ",\-?,(\S*)\)$/", $sattrs['nisNetgroupTriple'][$i], $matches)===FALSE) {
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
        return($this->execute());
    }

    static function plInfo() {
      return array(
        "plShortName"     => _("NIS Netgroup"),
        "plDescription"   => _("NIS Netgroup member"),
        "plSelfModify"    => FALSE,
        "plDepends"       => array(),
        "plPriority"      => 2,
        "plCategory"      => array("workstation","terminal","phone","server","component","printer"),
        "plObjectType"    => array("workstation","terminal","phone","server","component","printer"),
        "plProvidedAcls"  => array(
          "memberCn" => _("NIS Netgroup members")
        )
      );
    }

}

?>
