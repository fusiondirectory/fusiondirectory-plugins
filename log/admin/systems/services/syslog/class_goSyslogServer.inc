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

class goSyslogServer extends goService{
	
  var $cli_summary      = "This plugin is used within the ServerService Pluign \nand indicates that this server has syslog server enabled.";
  var $cli_description  = "Some longer text\nfor help";
  var $cli_parameters   = array("one" => "one is nice", "two" => "two is better");

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goSyslogServer");
  var $attributes       = array();
  var $StatusFlag       = "goSyslogServerStatus";
 
  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("goSyslogServer");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $acl;
  var $cn                    = "";
  var $goSyslogServerStatus  = "";
  var $view_logged  =FALSE;
 
  function goSyslogServer(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Syslog server");
  }


  function execute()
  { 
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }


    $smarty = get_smarty(); 
    return($smarty->fetch(get_template_path("goSyslogServer.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields               = goService::getListEntry();
    $fields['Message']    = _("Syslog server");
    #$fields['AllowEdit']  = false;
    return($fields);
  }

  function check()
  { 
    $message = plugin::check();
    return($message);
  }


  function save_object()
  {
    plugin::save_object();
  } 

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Logging service"),
          "plDescription" => _("Logging service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 88,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array()
          ));
  }


}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
