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

class filterSystemByIp  extends filterLDAP{

  static function query($parent,$base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {
    
    // IP Range selection 
    $filterByIP = preg_match("/__FilterByIP__/", $filter);
    $filter = preg_replace("/__FilterByIP__/","",$filter);
    $ipstart = "";
    $ipstop = "";
    if(preg_match("/__IPSTART__=/", $filter)){
      $ipstart = trim(preg_replace("/^.*__IPSTART__=([^_]*)__.*$/","\\1",$filter),"*");
    }
    if(preg_match("/__IPSTOP__=/", $filter)){
      $ipstop = trim(preg_replace("/^.*__IPSTOP__=([^_]*)__.*$/","\\1",$filter),"*");
    }
    

    // Get entries 
    $filter= preg_replace("/__IPSTART.*$/","",$filter);
    $entries = filterLDAP::query($parent,$base, $scope, $filter, $attributes, $category, $objectStorage);
    

    // Filter entries by IP-Address
    if($filterByIP){
      foreach($entries as $key => $entry){ 
        if(!isset($entry['ipHostNumber'])){
          unset($entries[$key]);
          continue;
        }
        if(!tests::is_in_ip_range($ipstart,$ipstop, $entry['ipHostNumber'][0])){
          unset($entries[$key]);
          continue;
        }
      }
    }
    return(array_values($entries));
  }

}
?>
