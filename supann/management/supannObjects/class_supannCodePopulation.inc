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

class supannCodePopulation extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Population Code'),
      'plDescription' => _('SupAnn Population Code'),
      'plObjectClass' => ['fdSupannPopulationCode'],
      'plFilter'      => '(objectClass=fdSupannPopulationCode)',
      'plObjectType'  => [
        'supannCodePopulation' => [
          'name'         => _('SupAnn Population Code'),
          'description'  => _('SupAnn Population Code'),
          'ou'           => get_ou('SupannPopulationCodeRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannPopulationCodeName',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannPopulationCodeName'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Population Code'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannPopulationCodeRDN')),
          new StringAttribute(
            _('Population Code Name'), _('The name of this Supann Population Code'),
            'fdSupannPopulationCodeName', TRUE
          ),
          new StringAttribute(
            _('Population Code Label'), _('The label of this Supann Population Code'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('Population Code Description'), _('The description explaining this Supann Population Code'),
            'description', FALSE
          ),
        ],
      ],
    ];
  }
}
