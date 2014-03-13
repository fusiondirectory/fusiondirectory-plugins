<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class askClassName extends plugin
{
  /* attribute list for save action */
  var $ignore_account     = TRUE;
  var $attributes         = array();
  var $objectclasses      = array("whatever");

  var $objectClass        = false;
  var $forbidden          = array();
  var $ClassName          = "";
  var $ClassAlreadyExists = false;
  var $parent;

  function askClassName (&$config,$dn,$ui,$objectClass)
  {
    $this->ui = $ui;
    $this->objectClass = $objectClass;
    parent::__construct($config, $dn);
  }

  function execute()
  {
    /* Call parent execute */
    parent::execute();

    /* Fill templating stuff */
    $smarty = get_smarty();
    $display= "";

    /* First search for every ou, in given fai base
     * With a second search detect all object that belong to the differents ou.
     */

    $base = $this->parent->fai_release;
    $ldap = $this->config->get_ldap_link();

    $type_acl_mapping = array(
        "FAIpartitionTable"  => "faiPartitionTable",
        "FAIpackageList"     => "faiPackageList",
        "FAIscript"          => "faiScript",
        "FAIvariable"        => "faiVariable",
        "FAIhook"            => "faiHook",
        "FAIprofile"         => "faiProfile",
        "FAItemplate"        => "faiTemplate");

    $filter = "";
    foreach($type_acl_mapping as $map){
      $filter .= "(objectClass=".$map.")";
    }

    $res = FAI::get_all_objects_for_given_base($base,"(|".$filter.")",TRUE);

    $delete = array();
    $used   = array();
    foreach($res as $object){

      /* skip class names with this name */
      if(in_array($this->objectClass,$object['objectClass']) || in_array("FAIprofile",$object['objectClass'])){
        if(isset($object['FAIstate'][0]) && preg_match("/removed$/",$object['FAIstate'][0])){
          continue;
        }
        $delete[] = $object['cn'][0];
      }

      /* Skip profiles */
      if(!in_array("FAIprofile",$object['objectClass'])){
        if(isset($object['cn'])){
          $used[$object['cn'][0]]= $object['cn'][0];
        }
      }
    }

    /* Create headline
     * Depending on the object type we want to create, a different headline will be shown
     */
    switch($this->objectClass) {
      case "FAIpartitionTable":  $str =_("Create new FAI object")." - "._("Partition table");break;
      case "FAIpackageList"   :  $str =_("Create new FAI object")." - "._("Package bundle");break;
      case "FAIscript"        :  $str =_("Create new FAI object")." - "._("Script bundle");break;
      case "FAIvariable"      :  $str =_("Create new FAI object")." - "._("Variable bundle");break;
      case "FAIhook"          :  $str =_("Create new FAI object")." - "._("Hook bundle");break;
      case "FAIprofile"       :  $str =_("Create new FAI object")." - "._("Profile");break;
      case "FAItemplate"      :  $str =_("Create new FAI object")." - "._("Template bundle");break;
      default                 :  $str =_("Create new FAI object");break;
    }
    $smarty->assign("headline",$str);

    /* Save forbidden class names
     */
    $this->forbidden = $delete;

    /* Delete all class names which already have this type of object
     */
    foreach($delete as $del){
      unset($used[$del]);
    }

    /* if there is no class name which is missing for this type
     *  of objectClass, we only can create a new one, disable selectBox
     */
    if(count ($used)==0){
      $smarty->assign("ClassNamesAvailable", " disabled ");
      $smarty->assign("grey", 'style="color:#C0C0C0"');
    }else{
      $smarty->assign("ClassNamesAvailable", "");
      $smarty->assign("grey", "");
    }
    ksort($used);
    $smarty->assign("ClassNames", $used);
    $smarty->assign("ClassName",  $this->ClassName);
    $display.= $smarty->fetch(get_template_path('askClassName.tpl', TRUE));
    return($display);
  }

  /* Get posts and set class name
   */
  function save_object()
  {
    if(isset($_POST['classSelector']) && $_POST['classSelector'] == 1
       && isset($_POST['edit_continue'])){
      $this->ClassName          = $_POST['UseTextInputName'];
      $this->ClassAlreadyExists = true;
    }

    if(isset($_POST['classSelector']) && $_POST['classSelector'] == 2
       && isset($_POST['edit_continue'])){
      $this->ClassAlreadyExists = false;
      $this->ClassName          = $_POST['SelectedClass'];
    }
  }

  /* Check given class name
   */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* Only allow limited set of characters as class name to avoid issues */
    if (!preg_match('/^[A-Za-z0-9_-]*$/', $this->ClassName)) {
      $message[] = msgPool::invalid(_("Name"), $this->ClassName, "/[A-Za-z0-9_-]/");
    }

    if($this->ClassName == ""){
      $message[]= msgPool::required(_("Name"));
    }

    if(in_array($this->ClassName,$this->forbidden)){
      $message[]= msgPool::duplicated(_("Name"));
    }

    return ($message);
  }

  /* Return the class name */
  function save()
  {
    return $this->ClassName;
  }
}

class faiPluginClass extends plugin
{
  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message = parent::check();

    /* Ensure that we do not overwrite an allready existing entry */
    $new_dn   = $this->compute_dn();
    $ldap     = $this->config->get_ldap_link();
    $ldap->cat($new_dn);
    if (($ldap->count() != 0) && ($this->dn != $new_dn)) {
      $message[] = msgPool::duplicated(_("Name"));
    }

    return $message;
  }

  function compute_dn()
  {
    return 'cn='.$this->cn.','.get_ou(get_class($this).'RDN').$this->parent->parent->fai_release;
  }
}

class faiSimplePluginClass extends simplePlugin
{
  protected $base;

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    if ($dn == "new") {
      $ui = get_userinfo();
      $this->base = dn2base(session::global_is_set("CurrentMainBase")?"cn=dummy,".session::global_get("CurrentMainBase"):$ui->dn);
    } else {
      $this->base = dn2base($dn, get_ou(get_class($this).'RDN'));
    }
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message = parent::check();

    /* Ensure that we do not overwrite an allready existing entry */
    $new_dn   = $this->compute_dn();
    if ($this->dn != $new_dn) {
      $ldap     = $this->config->get_ldap_link();
      $ldap->cat($new_dn);
      if ($ldap->count() != 0) {
        $message[] = msgPool::duplicated(_("Name"));
      }
    }

    return $message;
  }

  function compute_dn()
  {
    return 'cn='.$this->cn.','.get_ou(get_class($this).'RDN').$this->base;
  }
}
?>
