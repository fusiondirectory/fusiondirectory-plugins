<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2019  FusionDirectory

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

class nisMap extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Mount point'),
      'plDescription'   => _('Autofs mount point'),
      'plObjectClass'   => ['nisMap'],
      'plObjectType'    => ['nisMap' => [
        'name'        => _('Mount point'),
        'ou'          => get_ou('autofsRDN'),
        'icon'        => 'geticon.php?context=applications&icon=autofs-nis-netmap&size=16',
        'mainAttr'    => 'nisMapName',
      ]],
      'plSearchAttrs' => ['nisMapName'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Mount point'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('autofsRDN')),
          new HostNameAttribute(
            _('Name'), _('Name of the mount point'),
            'nisMapName', TRUE
          )
        ]
      ]
    ];
  }
}
