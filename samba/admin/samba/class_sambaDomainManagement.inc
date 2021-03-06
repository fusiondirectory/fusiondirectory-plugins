<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2021 FusionDirectory

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

class sambaDomainManagement extends management
{
  protected $skipCpHandler  = TRUE;

  /* Default columns - SID instead of description */
  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'nameAttr',    'label' => 'Name']],
    ['LinkColumn',       ['attributes' => 'sambaSID',    'label' => 'SID']],
    ['ActionsColumn',    ['label' => 'Actions']],
  ];

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Samba domains'),
      'plTitle'       => _('Samba domain management'),
      'plDescription' => _('Manage Samba domains'),
      'plIcon'        => 'geticon.php?context=applications&icon=samba&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 40,
      'plManages'     => ['sambaDomain'],
    ];
  }

  protected function setUpListing ()
  {
    /* Disable base mode */
    $this->listing  = new managementListing($this, FALSE);
  }

  protected function configureActions ()
  {
    parent::configureActions();

    /* Remove create menu */
    foreach ($this->actions['new']->listActions() as $actionName) {
      unset($this->actionHandlers[$actionName]);
    }
    unset($this->actions['new']);
  }

  public static function getSambaDomains (): array
  {
    global $config;

    if (isset($config->data['SERVERS']['SAMBA'])) {
      return $config->data['SERVERS']['SAMBA'];
    }

    $domains  = objects::ls('sambaDomain', ['sambaDomainName' => 1, 'sambaSID' => 1, 'sambaAlgorithmicRidBase' => 1]);
    $data     = [];
    foreach ($domains as $domain) {
      $data[$domain['sambaDomainName']] = [
        'SID'     => ($domain['sambaSID'] ?? $config->get_cfg_value('SambaSID')),
        'RIDBASE' => ($domain['sambaAlgorithmicRidBase'] ?? $config->get_cfg_value('SambaRidBase')),
      ];
    }

    /* If no samba domains are found, create a fake one */
    if ((count($data) == 0)) {
      $data['DEFAULT'] = [
        'SID'     => $config->get_cfg_value('SambaSID'),
        'RIDBASE' => $config->get_cfg_value('SambaRidBase'),
      ];
    }

    $config->data['SERVERS']['SAMBA'] = $data;
    return $config->data['SERVERS']['SAMBA'];
  }

  public static function resetDomainsCache ()
  {
    global $config;

    unset($config->data['SERVERS']['SAMBA']);
  }
}
