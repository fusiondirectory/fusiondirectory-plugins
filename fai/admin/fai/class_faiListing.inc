<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2014-2019  FusionDirectory

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

class faiListing extends managementListing
{
  function setUpBaseSelector ()
  {
    global $config;

    if ($this->baseMode) {
      // Instanciate base selector
      $this->baseSelector = new faiBaseSelector($this->bases, $this->base);
      $this->refreshBasesList();
    } else {
      $this->base = $config->current['BASE'];
    }
  }

  /*!
   * \brief Refresh the bases list
   */
  function refreshBasesList ()
  {
    // Fill internal bases list
    $this->bases = static::getReleaseList();

    // Populate base selector if already present
    if ($this->baseSelector && $this->baseMode) {
      $this->baseSelector->setBases($this->bases);
      $this->baseSelector->setBase($this->base);
      $this->baseSelector->update(TRUE);
    }
  }

  public function noFaiBranches (): bool
  {
    return empty($this->bases);
  }

  /*! \brief   Returns a list of all releases for useable for drop down boxes.
   */
  public static function getReleaseList (): array
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=FAIbranch)', ['ou','FAIstate']);

    $list = [];

    while ($release = $ldap->fetch()) {
      preg_match('/[^,],ou=([^,]+),/', $release['dn'], $m);
      $list[$release['dn']] = $m[1].'/'.$release['ou'][0];
    }

    return $list;
  }
}
