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

class goNtpServer extends goService{

  var $cli_summary      = "This plugin is used within the ServerService Pluign \nand indicates that this server supports NTP service.";
  var $cli_description  = "Some longer text\nfor help";
  var $cli_parameters   = array("one" => "one is nice", "two" => "two is better");

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goNtpServer");
  var $attributes       = array("goTimeSource");
  var $StatusFlag       = "goNtpServerStatus";

  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("goNtpServer");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $goTimeSource     = array();
  var $goNtpServerStatus= "";
  var $acl;
  var $cn               = "";
  var $view_logged  =FALSE;

  function goNtpServer(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Time service (NTP)");

    /* Load arrays */
    $tmp = array();
    if (isset($this->attrs['goTimeSource'])){
      for ($i= 0; $i<$this->attrs['goTimeSource']['count']; $i++){
        $tmp[$this->attrs['goTimeSource'][$i]]= $this->attrs['goTimeSource'][$i];
      }
    }
    $this->goTimeSource= $tmp;
  }


  function execute()
  { 
    $smarty = get_smarty(); 

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    /* Here we add a new entry  */
    if(isset($_POST['NewNTPAdd']) && $_POST['NewNTPExport'] != "" && $this->acl_is_writeable("goTimeSource")) {
      $this->goTimeSource[$_POST['NewNTPExport']]= $_POST['NewNTPExport'];
      asort($this->goTimeSource);
    }

    /* Deleting an Entry, is a bit more complicated than adding one*/
    if((isset($_POST['DelNTPEnt'])) && (isset($_POST['goTimeSource'])) && $this->acl_is_writeable("goTimeSource")) {
      foreach ($_POST['goTimeSource'] as $entry){
        if (isset($this->goTimeSource[$entry])){
          unset($this->goTimeSource[$entry]);
        }
      }
    }

    $smarty->assign("goTimeSourceACL", $this->getacl("goTimeSource")) ;
    $smarty->assign("goTimeSource" , $this->goTimeSource); 
    return($smarty->fetch(get_template_path("goNtpServer.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields               = goService::getListEntry();
    $fields['Message']    = _("Time service (NTP)");
    #$fields['AllowEdit']  = true;
    return($fields);
  }


  function save()
  {
    plugin::save();
    $this->attrs['goTimeSource'] =array();
    foreach($this->goTimeSource as $entry){
      $this->attrs['goTimeSource'][] = $entry;
    }
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

    /* Load arrays */
    $tmp = array();
    if (isset($source['goTimeSource'])){
      for ($i= 0; $i<$source['goTimeSource']['count']; $i++){
        $tmp[$source['goTimeSource'][$i]]= $source['goTimeSource'][$i];
      }
    }
    $this->goTimeSource= $tmp;


  }

  function check(){ return array();}

  function save_object(){;}
  
  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Time service"),
          "plDescription" => _("Time service - NTP")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 91,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
            "goTimeSource"        =>_("Ntp source"))
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
