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

class filterRepository {

  static function query($parent,$base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {
    $list = servrepository::getRepositoryList();

    // Convert plugin data into useable format for list.
    $data = array();
    foreach($list as $key => $entry){   
      $entry['objectClass'] = array('FAKE_OC_FaiRepository');
      $item = array('count' => count($entry));
      $item['dn'] = $key;

      if(!count($entry['Sections'])){
        $entry['Sections'][] = '';
      }
      foreach($entry as $name => $value){   
        $item[] = $name;
        if(!is_array($value)) $value=array($value);
        $value= array_values($value);
        $value['count'] = count($value);
        $item[$name] = $value;
      }
      $data[] = $item;
    }
    return($data);
  }
}

?>
