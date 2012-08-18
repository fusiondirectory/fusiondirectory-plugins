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

class serviceFax extends goService {

  var $cli_summary      = "This plugin is used within the ServerService Plugin \nand indicates that this server supports fax informations.";
  var $cli_description  = "Some longer text\nfor help";
  var $cli_parameters   = array("one" => "one is nice", "two" => "two is even better");

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goFaxServer");
  var $attributes       = array("goFaxAdmin", "goFaxPassword");
  var $StatusFlag       = "goFaxServerStatus";

  /* This class can't be assigned twice so it conflicts with itself */
  var $conflicts        = array("serviceFax");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $acl;

  var $goFaxAdmin         = "";
  var $goFaxPassword      = "";
  var $goFaxServerStatus  = "";
  var $cn                 = "";
  var $view_logged  =FALSE;

  function serviceFax(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("FAX service");
  }


  function execute()
  {
    $smarty = get_smarty();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    /* Assign acls */
    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation ){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    return($smarty->fetch(get_template_path("serviceFax.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields               = goService::getListEntry();
    $fields['Message']    = _("FAX service");
    #$fields['AllowEdit']  = true;
    return($fields);
  }

  function check()
  {
    $message = plugin::check();
    if (empty($this->goFaxAdmin)){
      $message[]= msgPool::required(_("User"));
    }
    if (empty($this->goFaxPassword)){
      $message[]= msgPool::required(_("Password"));
    }
    return($message);
  }


  function save_object()
  {
    if(isset($_POST['serviceFaxPosted'])){
      plugin::save_object();
    }
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("FAX service"),
          "plDescription" => _("FAX service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 99,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
             "goFaxAdmin"     => _("Login name"),
             "goFaxPassword"  => _("Password"))
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
