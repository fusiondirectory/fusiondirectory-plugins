<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class filterServerService
{
  static function query($parent, $base, $scope, $filter, $attributes, $category, $objectStorage = "")
  {
    $plugins  = ServerService::getServiceList();
    $list     = array();
    foreach ($plugins as $name => $obj) {
      if ($obj->is_account) {
        $data = $plugins[$name]->getListEntry();
        $data['objectClass']  = array('FAKE_OC_ServerService');
        $data['name']         = $name;
        foreach (array('AllowStatus','AllowStart','AllowStop','AllowRestart','AllowRemove','AllowEdit') as $key) {
          if ($data[$key]) {
            $data['objectClass'][] = $key;
          }
        }
        foreach ($data as $key => $value) {
          if (!is_array($value)) {
            $value = array($value);
          }
          $list[$name][]              = $key;
          $list[$name][$key]          = $value;
          $list[$name][$key]['count'] = count($value);
          $list[$name]['count']       = count($data);
          $list[$name]['dn']          = $name;
        }
      }
    }
    return array_values($list);
  }
}
?>
