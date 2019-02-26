<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019  FusionDirectory

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
 * \brief Column showing the services active on a server
 */
class SystemServicesColumn extends Column
{
  protected $services;

  function __construct (managementListing $parent, array $attributes = NULL, string $label = NULL)
  {
    global $config;

    parent::__construct($parent, NULL, $label);

    $this->services = [];

    foreach ($config->data['TABS']['SERVERSERVICE'] as $plug) {
      if (class_available($plug['CLASS'])) {
        $pInfos = pluglist::pluginInfos($plug['CLASS']);
        if (isset($pInfos['plIcon'])) {
          $this->services[$plug['CLASS']] = [
            'icon'  => $pInfos['plIcon'],
            'title' => $pInfos['plShortName'],
          ];
        } else {
          @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $pInfos['plShortName'].' ('.$plug['CLASS'].')', 'No icon for');
        }
      }
    }
  }

  function fillNeededAttributes (array &$attrs)
  {
    $attrs['objectClass'] = '*';
  }

  function renderCell (ListingEntry $entry): string
  {
    $icons  = $this->computeIcons($entry);
    $result = '';
    foreach ($icons as $icon) {
      if (!empty($icon['icon'])) {
        $result .= '<input type="image" src="'.htmlentities($icon['icon'], ENT_COMPAT, 'UTF-8').'" '.
                    'alt="'.htmlentities($icon['title'], ENT_COMPAT, 'UTF-8').'" title="'.htmlentities($icon['title'], ENT_COMPAT, 'UTF-8').'" '.
                    'name="listing_edit_service_'.$icon['class'].'_'.$entry->row.'"/>';
      }
    }

    return $result;
  }

  function compare (ListingEntry $ao, ListingEntry $bo): int
  {
    if ($ao->getTemplatedType() != $bo->getTemplatedType()) {
      return strcmp($ao->getTemplatedType(), $bo->getTemplatedType());
    }

    // Extract values from ao and bo
    $a = $this->computeSortString($ao);
    $b = $this->computeSortString($bo);

    return strcmp($a, $b);
  }

  protected function computeSortString (ListingEntry $entry): string
  {
    if (isset($entry->cache[__CLASS__]['sort'])) {
      return $entry->cache[__CLASS__]['sort'];
    }

    $icons = $this->computeIcons($entry);
    $entry->cache[__CLASS__]['sort'] = implode('', array_map(
      function($icon)
      {
        return (empty($icon['icon']) ? 0 : 1);
      },
      $icons
    ));

    return $entry->cache[__CLASS__]['sort'];
  }

  protected function computeIcons (ListingEntry $entry): array
  {
    if (strtoupper($entry->getTemplatedType()) != 'SERVER') {
      return [];
    }
    if (isset($entry->cache[__CLASS__]['icons'])) {
      return $entry->cache[__CLASS__]['icons'];
    }

    $icons = [];

    if (!empty($entry)) {
      if ($entry->isTemplate()) {
        $attrs = $entry->getTemplatedFields();
      } else {
        $attrs = $entry;
      }
      foreach ($this->services as $class => $infos) {
        $status = $class::isAccount($attrs);
        if ($status !== FALSE) {
          $icons[] = [
            'icon'  => $infos['icon'],
            'title' => $infos['title'],
            'class' => $class,
          ];
        } else {
          $icons[] = [
            'class' => $class,
          ];
        }
      }
    }

    $entry->cache[__CLASS__]['icons'] = $icons;
    return $entry->cache[__CLASS__]['icons'];
  }
}
