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

class goKioskService extends goService{
	
  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goEnvironmentServer");
  var $attributes       = array("gotoKioskProfile");
  var $StatusFlag       = "";
 
  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("goKioskService");

  var $baseDir          = "../kiosk";
  var $filesToAttach    = array();
  var $cn               = "unknown";  
  var $orig_cn          = "unknown";  
  var $gotoKioskProfiles= array();
  var $gotoKioskProfile = array();
  var $server_path      = "http://%cn/kiosk";
 
  function goKioskService(&$config, $dn, $parent)
  {
    goService::goService($config,$dn);

    /* Get the current object name. 
     */
    if(isset($parent->parent->cn)){
      $this->cn = &$parent->parent->cn;
    }elseif(isset($this->attrs['cn'][0])){
      $this->cn = $this->attrs['cn'][0];
    }
    $this->orig_cn = $this->cn;

    $this->DisplayName = _("Kiosk profile service");
    $this->baseDir = $this->config->search('environment', 'kioskpath',array('menu','tabs'));
    $this->server_path = preg_replace("/%cn/", $this->cn, $this->server_path);

    /* Load list of profiles and check if they still exists */
    if ($this->baseDir == ""){
#      msg_dialog::display(_("Configuration error"), 
#      msgPool::cmdnotfound("KIOSKPATH",_("Kiosk profile service")), ERROR_DIALOG);
    }else{
      $this->gotoKioskProfiles = array();
      if(isset($this->attrs['gotoKioskProfile']) && is_array($this->attrs['gotoKioskProfile'])){
        for($i = 0 ; $i < $this->attrs['gotoKioskProfile']['count']; $i ++){
          $url = $this->attrs['gotoKioskProfile'][$i];
          $this->server_path = preg_replace("/\/[^\/]*$/","",$url);
          $name= preg_replace("/^.*\//","",$url);
          $this->gotoKioskProfiles[] = array('url'     => $url , 
                                            'name'    => $name , 
                                            'initial' =>TRUE,
                                            'exists'  => file_exists($this->baseDir."/".$name));
        }
      }
    }
  }


  function remove_from_parent()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gotoEnvironment)(gotoKioskProfile=*/".$this->orig_cn."/*))",array("cn"));
    if ($ldap->count() != 0){
      $obj= array();
      while($attrs = $ldap->fetch()){
        $obj[$ldap->getDN()]= $attrs['cn'][0];
      }    
      msg_dialog::display(_("Error"), msgPool::stillInUse(_("Kiosk profile service"), msgPool::buildList($obj)), ERROR_DIALOG);
    }else{
      goService::remove_from_parent();
    }
  }
  
  
  function execute()
  { 
    /* log actions */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    /* Fill templating stuff */
    $smarty= get_smarty();
    $smarty->assign("baseDir",$this->baseDir);
    $smarty->assign("ThisACL",$this->getacl(""));
    $display= "";

    /* Download kiosk profile 
     */
    if(isset($_GET['getkiosk'])){
      $id = $_GET['getkiosk'];

      foreach($this->gotoKioskProfiles as $profile){
        if($profile['name'] == $id){
          $name = $this->baseDir."/".$id;
          if(file_exists($name)){
            $content = file_get_contents($name);    
            send_binary_content($content,$id);
          }
        }
      }
    }
    

    /* Add new kiosk profile
     * in profile directory ($this->baseDir);
     */
    if((isset($_POST['profileAdd']))&&(isset($_FILES['newProfile'])) && $this->acl_is_writeable("")){
      $file = $_FILES['newProfile'];
      if(!file_exists($this->baseDir.$file['name'])){
        $tmp = array(
            'url'     => "" ,
            'name'    => $file['name'] ,
            'initial' => FALSE,
            'tmp_name'=> $file['tmp_name'],
            'content' => file_get_contents($file['tmp_name']),
            'exists'  => TRUE);
        $this->gotoKioskProfiles[] = $tmp;
      }
    }

    $only_once = true;
    foreach($_POST as $name => $value){

      if((preg_match("/^delkiosk_/",$name))&&($only_once) && $this->acl_is_writeable("")){
  
        /* Get id, name and path */
        $only_once = false;
        $id = preg_replace("/^delkiosk_/","",$name);
        $id = preg_replace("/_.*$/","",$id);
        $name = $this->gotoKioskProfiles[$id]['name'];
        $filename = $this->baseDir."/".$name;

        /* check if profile is still in use */
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(&(objectClass=gotoEnvironment)(gotoKioskProfile=*".$name.")",array("cn","uid","gotoKioskProfile"));

        if ($ldap->count() != 0){

          $obj= array();
          $found= false;
          while(($attrs = $ldap->fetch()) && ($cnt)){
            $check = preg_replace("/^.*\//i","",$attrs['gotoKioskProfile'][0]);
            if($check == $name){
              $obj[$ldap->getDN()]= $attrs['cn'][0];
              $found= true;
            }
          }
 
          if ($found){
            msg_dialog::display(_("Error"), msgPool::stillInUse(_("Kiosk profile"), msgPool::buildList($obj)), ERROR_DIALOG);
          }
        }else{
          if($this->gotoKioskProfiles[$id]['initial']){
            $res = @unlink($filename);
            if(!$res){
              if(!is_writeable($filename)){
                msg_dialog::display(_("Error"), msgPool::cannotDeleteFile($filename), ERROR_DIALOG);
              }
              if(!file_exists($filename)){
                msg_dialog::display(_("Error"), msgPool::fileDoesNotExist($filename), ERROR_DIALOG);
              }
            }
            unset($this->gotoKioskProfiles[$id]);
          }
        }
      }
    }

    /* Display list of profiles */
    $divlist = new divSelectBox("KioskProfiles");
    $divlist -> SetHeight (300);


    foreach($this->gotoKioskProfiles as $key => $val ){
      $del = "&nbsp;";
      if($this->acl_is_writeable("")){
        $del = "<input type='image' src='images/lists/trash.png'
          class='center' alt='delete' name='delkiosk_".$key."'>";
      }
      $divlist->AddEntry(array(
            array("string"=>"<a href='?plug=".$_GET['plug']."&getkiosk=".$val['name']."'>".$val['name']."</a>"),
            array("string"=>$del,
              "attach"=>" style='border-right: 0px;width:24px; text-align:center;' ")
            ));
    }

    /* Assign all existing profiles to smarty */
    $smarty->assign("divlist",$divlist->DrawList());
    $smarty = get_smarty(); 
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    $smarty->assign("server_path",$this->server_path);
    return($smarty->fetch(get_template_path("goKioskService.tpl",TRUE,dirname(__FILE__))));
  }



  function getListEntry()
  {
    $fields = goService::getListEntry();
    $fields['Message']    = _("Kiosk profile service");
    #$fields['AllowEdit']  = TRUE;
    #$fields['AllowStart']  = false;
    #$fields['AllowStop']  = false;
    #$fields['AllowRestart']  = false;
    return($fields);
  }

  function check()
  { 
    $message = plugin::check();
    return($message);
  }

  function save()
  {
    goService::save();

    $method = $this->server_path."/";
    $method = preg_replace("/\/\/*$/","/",$method);
    $this->attrs['gotoKioskProfile'] = array();
    foreach($this->gotoKioskProfiles as $profile){
      if(!$profile['initial']){

        $contents = $profile['content'];
        $path = $this->baseDir."/".$profile['name'];
        $fp = @fopen($path,"w");
        if(!$fp){
          msg_dialog::display(_("Error"), msgPool::cannotWriteFile($path), ERROR_DIALOG);
        }else{
          fwrite($fp,$contents,strlen($contents));
          $this->attrs['gotoKioskProfile'][] = $method.$profile['name'];
        }
        @unlink($profile['tmp_name']);
      }else{
        $this->attrs['gotoKioskProfile'][] = $method.$profile['name'];
      }
    }
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->cd($this->dn);
    $ldap->modify($this->attrs);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
    }
  }

  
  function save_object()
  {
    if(isset($_POST['goKioskPosted'])){
      goService::save_object();
      if(isset($_POST['server_path'])){
        $this->server_path = get_post("server_path");
      }
    }
  }  


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Kiosk"),
          "plDescription" => _("Kiosk profile management")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 100,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array()
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
