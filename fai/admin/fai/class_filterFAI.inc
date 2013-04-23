<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class filterFAI extends filterLDAP{

  static function query($parent,$base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {
  /* Create a new list of FAI object
     * Generate List of Partitions,Hooks,Scripts,Templates,Profiles ...
     */
    $ObjectTypes = array(
        "FAIpartitionTable"  => array("OU"=> get_ou('faiPartitionRDN') , "CHKBOX"=>"ShowPartitions"  ,"ACL" => "faiPartitionTable"),
        "FAIpackageList"     => array("OU"=> get_ou('faiPackageRDN')   , "CHKBOX"=>"ShowPackages"    ,"ACL" => "faiPackage"),
        "FAIscript"          => array("OU"=> get_ou('faiScriptRDN')    , "CHKBOX"=>"ShowScripts"     ,"ACL" => "faiScript"),
        "FAIvariable"        => array("OU"=> get_ou('faiVariableRDN')  , "CHKBOX"=>"ShowVariables"   ,"ACL" => "faiVariable"),
        "FAIhook"            => array("OU"=> get_ou('faiHookRDN')      , "CHKBOX"=>"ShowHooks"       ,"ACL" => "faiHook"),
        "FAIprofile"         => array("OU"=> get_ou('faiProfileRDN')   , "CHKBOX"=>"ShowProfiles"    ,"ACL" => "faiProfile"),
        "FAItemplate"        => array("OU"=> get_ou('faiTemplateRDN')  , "CHKBOX"=>"ShowTemplates"   ,"ACL" => "faiTemplate"));

    global $config;
    $tmp = FAI::get_all_objects_for_given_base($base,$filter);
    $ui = get_userinfo();
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    $result = array();
    foreach($tmp as $entry) {

      /* Get some more informations about the object */
      $ldap->cat($entry['dn'], array("cn","description","objectClass","FAIclass","FAIstate","objectClass"));
      $object  = $ldap->fetch();

      /* Walk through possible types */
      foreach($ObjectTypes as $type => $rest){

        $acl = $ui->get_permissions($object['dn'],"fai/".$rest ['ACL']);

        if(in_array($type,$object['objectClass']) && preg_match("/r/",$acl)){

          /* Prepare object */
          unset($object['objectClass']['count']);
          if(!isset($object['description'][0])){
            $object['description'][0]="";
          }

          $dn = preg_replace("/^[^,]*+,/","",$object['dn']);

          /* Clean up object informations */
          $obj                  = array();
          $obj['cn']            = $object['cn'][0];
          $obj['dn']            = $object['cn'][0].",".get_ou("faiBaseRDN").$config->current['BASE'];
          $obj['acl']           = $acl;
          $obj['class']         = $rest ['ACL'];
          $obj['FAIstate']      = $entry['FAIstate'];
          $obj['description']   = $object['description'][0];
          $obj['objectClass']   = $object['objectClass'];
          $obj['objectClass'][] = 'FAKE_OC_FAI';

          if(!preg_match("/freeze/i", $entry['FAIstate'])){
            $obj['objectClass'][] = "FAKE_OC_IsBranch";
          }
          $object['FAIstate'] = $entry['FAIstate'];

          $obj['TYPES'][]       = $type;
          $obj['GROUPS'][$type]      = $object;

          if(!isset($result[$obj['cn']])){
            $result[$obj['cn']] = $obj;
          }else{
            $result[$obj['cn']]['TYPES'][]=$type;
            $result[$obj['cn']]['GROUPS'][$type]=$object;
          }
        }
      }
    }

    // Make results useable for the listing class.
    $entries = array();
    foreach($result as $entry) {
      $item = array('count' => count($entry)-1);
      foreach($entry as $name => $value) {
        if ($name != "GROUPS"){
          $item[] = $name;
          $item[$name] = $value ;
        }
      }
      $item['GROUPS'] = $entry['GROUPS'];
      $entries[] = $item;
    }

    return $entries;
  }
}
?>
