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

class rSyslogServer extends goService{

  /* This plugin only writes its objectClass */
  var $objectclasses  = array("goLogDBServer");
  var $attributes     = array("gosaLogDB","goLogAdmin","goLogPassword");
  var $StatusFlag     = "";

  /* This class can't be assigned twice so it conflicts with itself */
  var $conflicts      = array("goLogDBServer");

  var $DisplayName    = "";
  var $dn             = NULL;
  var $acl;
  var $cn             = "";
  var $gosaLogDB      = "Syslog";
  var $goLogAdmin     = "rsyslog";
  var $goLogPassword  = "";
  var $view_logged    = FALSE;

  var $goServerStatus   = "";


  function rSyslogServer(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Logs service");
  }


  function execute()
  {
    $smarty = get_smarty();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    return($smarty->fetch(get_template_path("serviceSyslog.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields = goService::getListEntry();
    $fields['Message']      = _("Logs service");
    #$fields['AllowEdit']    = true;
    #$fields['AllowStart']   = FALSE;
    #$fields['AllowStop']    = FALSE;
    #$fields['AllowRestart'] = FALSE;
    return($fields);
  }


  function check()
  {
    $message = plugin::check();
      if (empty($this->goLogAdmin)){
        $message[]= msgPool::required(_("User"));
      }
      if (empty($this->gosaLogDB)){
        $message[]= msgPool::required(_("Database"));
      }
      if (empty($this->goLogPassword)){
        $message[]= msgPool::required(_("Password"));
      }
    return($message);
  }


  function save_object()
  {
    plugin::save_object();

    foreach($this->attributes as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = get_post($attr);
      }
    }
  }


  function save()
  {
    plugin::save();

    /* Check if this is a new entry ... add/modify */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->dn,array("objectClass"));
    if($ldap->count()){
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
    }else{
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
    }
    if($this->initially_was_account){
      new log("modify","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      $this->handle_post_events("modify");
    }else{
      $this->handle_post_events("add");
      new log("create","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
  }


  /* Remove service */
  function remove_from_parent()
  {
    goService::remove_from_parent();
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Logs service"),
          "plDescription" => _("Logs service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 99,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
            "gosaLogDB" => _("Database"),
            "goLogAdmin"     => _("User"),
            "goLogPassword" => _("Password"),
            "viewEntries"     => _("Allow view of entries on this server"))
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
