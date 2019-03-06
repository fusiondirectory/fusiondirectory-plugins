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

/*!
 * \brief Column showing the usage of an OPSI profile
 */
class OpsiProfileUsageColumn extends Column
{
  function renderCell (ListingEntry $entry): string
  {
    if (strcasecmp($entry->type, 'opsiProfile') != 0) {
      return '&nbsp;';
    }

    list($systems, $groups) = $this->computeStats($entry);

    if (($systems == 0) && ($groups == 0)) {
      return '&nbsp;';
    } else {
      $result = '';
      if ($systems > 0) {
        $typeInfos = objects::infos('workstation');
        $result .= '<img class="center" src="'.htmlentities($typeInfos['icon'], ENT_COMPAT, 'UTF-8').'" title="'.$typeInfos['name'].'" alt="'.$typeInfos['name'].'"/>&nbsp;'.$systems;
      }
      if ($groups > 0) {
        $typeInfos = objects::infos('ogroup');
        $result .= '<img class="center" src="'.htmlentities($typeInfos['icon'], ENT_COMPAT, 'UTF-8').'" title="'.$typeInfos['name'].'" alt="'.$typeInfos['name'].'"/>&nbsp;'.$groups;
      }
      return $result;
    }
  }

  function compare (ListingEntry $ao, ListingEntry $bo): int
  {
    if ($ao->getTemplatedType() != $bo->getTemplatedType()) {
      return strcmp($ao->getTemplatedType(), $bo->getTemplatedType());
    }

    if (strcasecmp($ao->type, 'opsiProfile') != 0) {
      return parent::compare($ao, $bo);
    }

    // Extract values from ao and bo
    list($a_systems, $a_groups) = $this->computeStats($ao);
    list($b_systems, $b_groups) = $this->computeStats($bo);

    if (($a_systems + $a_groups) < ($b_systems + $b_groups)) {
      return -1;
    } elseif (($a_systems + $a_groups) > ($b_systems + $b_groups)) {
      return 1;
    } elseif ($a_systems < $b_systems) {
      return -1;
    } elseif ($a_systems > $b_systems) {
      return 1;
    } else {
      return 0;
    }
  }

  protected function computeStats (ListingEntry $entry): array
  {
    if (isset($entry->cache[__CLASS__]['stats'])) {
      return $entry->cache[__CLASS__]['stats'];
    }

    $systems  = objects::count('workstation', NULL, '(&(objectClass=opsiClient)(fdOpsiProfileDn='.$entry->dn.'))', FALSE);
    $groups   = objects::count('ogroup',      NULL, '(&(objectClass=opsiClient)(fdOpsiProfileDn='.$entry->dn.'))', FALSE);

    $entry->cache[__CLASS__]['stats'] = [$systems, $groups];

    return $entry->cache[__CLASS__]['stats'];
  }
}
