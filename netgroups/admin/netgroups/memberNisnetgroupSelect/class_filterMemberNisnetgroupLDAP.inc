<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * and  compatible with GOsa 2.6 (http://www.gosa-project.org)
 * Copyright (C) 2011 Alejandro Escanero Blanco (aescanero@gmail.com)
 * Copyright (C) 2011-2013  FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

class filterMemberNisnetgroupLDAP {
  static function query($parent, $base, $scope, $filter, $attributes, $category, $objectStorage= "") {
    $result = array();
    $menu = array();
    $dn2index = array();
    $config = session::global_get('config');
    $ldap = $config->get_ldap_link(TRUE);
    $flag = ($scope == "sub") ? GL_SUBSEARCH : 0;
    $tmp = self::get_list($parent, $base, $filter, $attributes, $category, $objectStorage, $flag | GL_SIZELIMIT);

    // Sort out menu entries, but save info
    $index = 0;
    foreach ($tmp as $entry) {
      if (in_array_ics("nisNetgroup", $entry['objectClass'])) {
        $result[$index] = $entry;
        $dn2index[$entry['dn']] = $index;
        $index++;
      } else {
        foreach ($objectStorage as $storage) {
          $group = preg_replace("/^.*,([^,]+)," . preg_quote("$storage$base") . "$/", '$1', $entry['dn']);
          $group = "$group,$storage$base";

          // The current group implementation has no multiple storage settings - so break here
          break;
        }
        $menu[$group] = true;
      }
    }

    return filterLDAPBlacklist::filterByBlacklist($result);
  }

  static function get_list($parent, $base, $filter, $attributes, $category, $objectStorage, $flags= GL_SUBSEARCH) {
    $tmp = array();
    foreach ($objectStorage as $storage) {
      $list = filterLDAP::get_list($parent, $base, $filter, $attributes, $category, array($storage), $flags);
      foreach ($list as $entry) {
        $tmp[] = $entry;
      }
    }
    return $tmp;
  }
}

?>
