<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2012-2016  FusionDirectory

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

class componentGeneric extends ipHostPlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Component'),
      'plDescription' => _('Component information'),
      'plObjectClass' => ['device', 'ieee802Device'],
      'plObjectType'  => [
        'component' => [
          'name'        => _('Network device'),
          'description' => _('Network device'),
          'icon'        => 'geticon.php?context=devices&icon=network-device&size=16',
          'ou'          => get_ou('componentRDN'),
        ]
      ],
      'plSearchAttrs' => ['description', 'ipHostNumber', 'macAddress'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('componentRDN')),
          new HostNameAttribute(
            _('Name'), _('The name of the component'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('A short description of the component'),
            'description', FALSE
          ),
          new HiddenArrayAttribute('ipHostNumber', TRUE),
          new HiddenArrayAttribute('macAddress', FALSE),
        ],
      ],
    ];
  }
}
