<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class serverGeneric extends workstationGeneric
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Server'),
      'plDescription' => _('Server information'),
      'plObjectClass' => ['fdServer', 'ieee802Device'],
      'plFilter'      => '(objectClass=fdServer)',
      'plObjectType'  => [
        'server' => [
          'name'        => _('Server'),
          'description' => _('Server'),
          'icon'        => 'geticon.php?context=devices&icon=server&size=16',
          'ou'          => get_ou('serverRDN'),
        ]
      ],
      'plSearchAttrs' => ['description', 'ipHostNumber', 'macAddress'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo ($word = NULL, $rdn = NULL): array
  {
    if ($word === NULL) {
      $word = _('server');
    }
    if ($rdn === NULL) {
      $rdn = get_ou('serverRDN');
    }
    return parent::getAttributesInfo($word, $rdn);
  }
}
