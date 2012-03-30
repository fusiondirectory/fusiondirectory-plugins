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

class argonautServer extends goService{

  var $cli_summary      = "This plugin is used within the ServerService Pluign \nand indicates that this server supports Argonaut service.";
  var $cli_description  = "Some longer text\nfor help";
  var $cli_parameters   = array("one" => "one is nice", "two" => "two is better");

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("argonautServer");
  var $attributes       = array("argonautDeleteFinished","argonautPort",
                                "argonautWakeOnLanInterface","argonautIpTool","argonautLogDir");
  var $StatusFlag       = "argonautServerStatus";

  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("argonautServer");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $acl;
  var $cn               = "";
  var $view_logged  =FALSE;
  
  var $argonautDeleteFinished     = true;
  var $argonautPort               = 8080;
  var $argonautWakeOnLanInterface = "eth0";
  var $argonautIpTool             = "/sbin/ifconfig";
  var $argonautLogDir             = "/var/log/argonaut";
  
  var $argonautServerStatus = "";

  function argonautServer(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Argonaut service");
    $this->argonautDeleteFinished = ($this->argonautDeleteFinished == "TRUE"); // conversion from LDAP format to PHP boolean
  }


  function execute()
  { 
    $smarty = get_smarty(); 

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    $smarty->assign("argonautACL", $this->getacl("argonaut")) ;
    foreach ($this->attributes as $attribute) {
      $smarty->assign("$attribute" , $this->$attribute);
    }
    return($smarty->fetch(get_template_path("argonautServer.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields               = goService::getListEntry();
    $fields['Message']    = _("Argonaut service");
    #$fields['AllowEdit']  = true;
    return($fields);
  }


  function save()
  {
    plugin::save();
    $this->attrs['argonautDeleteFinished'] = ($this->argonautDeleteFinished?"TRUE":"FALSE"); // conversion from PHP format to LDAP boolean
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
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
    if($this->initially_was_account){
      $this->handle_post_events("modify");
      new log("modify","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      $this->handle_post_events("add");
      new log("create","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }
  }

  
  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);
  }

  function check()
  { 
    $message = plugin::check();
    return($message);
  }

  function save_object()
  {
    if(isset($_POST['argonautServerPosted'])) {
      plugin::save_object();
      if($this->acl_is_writeable("argonaut")) {
        if(isset($_POST['argonautDeleteFinished'])) {
          $this->argonautDeleteFinished = true;
        } else {
          $this->argonautDeleteFinished = false;
        }
        foreach ($this->attributes as $attribute) {
          if($attribute=="argonautDeleteFinished") {
            continue;
          }
          if(isset($_POST[$attribute])) {
            $this->$attribute = $_POST[$attribute];
          }
        }
      }
    }
  }
  
  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Argonaut service"),
          "plDescription" => _("Argonaut service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 91,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
            "argonaut"        =>_("Modify argonaut service"))
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
