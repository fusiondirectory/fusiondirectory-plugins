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

class supannResourceStates extends simpleplugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupannState'),
      'plDescription' => _('Supann State'),
      'plObjectClass' => ['fdSupannObjects'],
      'plFilter'      => '(objectClass=SupannObjects)',
      'plObjectType'  => [
        'supannObjects' => [
          'name'        => _('supannObjects'),
          'description' => _('supannObjects'),
          'icon'        => 'geticon.php?context=devices&icon=telephone&size=16',
          'ou'          => get_ou('supannObjectsRDN'),
        ]
      ],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('supannObjectsRDN')),
          new StringAttribute(
            _('Name'), _('The name of the supann resource'),
            'fdSupannResourceName', TRUE
          ),
          new StringAttribute(
            _('Label'), _('The label of the supann resource'),
            'fdSupannRessourceLabel', TRUE
          ),
          new StringAttribute(
            _('Description'), _('A short description of supann resource'),
            'description', FALSE
          ),
        ]
      ],
    ];
  }
}
