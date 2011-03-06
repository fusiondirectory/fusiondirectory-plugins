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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class goLogDBServer extends goService{
	
  var $cli_summary      = "This plugin is used within the ServerService Plugin \nand indicates that this server supports log management.";
  var $cli_description  = "Some longer text\nfor help";
  var $cli_parameters   = array("one" => "One is nice", "two" => "two is better");

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goLogDBServer");
  var $attributes       = array("goLogAdmin", "goLogPassword" , "gosaLogDB");
  var $StatusFlag       = "goLogDBServerStatus";

  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("goLogDBServer");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $acl;
  var $cn                   = "";
  var $goLogDBServerStatus  = "";
  var $goLogAdmin           = "";
  var $goLogPassword        = "";  
  var $gosaLogDB            = "gomon";  
  var $view_logged  =FALSE;


  function goLogDBServer(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Syslog service database");
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
    return($smarty->fetch(get_template_path("goLogDBServer.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields = goService::getListEntry();
    $fields['Message']      = _("Syslog service database");
    #$fields['AllowEdit']    = true;
    return($fields);
  }


  function check()
  { 
    $message = plugin::check();
    if (empty($this->goLogAdmin)){
      $message[]= msgPool::required(_("User"));
    }
    if (empty($this->goLogPassword)){
      $message[]= msgPool::required(_("Password"));
    }
    if (empty($this->gosaLogDB)){
      $message[]= msgPool::required(_("Database"));
    }
    return($message);
  }


  function save_object()
  {
    if(isset($_POST['goLogDBServerPosted'])){
      plugin::save_object();
    }
  } 

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Logging database"),
          "plDescription" => _("Logging database")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 97,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
            "start"         => _("Start"),
            "stop"          => _("Stop"),
            "restart"       => _("Restart"),
            "goLogAdmin"    => _("Admin"),
            "goLogPassword" => _("Password"),
            "gosaLogDB"       => _("Database"))
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
