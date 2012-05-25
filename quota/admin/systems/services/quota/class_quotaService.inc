<?php

/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011  FusionDirectory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class quotaService extends goService {

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("quotaService");
  var $attributes       = array("quotaLdapServerURI","quotaLdapServerUserDn",
                                "quotaLdapServerUserPassword","quotaDeviceParameters",
                                "quotaLdapSearchIdAttribute",
                                "quotaMsgCharsetSupport",
                                "quotaMsgSignatureSupport","quotaMsgContactSupport",
                                "quotaMsgFromSupport","quotaMsgSubjectSupport",
                                "quotaMsgContentSupport","quotaCarbonCopyMail",
                                "quotaMailCommand");

  var $DisplayName      = "";
  var $dn               = NULL;

  var $quotaDeviceParameters        = array();
  var $quotaLdapSearchIdAttribute   = "";
  var $quotaMsgCharsetSupport       = "UTF-8";
  var $quotaMsgSignatureSupport     = "";
  var $quotaMsgContactSupport       = "";
  var $quotaMsgFromSupport          = "";
  var $quotaMsgSubjectSupport       = "";
  var $quotaMsgContentSupport       = "";
  var $quotaLdapServerURI           = "";
  var $quotaLdapServerUserDn        = "";
  var $quotaLdapServerUserPassword  = "";
  var $quotaCarbonCopyMail          = "";
  var $quotaMailCommand             = "/usr/sbin/sendmail -t";
  var $charsets                     = array();

  var $ldapServers = array();

  var $acl;
  var $cn           = "";
  var $description  = "";
  var $view_logged  =FALSE;

  function quotaService(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Quota service");

    /* Load arrays */
    /* loading of each array quotaDeviceParameters */
    $tmp = array();
    if (isset($this->attrs['quotaDeviceParameters'])) {
      for ($i= 0; $i<$this->attrs['quotaDeviceParameters']['count']; $i++) {
        $tmp[$this->attrs['quotaDeviceParameters'][$i]] = $this->attrs['quotaDeviceParameters'][$i];
      }
    }
    $this->quotaDeviceParameters = $tmp;


    /* loading of each array of any share delared on this server  */
    $tmp = array();
    if (isset($this->attrs['goExportEntry'])) {
      for ($i= 0; $i<$this->attrs['goExportEntry']['count']; $i++) {
        $tmp[$this->attrs['goExportEntry'][$i]] = $this->attrs['goExportEntry'][$i];
      }
    }
    $this->goExportEntry = $tmp;

    $this->quotaLdapSearchIdAttribute = $this->config->get_cfg_value("accountPrimaryAttribute");


    /********************** LDAP SERVER SELECTION SYSTEM ***********************/
    $ldap = $this->config->get_ldap_link();

    /* Get List of server with LDAP service */
    $this->ldapServers = array();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=goServer)(objectClass=goLdapServer))", array("cn", "goLdapBase"));
    while ($attrs = $ldap->fetch()) {
      $this->ldapServers[$attrs['goLdapBase'][0]] = $attrs['cn'][0];
    }
  }

  function execute()
  {
    $smarty = get_smarty();

    if ($this->is_account && !$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    /* PART that should go in save_object if it was called only once - see https://forge.fusiondirectory.org/issues/848 */
    /* Delete Device record */
    if ((isset($_GET['act']) && ($_GET['act'] == "deleteDevPar"))) {
      $tmp = base64_decode($_GET['id']);
      unset($this->quotaDeviceParameters[$tmp]);
    }

    /* Add device record */
    if (isset($_POST['addquotadevice'])) {
      $quota_device_added =  $_POST['quota_device_name'].":"
                            .$_POST['quota_device_blocksize'].":"
                            .$_POST['quota_device_description'];
      $this->quotaDeviceParameters[] = $quota_device_added;
    }
    /* END OF PART */

    if (empty($this->ldapServers)) {
      $display =  "<img alt=\"\"src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
                  _("There is no ldap server configured")."</b>";
      $display .= back_to_main();
      return $display;
    }

    /* assign all $this objects values in variable for pushing in smarty template only this quota object*/
    foreach($this->attributes as $val) {
      $smarty->assign($val, $this->$val);
    }

    /*  prepare device listing */
    $quotaDevicesList = new divSelectBox("QuotaDeviceslist");
    $quotaDevicesList->SetHeight(80);

    /* prepare deletion links */
    $link_delete = "<a href='?plug=".$_GET['plug']."&act=deleteDevPar&id=%s'><img style='border: 0px' src='images/lists/trash.png'></a>";
    foreach ($this->quotaDeviceParameters as $quotaparameters => $values ) {
      $quotadevice_exploded=explode(":",$values);
      $quotaDevicesList->AddEntry(array(
            array("string" => _("Device")." : " . $quotadevice_exploded[0]),
            array("string" => _("BlockSize")." : " . $quotadevice_exploded[1]),
            array("string" => _("Description")." : ".$quotadevice_exploded[2]),
            array("string" =>str_replace("%s",base64_encode($quotaparameters),$link_delete))
          ));
    }
    $smarty->assign("quotaDevicesList",$quotaDevicesList->DrawList());

    $smarty->assign("ldapServers",$this->ldapServers);

    /* create drop list for all available share */
    $shareInfos = array();
    foreach ($this->goExportEntry as $shareEntry => $values) {
      $shareEntry_exploded=explode("|",$values);
      $share_name       = $shareEntry_exploded[0];
      $share_prot       = $shareEntry_exploded[2];
      $share_device     = $shareEntry_exploded[4];
      $shareInfos[$share_device] = $share_device."  ( ".$share_name." "._("by")." ".$share_prot." )";
    }
    $smarty->assign("shareInfos",$shareInfos);

    /* get all encodings types available */
    $this->charsets = array();
    if (!file_exists(CONFIG_DIR."/encodings")) {
      msg_dialog::display(_("Error"), msgPool::fileDoesNotExist(CONFIG_DIR."/encodings"), ERROR_DIALOG);
    } else {
      if (!is_readable(CONFIG_DIR."/encodings")) {
        msg_dialog::display(_("Error"), msgPool::cannotReadFile(CONFIG_DIR."/encodings"), ERROR_DIALOG);
      } else {
        $fp = fopen(CONFIG_DIR."/encodings","r");
        $i = 100;
        while (!feof($fp)&&$i) {
          $i--;
          $str = trim(fgets($fp,256));

          /* Skip comments */
          if(!preg_match("/^#/",$str)) {
            $arr = explode("=",$str);
            if(count($arr)==2) {
              $this->charsets[$arr[0]]=$arr[1];
            }
          }
        }
      }
    }
    $smarty->assign("charsets" ,$this->charsets);

    /* special Value for adding ballon info */
    $smarty->assign("HELP_get_blocksize",_("HELP how to get blocksize"));

    return($smarty->fetch(get_template_path("quotaService.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields               = goService::getListEntry();
    $fields['Message']    = _("Quota service");

    return($fields);
  }

  function save_object()
  {
    if (isset($_POST['quotaServicePosted'])) {
      plugin::save_object();
    }
  }

  function save()
  {
    plugin::save();

    foreach($this->attributes as $val){
      if ($this->$val != ""){
        $this->attrs[$val]= array($this->$val);
      }
    }

    $this->attrs['quotaDeviceParameters'] = array();

    foreach($this->quotaDeviceParameters as $entry){
      $this->attrs['quotaDeviceParameters'][] = $entry;
    }

    /* Check if this is a new entry ... add/modify */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->dn,array("objectClass"));
    if($ldap->count()) {
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
    } else {
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
    }
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Quota Service"),
          "plDescription" => _("Quota Service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 91,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
              "quotaDeviceParameters"         =>_("parameters for devices with quota"),
              "quotaLdapSearchIdAttribute"    =>_("id of user in LDAP"),
              "quotaMsgCharsetSupport"        =>_("charset of Message"),
              "quotaMsgSignatureSupport"      =>_("signature of warn message"),
              "quotaMsgContactSupport"        =>_("mail of contact for support"),
              "quotaMsgFromSupport"           =>_("subject of mail"),
              "quotaMsgSubjectSupport"        =>_("subject of support"),
              "quotaMsgContentSupport"        => _("core of message of Support"),
              "quotaCarbonCopyMail"           => _("Carbon Copy for all mails"),
              "quotaMailCommand"              => _("Mail method for sending mail"),
              "quotaLdapServerUserDn"         => _("User for ldap connection"),
              "quotaLdapServerUserPassword"   => _("Password for ldap connection"),
              "quotaLdapServerURI"            => _("URI for LDAP server"),
          )
        ));
  }


  function check(){ return array(); }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
