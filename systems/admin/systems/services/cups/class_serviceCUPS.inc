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

class serviceCUPS extends goService {
  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goCupsServer");
  var $attributes       = array();
  var $StatusFlag       = "serviceCUPSStatus";

  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("serviceCUPS");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $acl;
  var $cn                  = "";
  var $serviceCUPSStatus  = "";
  var $view_logged  =FALSE;

  function serviceCUPS(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Print service");
  }


  function execute()
  {
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    $smarty = get_smarty();
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    return($smarty->fetch(get_template_path("serviceCUPS.tpl",TRUE,dirname(__FILE__))));
  }

  function getListEntry()
  {
    $fields = goService::getListEntry();
    $fields['Message']    = _("Print service");
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
    return array(
      "plShortName"   => _("Cups"),
      "plDescription" => _("Print service")." ("._("Services").")",
      "plIcon"        => "plugins/systems/images/service_print.png",
      "plSelfModify"  => FALSE,
      "plPriority"    => 100,
      "plCategory"    => array("server"),

      "plProvidedAcls"=> array(
        "start"         => _("Start"),
        "stop"          => _("Stop"),
        "restart"       => _("Restart"),
      )
    );
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
