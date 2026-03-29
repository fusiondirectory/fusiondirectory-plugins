<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019  FusionDirectory

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

class supannCivilite extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Civility'),
      'plDescription' => _('SupAnn Civility'),
      'plObjectClass' => ['fdSupannCivilite'],
      'plFilter'      => '(objectClass=fdSupannCivilite)',
      'plObjectType'  => [
        'supannCivilite' => [
          'name'         => _('SupAnn Civility'),
          'description'  => _('SupAnn Civility'),
          'ou'           => get_ou('SupannCiviliteRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannCiviliteName',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannCiviliteName'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Civility'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannCiviliteRDN')),
          new StringAttribute(
            _('Civility Name'), _('The name of the Supann Civility'),
            'fdSupannCiviliteName', TRUE
          ),
          new StringAttribute(
            _('Civility Label'), _('The label of the Supann Civility'),
            'fdSupannCiviliteLabel', TRUE
          ),
          new StringAttribute(
            _('Civility Description'), _('The description explaining this Supann Civility'),
            'description', FALSE
          ),
        ],
      ],
    ];
  }
}
