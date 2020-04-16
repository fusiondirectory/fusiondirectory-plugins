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

class SudoUsersGroupsAttribute extends ObjectsAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $required, array $defaultValue = [], string $store_attr = 'uid', string $display_attr = 'nameAttr', array $filterElementDefinitions = NULL, string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, ['user', 'group', 'ogroup'], $defaultValue, $store_attr, $display_attr, $filterElementDefinitions, $acl);
  }

  function addValue (string $dn, $attrs = NULL)
  {
    if (
        ($this->isTemplate() && ($attrs === NULL)) ||
        ($this->store_attr == 'dn')
      ) {
      $value = $dn;
    } elseif (in_array('posixGroup', $attrs['objectClass']) || in_array('groupOfNames', $attrs['objectClass'])) {
      $value = trim('%'.$attrs['cn'][0]);
    } elseif (isset($attrs['uid'][0])) {
      $value = trim($attrs['uid'][0]);
    }
    if (!in_array($value, $this->value) && !in_array('!'.$value, $this->value)) {
      end($this->value);
      $new_key = key($this->value) + 1;
      $this->value[$new_key] = $value;
      $this->fillDisplayValueFrom($new_key, $attrs);
    }
  }

  function getFilterBlackList ()
  {
    $used = [];
    foreach ($this->value as $name) {
      $str = preg_replace('/^!/', '', $name);
      if (preg_match('/^%/', $str)) {
        $used['cn'][] = preg_replace('/^%/', '', $str);
      } else {
        $used['uid'][] = $str;
      }
    }
    return $used;
  }

  protected function fillDisplayValue ($i)
  {
    $value = $this->value[$i];
    if (preg_match('/^%/', $value)) {
      $objects = objects::ls(['group','ogroup'], $this->selectManagementParameters[2], NULL, '(cn='.ldap_escape_f(preg_replace('/^%/', '', $value)).')');
    } else {
      $objects = objects::ls(['user'], $this->selectManagementParameters[2], NULL, '(uid='.ldap_escape_f($value).')');
    }
    if (empty($objects) && $this->isTemplate()) {
      $this->fillDisplayValueFrom($i, NULL);
    } else {
      $this->fillDisplayValueFrom($i, reset($objects));
    }
  }
}
