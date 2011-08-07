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

class gotoLogView extends plugin
{
  
  var $mac;
  var $logs;
  var $event;
  var $parent;
  var $config;
  
  var $o_queue;  

  var $selected_date;
  var $selected_file = 0;

  var $attributes = array("macAddress");
  var $macAddress = "";

  var $sort_by  = "time";
  var $sort_dir = 1; // 1 => up, 0 => down
  
  var $ignore_account = TRUE;
  var $standalone = FALSE;
 
  function __construct(&$config,$dn,$parent)
  {
    $this->config = $config;
    $this->parent = $parent;

    /* Try to fetch logs for the given event (mac)
     */
    $this->o_queue = new gosaSupportDaemon();

    /* Load ldap object if given 
        and use this macAddress.
     */
    if(is_object($parent) && $dn != "" && $dn != "new"){
      plugin::plugin($config,$dn,$parent);
    } 

    /* Get correct macAddress.
        Check if an event is given or a ldap object.
     */      
    if(is_array($this->parent) && isset($this->parent['MACADDRESS'])){
      $this->mac = $this->parent['MACADDRESS'];
      $this->standalone = TRUE;
    }elseif(isset($parent->attrs['macAddress'][0])){
      $this->mac = $parent->attrs['macAddress'][0];
      $this->standalone = FALSE;
    }

    /* Query for log files
     */
    $res = $this->o_queue->get_log_info_for_mac($this->mac);
    if($this->o_queue->is_configured() && $this->o_queue->is_error()){
      msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
    }

    /* Check if there is at least one log file 
     */
    if(!isset($res[$this->mac]) || !is_array($res[$this->mac])){
      $this->logs = array();
    }else{
      $this->selected_date = key($res[$this->mac]);
      $this->logs = $res;
    }

  }


  function execute()
  {
    $smarty = get_smarty();
    $smarty->assign("logs",$this->logs);   
    $smarty->assign("logs_available", isset($this->logs[$this->mac]));
    $smarty->assign("mac",$this->mac);
    $smarty->assign("selected_file",$this->selected_file);
    $smarty->assign("selected_date",$this->selected_date);
    $smarty->assign("log_file", $this->get_log($this->mac,$this->selected_date,$this->selected_file));        
    $smarty->assign("standalone",$this->standalone);

    if (isset($this->logs[$this->mac])){
	    $date = date("d.m.Y H:i:s",$this->logs[$this->mac][$this->selected_date]['REAL_DATE']);
    }
    $file = $this->selected_file;
    $smarty->assign("selected_log",_("none"));
    if(!empty($file)){
      $smarty->assign("selected_log",$file.", ".$date);
    }

    $divlist = new divlist("log_view");

    /* Create sort direction images 
     */
    if($this->sort_dir){
      $img = "<img src='images/lists/sort-down.png' border='0' alt='\\/'>";
    }else{
      $img = "<img src='images/lists/sort-up.png' border='0' alt='/\\'";
    }
    if($this->sort_by == "file"){
      $img1 = $img;
      $img2 = "";
    }else{
      $img1 = "";
      $img2 = $img;
    }


    /* Create list header 
     */
    $divlist->SetHeader(array(
          array("string"=>"<a href='?plug=".$_GET['plug']."&sort_by=file'>"._("File")." ".$img1."</a>",
                "attach"=>"width='200px;'"),
          array("string"=>"<a href='?plug=".$_GET['plug']."&sort_by=time'>"._("Date")." ".$img2."</a>",
                "attach" => "style='border-right:none;'"),
          ));

    /* Create divlist list 
     */
    $divlist->SetEntriesPerPage(0);
    $divlist->SetHeight(150); 

    /* Create sortable array
     */ 
    $link = "<a href='?plug=".$_GET['plug']."&time=%time%&file=%file%&mac=%mac%'>%str%</a>";
    $to_add = array();
    $sort_by = $this->sort_by;
    foreach($this->logs as $mac => $times){
      foreach($times as $time => $data){
        $rtime = $data['REAL_DATE'];
        foreach($data['FILES'] as $file){

          $highlight = "";
          if($file == $this->selected_file && $time == $this->selected_date && $mac == $this->mac){
            $highlight = "background-color:#CCCCCC";
          }

          $use_link = preg_replace(array("/%mac%/","/%time%/","/%file%/"),array($mac,$time,$file),$link);
          $to_add[$$sort_by.$file.$time] = array(
            array("string" => preg_replace("/%str%/",$file,$use_link),
                  "attach" => "style='width:200px; $highlight'"),
            array("string" => preg_replace("/%str%/",date("d.m.Y H:i:s",$rtime),$use_link),
                  "attach" => "style='border-right:none; $highlight'"),
            );
        }
      }
    }

    /* Sort entries 
     */
    if(!$this->sort_dir){
      uksort($to_add, "strnatcasecmp");
    }else{
      uksort($to_add, "strnatcasecmp");
      $to_add = array_reverse($to_add);
    }

    /* Append entries to list 
     */
    foreach($to_add as $entry){
      $divlist->AddEntry($entry);
    }

    $smarty->assign("ACL",preg_match("/r/",$this->getacl("")));
    $smarty->assign("divlist",$divlist->DrawList());
    return($smarty->fetch(get_template_path('log_view.tpl', TRUE,dirname(__FILE__))));
  }


  function get_log($mac,$date,$file)
  {
    $res = $this->o_queue->get_log_file($mac,$date,$file);
    if($this->o_queue->is_configured() && $this->o_queue->is_error()){
      msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
    }
    $res = nl2br(htmlentities($res));
    return($res);
  }

  
  function save_object()
  {
    foreach(array("time"=>"selected_date","file"=>"selected_file") as $attr => $dest){
      if(isset($_GET[$attr])){
        $this->$dest = $_GET[$attr];
      }
    }
    if(isset($_GET['sort_by']) && in_array($_GET['sort_by'],array("file","time"))){
      if($_GET['sort_by'] == $this->sort_by){
        $this->sort_dir = !$this->sort_dir;
      }
      $this->sort_by = $_GET['sort_by'];
    }
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Log view"),
          "plDescription" => _("GOto log view"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 30,
          "plSection"     => array("administration"),
          "plCategory"    => array("workstation","server"),

          "plProvidedAcls"=> array()
            ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
