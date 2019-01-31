<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class SudoUsersGroupsAttribute extends DialogAttribute
{
  protected $dialogClass = 'UserGroupSelectDialog';

  function addValue ($dn, $entry)
  {
    if (in_array('posixGroup', $entry['objectClass']) || in_array('groupOfNames', $entry['objectClass'])) {
      $name = trim("%".$entry['cn'][0]);
    } elseif (isset($entry['uid'][0])) {
      $name = trim($entry['uid'][0]);
    }
    if (!in_array($name, $this->value) && !in_array("!".$name, $this->value)) {
      $this->value[] = $name;
    }
  }

  function getFilterBlackList ()
  {
    $used = array();
    foreach ($this->value as $name) {
      $str = preg_replace("/^!/", "", $name);
      if (preg_match("/^%/", $str)) {
        $used['cn'][] = preg_replace("/^%/", "", $str);
      } else {
        $used['uid'][] = $str;
      }
    }
    return $used;
  }
}
