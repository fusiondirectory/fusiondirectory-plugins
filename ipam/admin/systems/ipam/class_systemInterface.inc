<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2019-2020 FusionDirectory

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

class networkInterface extends simplePlugin
{
  public $base;

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Interface'),
      'plDescription' => _('Network interface'),
      'plObjectClass' => ['fdNetworkInterface'],
      'plObjectType'  => ['networkInterface' => [
        'name'        => _('Network interface'),
        'ou'          => '',
        'icon'        => 'geticon.php?context=category&icon=network&size=16',
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('General information'),
        'attrs' => [
          new HostNameAttribute(
            _('Name'), _('Interface name'),
            'cn', TRUE
          ),
          new MacAddressAttribute(
            _('Mac address'), _('Mac address of this system'),
            'macAddress', FALSE
          ),
          new SelectAttribute(
            _('VLAN'), _('VLAN'),
            'fdNetworkInterfaceVlanId', FALSE
          ),
          new StringAttribute(
            _('VLAN tag'), _('VLAN tag'),
            'fdNetworkInterfaceVlanTag', FALSE
          ),
          new ObjectSelectAttribute(
            _('Subnet'), _('Subnet'),
            'fdNetworkInterfaceSubnetDN', FALSE,
            'ipamSubnet'
          ),
          new SetAttribute(
            new IPAttribute(
              _('IP address'), _('IP addresses this system uses (v4 or v6)'),
              'ipHostNumber', FALSE
            )
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $vlans    = objects::ls('ipamVlan', ['fdIpamVlanInnerId' => 1,'cn' => 1]);
    $this->attributesAccess['fdNetworkInterfaceVlanId']->setChoices(array_column($vlans, 'fdIpamVlanInnerId'), array_column($vlans, 'cn'));
  }
}
