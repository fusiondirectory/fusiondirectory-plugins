<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
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

/*!
 * \brief This class handles the filter for servicesManagement
 */
class servicesManagementFilter extends managementFilter
{
  function __construct (servicesManagement $parent)
  {
    $this->parent = $parent;

    $this->pid = preg_replace('/[^0-9]/', '', microtime(TRUE));

    $this->types = [];

    $this->filterElements = [];

    $this->fixedScope = TRUE;
  }

  function update (string $base)
  {
  }

  function render (): string
  {
    return '';
  }

  function query (array $attrs, string $base): array
  {
    $plugins  = $this->parent->getServiceList();
    $entries  = [];
    $row      = 0;
    foreach ($plugins as $name => $obj) {
      if ($obj->is_account) {
        $data = $obj->getListEntry();
        $data['name'] = $name;
        foreach ($data as $key => $value) {
          if (!is_array($value)) {
            $data[$key] = [$value];
          }
        }
        if (!isset($this->parent->dn)) {
          echo "<h1>".get_class($this->parent)."</h1>\n";
        }
        $entries[$name] = new ListingEntry($this->parent->listing, 'service', $this->parent->dn, $data, $row++);
      }
    }
    return [$entries, ['service' => count($entries)]];
  }
}
