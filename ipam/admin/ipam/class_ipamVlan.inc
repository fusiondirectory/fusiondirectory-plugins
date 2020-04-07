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

class ipamVlan extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('VLAN'),
      'plDescription' => _('VLAN IPAM object'),
      'plObjectClass' => ['fdIpamVlan'],
      'plObjectType'  => ['ipamVlan' => [
        'name'        => _('VLAN'),
        'ou'          => get_ou('ipamVlanRDN'),
        'icon'        => 'geticon.php?context=types&icon=vlan&size=16',
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
          new BaseSelectorAttribute(get_ou('ipamSubnetRDN')),
          new HostNameAttribute(
            _('Name'), _('Subnet name'),
            'cn', TRUE
          ),
          new TextAreaAttribute(
            _('Description'), _('Description of this simple security object'),
            'description', FALSE
          ),
          new BooleanAttribute(
            _('Active'), _('Is this subnet active'),
            'fdIpamActive', TRUE
          ),
          new StringAttribute(
            _('Organization'), _('Subnet organization'),
            'o', FALSE
          ),
          new IntAttribute(
            _('Inner ID'), _('Inner VLAN ID'),
            'fdIpamVlanInnerId', FALSE,
            1, 4096
          ),
          new IntAttribute(
            _('QinQ ID'), _('QinQ VLAN ID'),
            'fdIpamVlanQinQId', FALSE,
            1, 4096
          ),
          new StringAttribute(
            _('Applicant'), _('Applicant'),
            'fdIpamApplicant', FALSE
          ),
        ]
      ],
    ];
  }
}
