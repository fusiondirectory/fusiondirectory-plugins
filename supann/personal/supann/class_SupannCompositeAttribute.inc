<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2010-2012 Antoine Gallavardin
  Copyright (C) 2013-2020 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class SupannCompositeAttribute extends CompositeAttribute
{
  function __construct ($description, $ldapName, array $attributes, $acl = "", $label = "Composite attribute")
  {
    parent::__construct($description, $ldapName, $attributes, '', '', $acl, $label);
  }

  function readValues (string $value): array
  {
    $values = [];
    $m = [];
    foreach ($this->attributes as &$attribute) {
      $shortname = preg_replace('/^[^_]+_/', '', $attribute->getLdapName());
      if (preg_match("/\\[$shortname=([^\\]]+)\\]/", $value, $m)) {
        $values[] = $m[1];
      } else {
        $values[] = "";
      }
    }
    unset($attribute);

    return $values;
  }

  function writeValues (array $values)
  {
    $value  = '';
    $i      = 0;
    foreach ($this->attributes as &$attribute) {
      if ($values[$i] != '') {
        $shortname = preg_replace('/^[^_]+_/', '', $attribute->getLdapName());
        $value .= "[$shortname=".$values[$i]."]";
      }
      $i++;
    }
    unset($attribute);
    return $value;
  }

  function supannGetValues (&$values)
  {
    foreach ($this->attributes as &$attribute) {
      $shortname = preg_replace('/^([^_]+)_.*$/', '\\1', $attribute->getLdapName());
      $value = $attribute->getValue();
      if (!isset($values[$shortname])) {
        $values[$shortname] = [];
      }
      if ($value == "") {
        continue;
      }
      $values[$shortname][$value] = $value;
    }
    unset($attribute);
  }
}
