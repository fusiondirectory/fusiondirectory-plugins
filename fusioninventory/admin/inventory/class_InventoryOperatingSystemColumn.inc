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
 * \brief Column showing the operating systems in an inventory
 */
class InventoryOperatingSystemColumn extends LinkColumn
{
  function renderCell(ListingEntry $entry)
  {
    $trans = $this->computeValue($entry);

    if ($trans != '') {
      return $this->renderLink($entry, $trans);
    }

    return '&nbsp;';
  }

  protected function computeValue(ListingEntry $entry)
  {
    global $config;

    if (isset($entry->cache[__CLASS__]['value'])) {
      return $entry->cache[__CLASS__]['value'];
    }

    $trans = array();

    $ldap = $config->get_ldap_link();
    $ldap->cd($entry->dn);
    $ldap->search('(objectClass=fdInventoryOPERATINGSYSTEM)', array('fdInventoryNAME','fdInventoryFQDN'), 'one');
    while ($attrs = $ldap->fetch()) {
      $link = $attrs['fdInventoryNAME'][0];
      if (isset($attrs['fdInventoryFQDN'][0])) {
        $link .= ' ('.$attrs['fdInventoryFQDN'][0].')';
      }
      $trans[] = $link;
    }

    $entry->cache[__CLASS__]['value'] = join("<br/>\n", $trans);

    return $entry->cache[__CLASS__]['value'];
  }

  function compare(ListingEntry $ao, ListingEntry $bo)
  {
    if ($ao->getTemplatedType() != $bo->getTemplatedType()) {
      return strcmp($ao->getTemplatedType(), $bo->getTemplatedType());
    }

    // Extract values from ao and bo
    $a = $this->computeValue($ao);
    $b = $this->computeValue($bo);

    return strcmp($a, $b);
  }
}
