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

class SupannOrderedArrayAttribute extends OrderedArrayAttribute
{
  function supannPrepareSave ()
  {
    $values = [];
    foreach ($this->value as $value) {
      $this->attribute->setValue($value);
      $this->attribute->supannGetValues($values);
    }
    foreach ($values as $ldapName => $array) {
      if ($this->plugin->attributesAccess[$ldapName]->isVisible()) {
        continue;
      }
      $this->plugin->attributesAccess[$ldapName]->setValue(array_values($array));
    }
  }
}
