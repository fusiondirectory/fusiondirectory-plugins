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

class faiScriptEntry extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes   = array("cn","description","FAIpriority","FAIscript");
  var $objectclasses= array();

  var $orig_cn              = "";

  var $dn            = "";
  var $cn            = "";
  var $FAIpriority   = "0";
  var $FAIscript     = "";
  var $description   = "";
  var $status        = "new";
  var $parent        = NULL;
  var $FAIstate      = "";
  
  function faiScriptEntry (&$config, $dn= NULL,$object=false)
  {
    plugin::plugin ($config, NULL);
    if($dn != "new"){
      $this->orig_cn= $object['cn'];
      $this->dn=$object['dn'];
      foreach($object as $name=>$value){
        $oname = $name;
        $this->$oname=$value;
      }
    }else{
      if(is_array($object)&&count($object)){
        $this->orig_cn= $object['cn'];
        $this->dn=$object['dn'];
        foreach($object as $name=>$value){
          $oname = $name;
          $this->$oname=$value;
        }
      }else{

        $this->status = "new";
        $this->orig_cn       = false;
      }
    }
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Fill templating stuff */
    $smarty     = get_smarty();
    $smarty->assign("freeze", preg_match("/freeze/i",$this->FAIstate));
    $display = "";
  
    if(isset($_POST['ImportUpload']) && $this->acl_is_writeable("FAIscript")){
      if(($_FILES['ImportFile']['error']!=0)){
        msg_dialog::display(_("Error"), msgPool::incorrectUpload(), ERROR_DIALOG);
      }else
      if(($_FILES['ImportFile']['size']==0)){
        msg_dialog::display(_("Error"), msgPool::incorrectUpload(_("file is empty")), ERROR_DIALOG);
      }else{
        $str = utf8_encode(addslashes(file_get_contents($_FILES['ImportFile']['tmp_name'])));
        $str = preg_replace("/[\r]/","",$str);
        $this->FAIscript = $str;
      }
    }
  
    foreach($this->attributes as $attrs){
      $smarty->assign($attrs,($this->$attrs));
    }

    /* File download requested */
    if(isset($_GET['getFAIScript'])){
      send_binary_content(stripslashes($this->FAIscript),$this->cn.".FAIscript");
    }

    /* Create download button*/
    if($this->dn != "new" && $this->acl_is_readable("FAIscript")){
      $smarty->assign("DownMe","<a href='?plug=".$_GET['plug']."&getFAIScript'>
        <img src='images/save.png' alt='"._("Download")."' title='"._("Download")."' border=0 class='center'>
        </a>");
    }else{
      $smarty->assign("DownMe","");
    }

    for($i =0 ; $i < 100 ; $i++){
      $FAIprioritys[$i]=$i;
    }

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translated){
      $acl = $this->getacl($name,preg_match("/freeze/i",$this->FAIstate));
      $smarty->assign($name."ACL",$acl);
    }

    $smarty->assign("FAIscript" , htmlentities($this->FAIscript));
    $smarty->assign("FAIprioritys",$FAIprioritys);
    $display.= $smarty->fetch(get_template_path('faiScriptEntry.tpl', TRUE));
    return($display);
  }

  /* Save data to object */
  function save_object()
  {
    if((isset($_POST['SubObjectFormSubmitted'])) && !preg_match("/freeze/", $this->FAIstate)){
      foreach($this->attributes as $attrs){
        if($this->acl_is_writeable($attrs)){
          if(isset($_POST[$attrs])){
            $this->$attrs = $_POST[$attrs];
          }else{
            $this->$attrs = "";
          }
        }
      }
    }
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();
  
    if(isset($this->parent->SubObjects[$this->cn]) && $this->cn != $this->orig_cn){
      $message[] = msgPool::duplicated(_("Name"));
    }

    $c = trim($this->cn);
    if($c == ""){
      $message[] = msgPool::required(_("Name"));
    }
    if(preg_match("/[^a-z0-9_\-]/i",$c)){
      $message[] = msgPool::invalid(_("Name"),$c,"/[a-z0-9_\-]/i");
    }

    $s = trim($this->FAIscript);
    if($s == ""){
      $message[]= msgPool::required(_("Script"));
    }

    return ($message);
  }
 
  function save()
  {
    $tmp=array();
    foreach($this->attributes as $attrs){ 
      $tmp[$attrs] = $this->$attrs;
    }

    /* Strip out dos newlines */
    $tmp['FAIscript']= strtr($this->FAIscript, array("\x0D" => ""));

    if(($this->orig_cn)&&($tmp['cn']!=$this->orig_cn)){
      $tmp['remove']['from']  = $this->orig_cn;
      $tmp['remove']['to']    = $tmp['cn'];
    }
  
    $tmp['dn']      = $this->dn;  
    $tmp['status']  = $this->status; 
    return($tmp);
  }

  
  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName" => _("Script entry"),
          "plDescription" => _("FAI script entry"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 19,
          "plSection"     => array("administration"),
          "plCategory"    => array("fai"),
          "plProvidedAcls" => array(
            "cn"              => _("Name"),
            "description"     => _("Description"),
            "FAIscript"       => _("Script entry"),
            "FAIpriority"     => _("Script Priority"))
          ));
  }


}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
