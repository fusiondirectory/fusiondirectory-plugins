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


  /* This class represents a set of dhcpOptions, which are used over several dhcp plugins.
   * E.g. 'dhcpHost' includes 'dhcpAdvanced', both use the same options.
   * In other words, they use a references to an instance of this class.
   */
class dhcpOption{

  var $options = array();

  function getAll(){
    return($this->options);
  }

  function get($name){
    if(isset($this->options[$name])){
      if(count($this->options[$name]) == 0){
        return("");
      }elseif(count($this->options[$name]) == 1){
        return($this->options[$name][0]);
      }else{
        return($this->options[$name]);
      }
    }
  }

  function removeAll($name){
    if(isset($this->options[$name])){
      unset($this->options[$name]);
    }
  }

  function remove($name,$entry = 0){
    if(!isset($this->options[$name])){
      return;
    }
    if(isset($this->options[$name][$entry])){
      unset($this->options[$name][$entry]);
    }
    if(count($this->options[$name]) == 0){
      unset($this->options[$name]);
    }else{
      $this->options[$name] = array_unique($this->options[$name]);
      $this->options[$name] = array_values($this->options[$name]);
    }
  }

  function set($name,$value = null){
    if($value == null){
      $value = trim(preg_replace("/^[^ ]*/",'',$name));
      $name  = trim(preg_replace("/ .*$/",'',$name));
    }

    if(!isset($this->options[$name])){
      $this->options[$name] = array();
    }

    $this->options[$name] = array($value);
  }

  function add($name,$value = null){
    if($value == null){
      $value = trim(preg_replace("/^[^ ]*/",'',$name));
      $name  = trim(preg_replace("/ .*$/",'',$name));
    }

    if(!isset($this->options[$name])){
      $this->options[$name] = array();
    }

    $this->options[$name][] = $value;
    $this->options[$name] = array_unique($this->options[$name]);
    $this->options[$name] = array_values($this->options[$name]);
  }

  function save(){
    $res = array();
    foreach($this->options as $key => $values){
      foreach($values as $value){
        $res[] = $key." ".$value;
      }
    }
    $res=array_unique($res);
    return($res);
  }

  function exists($name){
    if(isset($this->options[$name]) && count($this->options[$name]) == 0){
      return(false);
    }
    return(isset($this->options[$name]));
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
