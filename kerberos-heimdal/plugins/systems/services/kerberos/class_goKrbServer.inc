<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2010 Antoine Gallavardin
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


/*! brief   The kerberos service class 
 */
class goKrbServer extends goService{
  
  var $cli_summary      = "This pluign is used within the ServerService Pluign \nand indicates that this server supports NTP service.";
  var $cli_description  = "Some longer text\nfor help";
  var $cli_parameters   = array("eins" => "Eins ist toll", "zwei" => "Zwei ist noch besser");

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goKrbServer");
  var $attributes       = array("goKrbRealm");
  //var $StatusFlag       = "goKrbServerStatus";
 
  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("goKrbServer");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $goKrbServerStatus= "";
  var $cn               = "";
  var $goKrbRealm       = "";
  var $view_logged      = FALSE;

  var $policies         = array();
  var $id               = -1;
  var $macAddress       = "" ;


  /*! \brief Initializes the kerberos service 
      @param  Object  FusionDirectory configuration object.
      @param  String  The object dn we are currently editing.
      @param  Object  The parent object.
   */ 
  public function goKrbServer(&$config,$dn,$parent)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Kerberos service");
    $this->parent = $parent;

    /* Detect macAddress of this device 
     */
    $this->macAddress = "";
    if(isset($this->parent->parent->netConfigDNS->macAddress)){
      $this->macAddress = &$this->parent->parent->netConfigDNS->macAddress;
    }
  }

  /*! \brief  Create HTML output
      @return String HMTL output.
   */
  public function execute()
  { 
    $smarty = get_smarty(); 

    if(!$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    /*******
      Create HTML output for this plugin
     *******/  

    $tmp = $this->plinfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $divlist = new divSelectBox("Kerberos Policies");
    $action  = "<input type='image' src='images/lists/edit.png' class='center' name='policy_edit_%id'>";
    $action .= "<input type='image' src='images/lists/trash.png' class='center' name='policy_del_%id'>";
    foreach($this->policies as $key => $policy){
      if($policy['STATUS'] == "REMOVED") continue;

      $actions = preg_replace("/%id/",$key,$action);
      $field1 = array("string" => $policy['NAME']);
      $field3 = array("string" => $actions, 
                      "attach"=>"style='width:40px; text-align:right; border:0px;'");

      $divlist->AddEntry(array($field1,$field3));
    }

    $smarty->assign("divlist",$divlist->DrawList());
    return($smarty->fetch(get_template_path("goKrbServer.tpl",TRUE,dirname(__FILE__))));
  }


  /* \brief  Return service informations, which will be shown in the service overview.
     @return  Array  Some service information.
   */
  public function getListEntry()
  {
    $fields               = goService::getListEntry();
    $fields['Message']    = _("Kerberos service (kadmin access informations)");
    $fields['AllowEdit']  = true;
    return($fields);
  }

  /*! \brief Checks if all given values are valid 
      @return Array   An array containing all error messages.
   */
  public function check()
  { 
    $message = plugin::check();
    if (empty($this->goKrbRealm)){
      $message[]= msgPool::required(_("Realm"));
    }
    if(count($this->policies) && (empty($this->macAddress) || !tests::is_mac($this->macAddress))){
      $message[] = _("In order to save policy changes, the server requires a valid mac address.");
    }
    return($message);
  }
  
  /*! \brief Save changes to ldap
   */
  public function save()
  {
    goService::save();

  }

  /*! \brief Return plugin informations for acl handling 
      @return Array   ACL infos.
   */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Kerberos"),
          "plDescription" => _("Kerberos access information")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 95,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
            "goKrbPolicy"   => _("Policies"),
            "goKrbRealm"    => _("Realm")) 
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
