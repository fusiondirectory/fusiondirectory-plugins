<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2026  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannRole extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn Role'),
      'plDescription' => _('SupAnn Role'),
      'plObjectClass' => ['fdSupannRole'],
      'plFilter'      => '(objectClass=fdSupannRole)',
      'plObjectType'  => [
        'supannRole' => [
          'name'         => _('SupAnn Role'),
          'description'  => _('SupAnn Role'),
          'ou'           => get_ou('SupannRoleRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannLabel',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannName', 'fdSupannLabel'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Role'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannRoleRDN')),
          new StringAttribute(
            _('Role Name'), _('The name of this Supann Role'),
            'fdSupannName', TRUE, '', '', '/^{.+?}.+$/', '{PREFIX}VALUE'
          ),
          new StringAttribute(
            _('Role Label'), _('The label of this Supann Role'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('Role Description'), _('The description explaining this Supann Role'),
            'description', FALSE
          ),
        ],
      ],
    ];
  }
}
