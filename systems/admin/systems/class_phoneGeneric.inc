<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2018 FusionDirectory

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

class phoneGeneric extends ipHostPlugin
{
  var $mainTab = TRUE;

  var $objectclasses = ['fdPhone', 'ieee802Device'];

  var $inheritance = ['gosaGroupOfNames' => 'member'];

  static function plInfo ()
  {
    return [
      'plShortName'   => _('Phone'),
      'plDescription' => _('Phone information'),
      'plObjectType'  => [
        'phone' => [
          'name'        => _('Phone'),
          'description' => _('Phone hardware'),
          'filter'      => 'objectClass=fdPhone',
          'icon'        => 'geticon.php?context=devices&icon=telephone&size=16',
          'ou'          => get_ou('phoneRDN'),
        ]
      ],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo ()
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('phoneRDN')),
          new HostNameAttribute(
            _('Name'), _('The name of the phone'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('A short description of the phone'),
            'description', FALSE
          ),
        ]
      ],
      'phone' => [
        'name'  => _('Phone'),
        'attrs' => [
          new StringAttribute(
            _('Serial Number'), _('The serial number of the phone'),
            'serialNumber', FALSE
          ),
          new StringAttribute(
            _('Telephone Number'), _('The telephone number of the phone'),
            'telephoneNumber', FALSE
          ),
        ]
      ],
      'network' => [
        'name'      => _('Network settings'),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
        'attrs'     => [
          new SetAttribute(
            new IPAttribute(
              _('IP address'), _('IP addresses this system uses (v4 or v6)'),
              'ipHostNumber', FALSE
            )
          ),
          new SetAttribute(
            new MacAddressAttribute(
              _('Mac address'), _('Mac address of this system'),
              'macAddress', FALSE
            )
          ),
        ],
      ],
    ];
  }
}
?>
