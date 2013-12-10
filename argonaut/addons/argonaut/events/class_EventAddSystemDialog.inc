<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class EventAddSystemDialog extends management
{
  // Tab definition
  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;

  function __construct($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;
    $this->storagePoints = array(
        get_ou("serverRDN"),
        get_ou("workstationRDN"),
        get_ou("sambaMachineAccountRDN"),
        get_ou("ogroupRDN"),
        get_ou("terminalRDN"));

    // Build filter
    $filter = new filter(get_template_path("../../addons/argonaut/events/eventTargetSystems-filter.xml", TRUE));
    $filter->setObjectStorage($this->storagePoints);
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("../../addons/argonaut/events/eventTargetSystems-list.xml", TRUE));
    $headpage->setFilter($filter);
    parent::__construct($config, $ui, "object", $headpage);
  }

  function save_object()
  {
  }

  function save()
  {
    $act = $this->detectPostActions();
    $headpage = $this->getHeadpage();
    if (!isset($act['targets'])) {
      return array();
    }
    $ret = array();
    foreach ($act['targets'] as $dn) {
      $ret[] = $headpage->getEntry($dn);
    }
    return $ret;
  }

  function get_selected_targets()
  {
    // Collect mac addresses
    $entries = $this->save();
    $macs = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $member_dns = array();
    foreach ($entries as $entry) {
      if (isset($entry['macAddress'][0])) {
        $macs[] = $entry['macAddress'][0];
      }
      if (in_array("gosaGroupOfNames", $entry['objectClass'])) {
        $ldap->cat($entry['dn']);
        if ($ldap->count()) {
          $attrs = $ldap->fetch();
          if (isset($attrs['member'])) {
            for ($i = 0; $i < $attrs['member']['count']; $i++) {
              $member_dns[] = $attrs['member'][$i];
            }
          }
        }
      }
    }
    foreach ($member_dns as $dn) {
      $ldap->cat($dn, array('macAddress'));
      if ($ldap->count()) {
        $attrs = $ldap->fetch();
        if (isset($attrs['macAddress'])) {
          $macs[] = $attrs['macAddress'][0];
        }
      }
    }
    return $macs;
  }
}
?>
