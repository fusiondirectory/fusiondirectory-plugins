<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019  FusionDirectory

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

class TOTPRegistrationsAttribute extends DialogOrderedArrayAttribute
{
  protected $order        = FALSE;
  protected $dialogClass  = 'TOTPAddDialog';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = FALSE;
  }

  /*! \brief Convert text value to displayable array value */
  protected function getAttributeArrayValue ($key, $value)
  {
    return $value;
  }

  function readValue ($value)
  {
    return explode('|', $value, 2);
  }

  function writeValue ($key, $value)
  {
    return implode('|', $value);
  }

  public function getUserUid (): string
  {
    return $this->plugin->parent->getBaseObject()->uid;
  }
}
