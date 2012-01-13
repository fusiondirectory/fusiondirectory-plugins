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

class faiHookEntry extends plugin
{

  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes   = array("cn","description","FAIscript","FAItask");
  var $objectclasses= array();

  var $orig_cn              = "";
  var $tasks                = array("chboot", "configure", "debconf", "extrbase", "faiend", "finish",
                                    "install", "instsoft", "mirror", "mountdisks", "partition", "prepareapt",
                                    "savelog", "softupdate", "sysinfo","updatebase", "error");
  var $dn            = "";
  var $cn            = "";
  var $FAItask       = "chboot";
  var $FAIscript     = "";
  var $description   = "";
  var $status        = "new";
  var $parent        = NULL;
  var $FAIstate      = "";
  
  function faiHookEntry (&$config, $dn= NULL,$object=false)
  {
    plugin::plugin ($config, NULL);
    if($dn != "new"){
      $this->orig_cn= $object['cn'];
      $this->dn=$object['dn'];
      foreach($object as $name=>$value){
        $oname = $name;
        $this->$oname=$value;
      }
    }elseif(is_array($object)){
      if(count($object)){
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
    $display = "";
        
    if(isset($_POST['ImportUpload'])){
      if(($_FILES['ImportFile']['error']!=0)){
        msg_dialog::display(_("Error"), msgPool::incorrectUpload(), ERROR_DIALOG);
      }else
      if(($_FILES['ImportFile']['size']==0)){
        msg_dialog::display(_("Error"), msgPool::incorrectUpload(_("file is empty")), ERROR_DIALOG);
      }else{
        $str = utf8_encode(file_get_contents($_FILES['ImportFile']['tmp_name']));
        $str = preg_replace("/[\r]/","",$str);
        $this->FAIscript = addslashes ($str);
      }
    }

    /* File download requested */
    if(isset($_GET['getFAIHook'])){
      send_binary_content(stripslashes($this->FAIscript),$this->cn.".FAIhook");
    }

    /* Create download button*/
    if($this->dn != "new" && $this->acl_is_readable("FAIscript")){
      $smarty->assign("DownMe","<a href='?plug=".$_GET['plug']."&getFAIHook'>
          <img src='images/save.png' alt='"._("Download")."' title='"._("Download")."' border=0 class='center'>
        </a>");
    }else{
      $smarty->assign("DownMe","");  
    }

    $used_tasks = $this->parent->getUsedFAItask($this->cn);
    $tasks = $this->tasks;
    foreach($this->tasks as $id => $task){
      if(in_array($task,$used_tasks)){
        unset($tasks[$id]);
      }
    }
    $smarty->assign("tasks", $tasks);

     /* Magic quotes GPC, escapes every ' " \, to solve some security risks
     * If we post the escaped strings they will be escaped again
     */
    foreach($this->attributes as $attrs){
      if(get_magic_quotes_gpc()){
        $smarty->assign($attrs,(stripslashes($this->$attrs)));
      }else{
        $smarty->assign($attrs,($this->$attrs));
      }
    }

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translated){
      $acl = $this->getacl($name, preg_match("/freeze/",$this->FAIstate));
      $smarty->assign($name."ACL",$acl);
    }
   
    if(get_magic_quotes_gpc()){
      $smarty->assign("FAIscript" , htmlentities(stripslashes($this->FAIscript)));
    }else{
      $smarty->assign("FAIscript" , htmlentities($this->FAIscript));
    }
    $smarty->assign("freeze" , preg_match("/freeze/",$this->FAIstate));
    $display.= $smarty->fetch(get_template_path('faiHookEntry.tpl', TRUE));
    return($display);
  }

  /* Save data to object */
  function save_object()
  {
    if((isset($_POST['SubObjectFormSubmitted'])) && !preg_match("/^freeze/", $this->FAIstate)){
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
      $message[]= msgPool::duplicated(_("Name"));
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
          "plShortName" => _("Hook entry"),
          "plDescription" => _("FAI hook entry"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 21,
          "plSection"     => array("administration"),
          "plCategory"    => array("fai"),
          "plProvidedAcls" => array(
            "cn"                => _("Name"),
            "description"       => _("Description"),
            "FAItask"           => _("Task"),
            "FAIscript"         => _("FAI script"))
          ));
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
