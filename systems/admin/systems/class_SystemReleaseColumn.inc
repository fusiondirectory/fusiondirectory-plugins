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
 * \brief Column showing the FAI release of a computer
 */
class SystemReleaseColumn extends Column
{
  function fillNeededAttributes (array &$attrs)
  {
    $attrs['objectClass'] = '*';
    $attrs['FAIclass']    = 1;
  }

  function renderCell (ListingEntry $entry): string
  {
    return $this->computeRelease($entry);
  }

  function compare (ListingEntry $ao, ListingEntry $bo): int
  {
    // Extract values from ao and bo
    $a = $this->computeRelease($ao);
    $b = $this->computeRelease($bo);

    return strcmp($a, $b);
  }

  protected function computeRelease (ListingEntry $entry): string
  {
    if (isset($entry->cache[__CLASS__]['release'])) {
      return $entry->cache[__CLASS__]['release'];
    }

    $entry->cache[__CLASS__]['release'] = $this->searchRelease($entry);

    return $entry->cache[__CLASS__]['release'];
  }

  protected function searchRelease (ListingEntry $entry): string
  {
    global $config;

    // No objectclasses set - go ahead
    if (empty($entry['objectClass'])) {
      return '&nbsp;';
    }

    // Skip non fai objects
    if (!in_array_ics('FAIobject', $entry['objectClass'])) {
      return '&nbsp;';
    }

    // If we've an own fai class, just use this
    if (isset($entry['FAIclass'])) {
      foreach (explode(' ', $entry['FAIclass']) as $element) {
        if ($element[0] == ':') {
          return '&nbsp;'.htmlentities(mb_substr($element, 1), ENT_COMPAT, 'UTF-8');
        }
      }
    }

    // Load information if needed
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=gosaGroupOfNames)(FAIclass=*)(member='.$entry->dn.'))', ['FAIclass','cn']);
    while ($attrs = $ldap->fetch()) {
      $rel = htmlentities(preg_replace('/^.*:/', '', $attrs['FAIclass'][0]), ENT_COMPAT, 'UTF-8');
      $sys = htmlentities(sprintf(_('Inherited from %s'), $attrs['cn'][0]), ENT_COMPAT, 'UTF-8');
      $str = '&nbsp;<img class="center" src="plugins/ogroups/images/ogroup.png"
          title="'.$sys.'"
          alt="'.$sys.'"/>&nbsp;'.$rel;
      return $str;
    }

    return '&nbsp;';
  }
}
