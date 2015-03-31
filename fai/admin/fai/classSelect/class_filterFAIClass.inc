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

class filterFAIClass {

  static function query($parent,$base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {
    global $config;

    $base = session::get('filterFAIClass_base');
    $objectFilter = "(|(objectClass=FAIscript)(objectClass=FAItemplate)(objectClass=FAIhook)".
      "(objectClass=FAIvariable)(objectClass=FAIpartitionTable)(objectClass=FAIpackageList))";

    $FAI_objects = FAI::get_all_objects_for_given_base($base,$objectFilter,true);

    /* Create array with categories to be able to sort different types of objects */
    $categories = array("FAIscript"         => "faiScript",
        "FAItemplate"       => "faiTemplate",
        "FAIhook"           => "faiHook",
        "FAIvariable"       => "faiVariable",
        "FAIpartitionTable" => "faiPartitionTable",
        "FAIpackageList"    => "faiPackage");

    /* Read out all objects from fai tree */
    $ldap= $config->get_ldap_link();
    $ldap->cd($base);
    $ui = get_userinfo();
    $sort=array();
    $FAIAllclasses = array();

    foreach($FAI_objects as $obj){

      $ldap->cat($obj['dn']);
      $attrs = $ldap->fetch();

      /* Only use objects which have cn set */
      if(isset($attrs['cn'][0]) && preg_match("/".preg_quote($filter,'/')."/i", $attrs['cn'][0])){
        foreach($categories as $cat => $acl){
          if(in_array($cat,$attrs['objectClass'])){
            $acl =  $ui->get_permissions($attrs['dn'],"fai/".$acl);
            if(!isset($FAIAllclasses[$attrs['cn'][0]])){
              $FAIAllclasses[$attrs['cn'][0]] = $attrs ;
            }else{
              $FAIAllclasses[$attrs['cn'][0]]['objectClass'] =
                array_merge($attrs['objectClass'],$FAIAllclasses[$attrs['cn'][0]]['objectClass']);
            }
          }
        }
      }
    }
    return(filterFAIClass::filterByBlacklist(array_values($FAIAllclasses)));
  }

  static function filterByBlacklist($entries)
  {
    if(session::is_set('filterBlacklist')){
      $blist = session::get('filterBlacklist');
      foreach($blist as $attr_name => $attr_values){
        foreach($attr_values as $match){
          foreach($entries as $id => $entry){
            if(isset($entry[$attr_name])){
              $test = $entry[$attr_name];
              if(!is_array($test)) $test = array($test);
              if(in_array($match, $test)) unset($entries[$id]);
            }
          }
        }
      }
    }
    return(array_values($entries));
  }
}
?>
