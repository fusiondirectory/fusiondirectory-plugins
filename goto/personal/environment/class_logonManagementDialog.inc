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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class logonManagementDialog extends plugin
{
  /* attribute list for save action */
  var $ignore_account       = TRUE;
  var $attributes           = array("LogonName","LogonPriority","LogonLast","LogonOverload","LogonData","LogonDescription");
  var $objectclasses        = array("whatever");
  var $use_existing         = false;  

  var $LogonName        ="";  // Name for the LogonScript
  var $LogonPriority    ="";  // Priority defines the order in which the scripts will be processed 
  var $LogonLast        ="";  // Is this script marked as the last, all following scripts will be skipped
  var $LogonOverload    ="";  // If Overload is activated this script is overlaodable by a group script with same prio
  var $LogonData        ="";  // The script itself
  var $LogonDescription ="";
    
  var $nameIsEditable = true;

  var $real_LogonName = "";

  function logonManagementDialog (&$config, $dn= NULL,$data=false)
  {
    plugin::plugin ($config, $dn);
    
    if($data){
      $this->LogonName        = $data['LogonName'];
      $this->LogonPriority    = $data['LogonPriority'];
      $this->LogonOverload    = $data['LogonOverload'];
      $this->LogonLast        = $data['LogonLast'];
      $this->LogonData        = $data['LogonData'];
      $this->LogonDescription = $data['LogonDescription'];
      $this->nameIsEditable   = false;
      $this->real_LogonName   = $data['LogonName'];
    }
  }

  function execute()
  {
	/* Call parent execute */
	plugin::execute();

    $smarty= get_smarty();
    $display= "";

    if((isset($_POST['StartImport']))&&(isset($_FILES['importFile']))){
      $this->LogonData = file_get_contents($_FILES['importFile']['tmp_name']);
      @unlink($_FILES['importFile']['tmp_name']);
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
      if($this->$attr){
        $smarty->assign($attr."CHK"," checked ");
      }else{
        $smarty->assign($attr."CHK","");
      }
    }
    $prios=array(1,2,3,4,5,6,7,8,9,10);
    $smarty->assign("LogonPrioritys",$prios);
    $smarty->assign("LogonPriorityKeys",$prios);

    if(!$this->nameIsEditable){
      $smarty->assign("LogonNameACL"," disabled ");
    }else{
      $smarty->assign("LogonNameACL","");
    }
  

    $display.= $smarty->fetch(get_template_path('logonManagement.tpl', TRUE,dirname(__FILE__)));
    return($display);
  }

  function save_object()
  {
    if(isset($_POST['dialogissubmitted'])){
      foreach($this->attributes as $attr){

        if(!$this->nameIsEditable && $attr == 'LogonName'){
          continue;
        }
        if(isset($_POST[$attr])){
          $this->$attr = stripslashes($_POST[$attr]); 
        }
      }

      foreach(array("LogonLast","LogonOverload") as $checkBoxes){
        if(isset($_POST[$checkBoxes])){
          $this->$checkBoxes = stripslashes($_POST[$checkBoxes]);
        }else{
          $this->$checkBoxes = "";
        }

      }
    }
  }

  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();
  
    if(preg_match("/[^a-zA-Z]/",$this->LogonName)){
      $message[] = msgPool::invalid(_("Name"),$this->LogonName,"/[a-zA-Z]/");
    }

    if(empty($this->LogonName)){
      $message[] = msgPool::required(_("Script name"));
    }

    if(preg_match("/[^a-z0-9,\.-;:_\(\)!\? ]/i",$this->LogonDescription)){
      $message[] = msgPool::required(_("Description"),$this->LogonDescription,"/[a-z0-9,\.-;:_\(\)!\? ]/i");
    }

    return $message;
  }


  function save()
  {
    $a_return= array();
    foreach($this->attributes as $attr){
      $a_return[$attr]=$this->$attr;
    }  

    if(!$this->nameIsEditable){
      $a_return['LogonName']=$this->real_LogonName;
    }

    return($a_return); 
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
