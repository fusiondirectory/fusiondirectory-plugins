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

class SelectDeviceType extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes= array("ObjectGroup","SystemType");
  var $objectclasses= array("whatever");

  var $ObjectGroups   = array();
  var $SystemTypes       = array();

  var $ObjectGroup    = "none";
  var $SystemType        = "gotoWorkstation";

  var $dn;

  function SelectDeviceType  (&$config, $dn= NULL)
  {
    if(!is_array($dn)){
      $this->dns = array($dn);
    }else{
      $this->dns = $dn;
    }
    plugin::plugin ($config, NULL);

    /* Get object groups */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gosaGroupOfNames)(cn=*))",array("cn"));
    $tmp= array();
    while($attrs = $ldap->fetch()){
      $tmp[$attrs['dn']]= $attrs['cn'][0];
    }
    asort($tmp, SORT_LOCALE_STRING);
    $this->ObjectGroups= $tmp;

    $this->SystemTypes =array("gotoWorkstation"=>_("Workstation"), "gotoTerminal"=>_("Terminal"), "goServer"=>_("Server"));
    if(class_available("opsi")){
      $this->SystemTypes["FAKE_OC_OpsiHost"]= _("Windows workstation");
    }
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();


    /* Get object groups */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gosaGroupOfNames)(cn=*))",array("gosaGroupObjects","cn"));
    $tmp= array("W" => array(),"T" => array(), "S" => array(),"O" => array());
    while($attrs = $ldap->fetch()){
      $tmp[preg_replace("/[\[\] ]/","",$attrs['gosaGroupObjects'][0])][$attrs['dn']] = $attrs['cn'][0];
    }

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

    $map = array("gotoWorkstation" => "W","gotoTerminal" => "T","goServer" => "S", "FAKE_OC_OpsiHost" => "O");
    $smarty->assign("dns_cnt" , count($this->dns));
    $smarty->assign("ogroups", $tmp[$map[$this->SystemType]]);
    $smarty->assign("SystemTypes"     ,$this->SystemTypes);
    $smarty->assign("SystemTypeKeys"  ,array_flip($this->SystemTypes)); 
    $smarty->assign("ObjectGroup",$this->ObjectGroup);
    $smarty->assign("SystemType",$this->SystemType);
    $display.= $smarty->fetch(get_template_path('SelectDeviceType.tpl', TRUE,dirname(__FILE__)));
    return($display);
  }

  /* Save data to object */
  function save_object()
  {
    plugin::save_object();
    foreach($this->attributes as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = $_POST[$attr];
      }
    }
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    return ($message);
  }

  /* Save to LDAP */
  function save()
  {
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
