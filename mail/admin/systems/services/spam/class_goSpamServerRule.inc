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

class goSpamServerRule extends plugin {

  var $attributes = array("name","rule");
  var $name = "";
  var $rule = "";
  
  var $orig_name = "";
  var $view_logged  =FALSE;
  
  function goSpamServerRule(&$config,$dn,$name = "",$rule ="")
  {
    plugin::plugin($config,$dn);
    $this->name = $this->orig_name= $name;
    $this->rule = $rule;
  }


  function execute()
  {
    $smarty = get_smarty();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    return($smarty->fetch(get_template_path("goSpamServerRule.tpl",TRUE,dirname(__FILE__))));     
  }

  function save_object()
  {
    plugin::save_object();
    foreach($this->attributes as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = $_POST[$attr];
      }
    }
  }


  function save()
  {
    $ret =array();
    $ret['orig_name'] = $this->orig_name;
    $ret['name'] = $this->name;
    $ret['rule'] = $this->rule;
    return($ret);
  }
  
  function check()
  {
    $messages = plugin::check();
    return($messages);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
