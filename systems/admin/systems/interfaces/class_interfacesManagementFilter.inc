<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2020  FusionDirectory

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
 * \brief This class handles the interfaces management filter box
 */
class interfacesManagementFilter extends managementFilter
{
  protected $entriesCache;

  /*!
   * \brief Create a management filter
   *
   * \param string $parent management instance
   */
  function __construct (management $parent, bool $fixedScope = NULL, array $filterElementDefinitions = [])
  {
    global $config;

    parent::__construct($parent, $fixedScope, $filterElementDefinitions);
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
    /* Only read data from LDAP once */
    if (!isset($this->entriesCache)) {
      $this->entriesCache = parent::query($attrs, $base);
    }
    return $this->entriesCache;
  }

  function updateCache (string $dn, simpleTabs $object = NULL)
  {
    if ($object === NULL) {
      if (isset($this->entriesCache[0][$dn])) {
        unset($this->entriesCache[0][$dn]);
        $this->entriesCache[1][key($this->entriesCache[1])]--;
      }
      return;
    }
    if (!isset($this->entriesCache[0][$dn])) {
      $this->entriesCache[0][$dn] = new ListingEntry($this->parent->listing, $this->parent->objectTypes[0], $dn, [], $this->entriesCache[1][key($this->entriesCache[1])]);
      $this->entriesCache[1][key($this->entriesCache[1])]++;
    }
    foreach ($object->getBaseObject()->attributes as $attr) {
      $value = $object->getBaseObject()->$attr;
      if (empty($value)) {
        $value = NULL;
      } elseif (!is_array($value)) {
        $value = [$value];
      }
      $this->entriesCache[0][$dn][$attr] = $value;
    }
  }

  function resetFromCache (string $dn, simpleTabs $object)
  {
    if (!isset($this->entriesCache[0][$dn])) {
      return;
    }
    foreach ($object->getBaseObject()->attributes as $attr) {
      $value = $object->getBaseObject()->$attr;
      if (is_array($value)) {
        $object->getBaseObject()->$attr = ($this->entriesCache[0][$dn][$attr] ?? []);
      } else {
        $object->getBaseObject()->$attr = ($this->entriesCache[0][$dn][$attr][0] ?? '');
      }
    }
  }
}
