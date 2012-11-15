<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class wingeneric extends plugin
{
  /* Generic terminal attributes */
  var $interfaces= array();

  /* Needed values and lists */
  var $base= "";
  var $cn= "";
  var $description= "";
  var $orig_dn= "";
  var $shadowLastChange="";
  var $uidNumber=1000;
  var $gidNumber="515";
  var $loginShell="";
  var $gecos="";
  var $shadowMin="";
  var $shadowWarning="";
  var $shadowInactive="";
  var $uid="";
  var $sn="";
  var $givenName="";
  var $homeDirectory="";
  var $sambaSID="";
  var $sambaPrimaryGroupSID="";
  var $displayName="";
  var $sambaPwdMustChange="";
  var $sambaNTPassword="";
  var $sambaPwdLastSet="";
  var $sambaAcctFlags="[W          ]";
  var $netConfigDNS;

  var $SID="";
  var $ridBase= 0;
  var $sambaDomainName= "";

  var $orig_cn;
  var $orig_base;
  var $baseSelector;

  /* attribute list for save action */
  var $ignore_account = TRUE;
  var $attributes   = array("cn", "description","shadowLastChange",
                            "uidNumber","gidNumber","loginShell","gecos","shadowMin","shadowWarning",
                            "shadowInactive","uid","cn","sn","givenName","homeDirectory","sambaSID",
                            "sambaPrimaryGroupSID","displayName", "sambaPwdMustChange",
                            "sambaNTPassword","sambaPwdLastSet","sambaAcctFlags");
  var $objectclasses = array("posixAccount","account","sambaSamAccount","top");

  var $view_logged = FALSE;

  function wingeneric (&$config, $dn= NULL, $parent= NULL)
  {
    plugin::plugin ($config, $dn, $parent);
    $this->netConfigDNS = new networkSettings($this->config,$this,$this->objectclasses,FALSE,"uid");
    $this->netConfigDNS->cn =  preg_replace("/\\\$\$/","",$this->uid);
    $this->netConfigDNS->objectclasses=array("ipHost","ieee802Device");
    $this->netConfigDNS->MACisMust = TRUE;
    $this->netConfigDNS->IPisMust = TRUE;

    /* Set base */
    if ($this->dn == "new") {
      $ui = get_userinfo();
      $this->base= dn2base(session::global_is_set("CurrentMainBase")?"cn=dummy,".session::global_get("CurrentMainBase"):$ui->dn);
      $this->cn= "";
    } else {
      $this->base= preg_replace ("/^[^,]+,".preg_quote(get_winstations_ou(), '/')."/i", "", $this->dn);
    }

    $this->cn = preg_replace("/\\\$\$/","",$this->cn);

    /* Save dn for later references */
    $this->orig_dn    = $this->dn;
    $this->orig_base  = $this->base;
    $this->orig_cn    = $this->cn;

    /* Instanciate base selector */
    $this->baseSelector = new baseSelector($this->get_allowed_bases(), $this->base);
    $this->baseSelector->setSubmitButton(false);
    $this->baseSelector->setHeight(300);
    $this->baseSelector->update(true);

  }


  function set_acl_base($base)
  {
    plugin::set_acl_base($base);
    $this->netConfigDNS->set_acl_base($base);
  }


  function set_acl_category($cat)
  {
    plugin::set_acl_category($cat);
    $this->netConfigDNS->set_acl_category($cat);
  }


  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    if ($this->is_account && !$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view","winworkstation/".get_class($this),$this->dn);
    }

    /* Do we represent a valid winstations? */
    if (!$this->is_account && $this->parent === NULL) {
      $display = "<img alt=\"\" src=\"images/small-error.png\" align=middle>&nbsp;<b>".
        msgPool::noValidExtension(_("network"))."</b>";
      return $display;
    }

    /* Fill templating stuff */
    $smarty = get_smarty();
    $smarty->assign("usePrototype", "true");

    $tmp = $this->plInfo();
    foreach ($tmp['plProvidedAcls'] as $name => $translated) {
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    /* Assign attributes */
    foreach ($this->attributes as $attr) {
      $smarty->assign("$attr", $this->$attr);
    }

    $smarty->assign("base", $this->baseSelector->render());

    // Fill domain selection.
    $domains = array_keys($this->config->data['SERVERS']['SAMBA']);
    $smarty->assign("domains", $domains);
    $smarty->assign("sambaDomainName",$this->sambaDomainName);

    /* Show main page */
    $str = $this->netConfigDNS->execute();
    if (is_object($this->netConfigDNS->dialog)) {
      return($str);
    }
    $smarty->assign("netconfig", $str);
    return($smarty->fetch (get_template_path('wingeneric.tpl', TRUE,dirname(__FILE__))));
  }

  function remove_from_parent()
  {
    $this->netConfigDNS->remove_from_parent();
    $ldap = $this->config->get_ldap_link();
    $ldap->rmdir($this->dn);
    new log("remove","winworkstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }
    $this->handle_post_events("remove");

    /* Delete references to object groups */
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
    while ($ldap->fetch()){
      $og = new ogroup($this->config, $ldap->getDN());
      unset($og->member[$this->dn]);
      $og->save ();
    }
  }


  /* Save data to object */
  function save_object()
  {
    /* Create a base backup and reset the
       base directly after calling plugin::save_object();
       Base will be set seperatly a few lines below */
    $base_tmp = $this->base;
    plugin::save_object();
    $this->uid  = $this->cn."$";
    $this->base = $base_tmp;

    $this->netConfigDNS->save_object();

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

    // get sambaDomainName attribute
    if ($this->acl_is_writeable("sambaDomainName") && isset ($_POST['sambaDomainName'])) {
      $this->sambaDomainName = validate($_POST['sambaDomainName']);
    }
  }


  /* Check supplied data */
  function check()
  {
    if (!preg_match('/\$$/',$this->uid)) {
      $this->uid .= "$";
    }

    // Get samba domain and its sid/rid base
    if ($this->sambaSID != "") {
      $this->SID = preg_replace ("/-[^-]+$/", "", $this->sambaSID);
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=sambaDomain)(sambaSID=$this->SID))",array("sambaAlgorithmicRidBase","sambaDomainName"));
      if ($ldap->count() != 0) {
        $attrs= $ldap->fetch();
        if (isset($attrs['sambaAlgorithmicRidBase'])) {
          $this->ridBase = $attrs['sambaAlgorithmicRidBase'][0];
        } else {
          $this->ridBase = $this->config->get_cfg_value("sambaRidBase");
        }
        if ($this->sambaDomainName == "") {
          $this->sambaDomainName = $attrs['sambaDomainName'][0];
        }
      } else {
        // Fall back to a 'DEFAULT' domain, if none was found in LDAP.
        if ($this->sambaDomainName == "") $this->sambaDomainName= "DEFAULT";

        // Nothing in ldap, use configured sid and rid values.
        $this->ridBase  = $this->config->get_cfg_value("sambaRidBase");
        $this->SID      = $this->config->get_cfg_value("sambaSid");
      }
    }

    /* Call common method to give check the hook */
    $message = plugin::check();
    $message = array_merge($message, $this->netConfigDNS->check());
    $this->dn = "uid=".$this->uid.",".get_ou('sambaMachineAccountRDN').$this->base;

    if ($this->orig_dn != $this->dn) {
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->base);
      $ldap->search("(uid=".$this->uid.")", array("uid"));
      if ($ldap->count() != 0) {
        while ($attrs = $ldap->fetch()) {
          if ($attrs['dn'] != $this->orig_dn) {
            $message[] = msgPool::duplicated(_("Name"));
            break;
          }
        }
      }
    }

    // Check if a wrong base was supplied
    if (!$this->baseSelector->checkLastBaseUpdate()) {
      $message[] = msgPool::check_base();
    }

    /* Check if we are allowed to create or move this object
     */
    if ($this->orig_dn == "new" && !$this->acl_is_createable($this->base)) {
      $message[] = msgPool::permCreate();
    } elseif($this->orig_dn != "new" && $this->base != $this->orig_base && !$this->acl_is_moveable($this->base)) {
      $message[] = msgPool::permMove();
    }

    return $message;
  }

  /* Save to LDAP */
  function save()
  {
    plugin::save();

    /* Remove all empty values */
    if ($this->orig_dn == 'new') {
      $attrs= array();
      foreach ($this->attrs as $key => $val) {
        if (is_array($val) && count($val) == 0) {
          continue;
        }
        $attrs[$key] = $val;
      }

      if (!isset($this->attrs['uid'])) {
          $attrs['uid'] = $this->attrs['cn'];
      }

      if (!isset($this->attrs['homeDirectory'])) {
        $attrs['homeDirectory'] = "/dev/null";
      }

      $this->attrs = $attrs;
    }

    //verify that the $this->sambaSID exist and that it is correct
    //yes do nothing
    //no get it from config and generate a new sid

    if (($this->sambaSID == "") || (substr_count ( $this->sambaSID , "-" ) < 7))  {
      $this->SID = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'];
      $this->ridBase = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'];

      // create sambaSID for workstation
      $uidNumber = $this->uidNumber;
      while(TRUE) {
        $sid = $this->SID."-".($uidNumber*2 + $this->ridBase);
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(sambaSID=$sid)", array("sambaSID"));
        if ($ldap->count() == 0) {
          break;
        }
        $uidNumber++;
      }
      $this->attrs['sambaSID'] = $sid;
      $this->attrs['uidNumber'] = $uidNumber + $this->ridBase;

    }
    if (($this->gosaUnitTag) && (!in_array_ics("gosaAdministrativeUnitTag",$this->attrs['objectClass']))) {
      $this->attrs['objectClass'][] = "gosaAdministrativeUnitTag";
    }

    $curOcs = $this->attrs['objectClass'];
    $this->cleanup();

    // Prepare list of object classes
    $ocs = array();
    if (!empty($this->netConfigDNS->macAddress)) {
        $ocs [] = 'ieee802Device';
    } else {
        $curOcs = array_remove_entries(array('ieee802Device'), $curOcs);
        $this->attrs['macAddress'] = array();
    }
    if (!empty($this->netConfigDNS->ipHostNumber)) {
        $ocs [] = 'ipHost';
    } else {
        $curOcs = array_remove_entries(array('ipHost'), $curOcs);
        $this->attrs['ipHostNumber'] = array();
    }

    $this->attrs['objectClass'] = $curOcs;

    /* Write back to ldap */
    $ldap = $this->config->get_ldap_link();
    if ($this->orig_dn == 'new') {
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
      new log("create","winworkstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      $this->handle_post_events("add");
    } else {
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
      new log("modify","winworkstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      $this->handle_post_events("modify");
    }
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    $this->netConfigDNS->cn =  preg_replace("/\\\$\$/","",$this->uid);
    $this->netConfigDNS->objectclasses = $ocs;
    $this->netConfigDNS->save();

    /* Optionally execute a command after we're done */
    $this->handle_post_events("add");
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Win generic"),
      "plDescription" => _("Windows workstation generic"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("winworkstation" => array("description"  => _("Windows workstation"),
                                                         "objectClass"  => "gotoWorkstation")),
      "plObjectType"  => array("winworkstation" => array("name" => _("Windows workstation"),
                                                         "filter" => "objectClass=gotoWorkstation")),

      "plProvidedAcls"  => array(
        "cn"                  => _("Name"),
        "base"                => _("Base"),
        "description"         => _("Description"),
        "sambaDomainName"     => _("Domain")
      )
    );
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
