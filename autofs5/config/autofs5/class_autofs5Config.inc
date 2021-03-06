<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2020 FusionDirectory

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

class autofs5Config extends multiPluginSection
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Autofs configuration'),
      'plDescription'   => _('FusionDirectory autofs plugin configuration'),
      'plObjectClass'   => ['fdAutofs5PluginConf'],
      'plCategory'      => ['configuration'],
      'plObjectType'    => ['smallConfig'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'autofs5' => [
        'name'  => _('AutoFS'),
        'attrs' => [
          new StringAttribute(
            _('AutoFS RDN'), _('Branch in which automount info will be stored'),
            'fdAutofs5RDN', TRUE,
            'ou=autofs'
          ),
        ]
      ],
    ];
  }
}
