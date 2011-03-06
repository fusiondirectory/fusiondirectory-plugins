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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class filterOpsiLicense {

  static function query($parent,$base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {

    if(!class_available('opsi')) return(array());

    $config= session::global_get('config');
    $ldap= $config->get_ldap_link(TRUE);
    $flag= ($scope == "sub")?GL_SUBSEARCH:0;
    $result= filterOpsiLicense::get_list($parent,$base, $filter, $attributes, $category, $objectStorage, $flag);

    // Simple filtering 
    if(!empty($filter)){
      foreach($result as $key => $entry){
        if(!preg_match($filter, $entry['cn'][0])){
          unset($result[$key]);
        }
      }
    }

    return(filterACL::unifyResult($result));
  }

  static function get_list($parent,$base, $filter, $attributes, $category, $objectStorage, $flags= GL_SUBSEARCH)
  {
    $config= session::global_get('config');
    $si = new opsiLicenceHandler($config);

    if(!$si->enabled()) return(array());

    $res = $si->listPools();
    $result = array();
    if($si->is_error() || !is_array($res)){
      $this->init_successfull = FALSE;
      msg_dialog::display(_("Error"),msgPool::siError($si->get_error()),ERROR_DIALOG);
      return;
    }else{

      // Reset the list of licenses
      foreach($res as $item){

        $item['objectClass'] = array('fake_opsiLicense');

        // Fake an ldap entry, this enables ACL checks.
        $entry = array();
        $entry['dn'] = "opsi:cn=".$item['cn'][0].",".$config->current['BASE'];
        foreach($item as $name => $value){
          $entry[] = $name;
          $entry[$name] = $value;
        }
        $entry['count'] = count($item);
        $result[] = $entry;
      }
    }
    return($result);
  }

  static function unifyResult($result)
  {
    $res=array();
    foreach($result as $entry){
      if(!isset($res[$entry['dn']])){
        $res[$entry['dn']]=$entry;
      }
    }
    return(array_values($res)); 
  }
}

?>
