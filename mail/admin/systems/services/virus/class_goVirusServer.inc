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

class govirusserver extends goService{

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goVirusServer");
  var $attributes       = array("avMaxThreads","avMaxDirectoryRecursions","avUser","avFlags","avArchiveMaxFileSize","avArchiveMaxRecursion",
                                "avArchiveMaxCompressionRatio","avDatabaseMirror","avChecksPerDay","avHttpProxyURL");
  var $StatusFlag       = "avStatus";

  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("goVirusServer");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $cn               = "";
  var $avStatus         = "";

  var $ui               = NULL;

  var $Flags            = array("D","S","A","E");

  var $avFlags          = "DS";
  var $avFlagsD         = TRUE;
  var $avFlagsS         = TRUE;
  var $avFlagsA         = FALSE;
  var $avFlagsE         = FALSE;

  var $avMaxThreads                 = 5;
  var $avMaxDirectoryRecursions     = 4;
  var $avArchiveMaxFileSize         = 4000;
  var $avArchiveMaxRecursion        = 5;
  var $avArchiveMaxCompressionRatio = 95;
  var $avChecksPerDay               = 12;

  var $avUser                       = "";
  var $avHttpProxyURL               = "";
  var $avDatabaseMirror             = "";
  var $view_logged  =FALSE;

  function govirusserver(&$config,$dn, $parent= NULL)
  {
    /* Init class */
    goService::goService($config, $dn, $parent);
    $this->DisplayName = _("Antivirus service");

    /* Get userinfo & acls */
    $this->ui = get_userinfo();

    /* Get Flags */
    foreach($this->Flags as $flag){
      $var = "avFlags".$flag;
      if(preg_match("/".$flag."/",$this->avFlags)){
        $this->$var = TRUE;
      }
    }
  }


  function execute()
  {
    $smarty = get_smarty();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }


    /* Set acls */
    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation) {
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    $display = "";
    $smarty->assign("servtabs",FALSE);
    $this->is_account = true;

    /* Assign smarty vars */
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    /* Assign checkbox states */
    foreach($this->Flags as $Flag){
      $var = "avFlags".$Flag;
      if($this->$var){
        $smarty->assign("avFlags".$Flag."CHK"," checked " );
      }else{
        $smarty->assign("avFlags".$Flag."CHK","");
      }
    }

    /* Assign value for max thread select box */
    $tmp = array();
    for($i = 1 ; $i <= 20 ; $i ++){
      $tmp[$i] = $i;
    }
    $smarty->assign("ThreadValues",$tmp);

    if($this->avFlagsA){
      $smarty->assign("avFlagsAState" , "" );
    }else{
      $smarty->assign("avFlagsAState" , " disabled " );
    }

    return($display.$smarty->fetch(get_template_path("goVirusServer.tpl",TRUE,dirname(__FILE__))));
  }


  function save()
  {
    if(!$this->is_account) return;

    /* Create Flags */
    $this->avFlags = "";
    foreach($this->Flags as $flag){
      $var = "avFlags".$flag;
      if($this->$var){
        $this->avFlags .=$flag;
      }
    }

    plugin::save();

    if(!$this->avFlagsA){
      $arr = array("avArchiveMaxFileSize","avArchiveMaxRecursion","avArchiveMaxCompressionRatio");
      foreach($arr as $attr){
        $this->attrs[$attr] =  array();
      }
      $this->attrs['avFlags'] = preg_replace("/E/","",$this->attrs['avFlags']);
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
    if($this->initially_was_account){
      $this->handle_post_events("modify");
      new log("modify","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      $this->handle_post_events("add");
      new log("create","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
  }

  function check()
  {
    $message = plugin::check();

    $mustBeNumeric = array(
          "avMaxDirectoryRecursions"     =>_("Maximum directory recursions"),
          "avMaxThreads"                 =>_("Maximum threads"),
          "avArchiveMaxFileSize"         =>_("Maximum file size"),
          "avArchiveMaxRecursion"        =>_("Maximum recursions"),
          "avArchiveMaxCompressionRatio" =>_("Maximum compression ratio"),
          "avChecksPerDay"               =>_("Checks per day"));

    foreach($mustBeNumeric as $key => $trans){
      if(!is_numeric($this->$key)){
        $message[] = msgPool::invalid($trans,$this->$key,"/[0-9]/");
      }
    }

    foreach(array("avUser"=>_("Database user"),"avHttpProxyURL"=>_("Http proxy URL"),"avDatabaseMirror"=>_("Database mirror")) as $attr => $name){
      if(!preg_match("/^[a-z0-9:_\-\.\/]*$/",$this->$attr)){
        $message[] = msgPool::invalid($name,$this->$attr,"/[a-z0-9:_\-\.\/]/");
      }
    }

    return($message);
  }


  function save_object()
  {
    if(isset($_POST['goVirusServer'])){
      plugin::save_object();
      foreach($this->Flags as $flag){

        $var = "avFlags".$flag;
        if($this->acl_is_writeable($var)){
          if(isset($_POST[$var])){
            $this->$var = TRUE;
          }else{
            $this->$var = FALSE;
          }
        }
      }
    }
  }


  /* For newer service management dialogs */
  function getListEntry()
  {
    $fields       = goService::getListEntry();
    #$fields['AllowEdit']    = true;
    $fields['Message']      = _("Antivirus service");
    return($fields);
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Antivirus service"),
          "plDescription" => _("Antivirus service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 96,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),
          "plProvidedAcls"=> array(

          "start"         => _("Start"),
          "stop"          => _("Stop"),
          "restart"       => _("Restart"),

          "avFlagsD"         =>_("Enable debugging"),
          "avFlagsS"         =>_("Enable mail scanning"),
          "avFlagsA"         =>_("Enable scanning of archives"),
          "avFlagsE"         =>_("Block encrypted archives"),

          "avMaxThreads"                 =>_("Maximum threads"),
          "avMaxDirectoryRecursions"     =>_("Maximum directory recursions"),
          "avUser"                       =>_("Anti virus user"),
          "avArchiveMaxFileSize"         =>_("Maximum file size"),
          "avArchiveMaxRecursion"        =>_("Maximum recursions"),
          "avArchiveMaxCompressionRatio" =>_("Maximum compression ratio"),
          "avDatabaseMirror"             =>_("Database mirror"),
          "avChecksPerDay"               =>_("Checks per day"),
          "avHttpProxyURL"               =>_("Http proxy URL"))
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
