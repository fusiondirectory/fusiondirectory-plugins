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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannRessourceState extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('State'),
      'plDescription' => _('SupAnn State'),
      'plObjectClass' => ['fdSupannRessourceState'],
      'plFilter'      => '(objectClass=fdSupannRessourceState)',
      'plObjectType'  => ['supannRessourceState' =>
        [
        'name'        => _('SupAnn State'),
        'description' => _('SupAnn State'),
        'ou'          => get_ou('SupannStateRDN'),
        'icon'        => 'geticon.php?context=applications&icon=supann-entite&size=16',
        'mainAttr'    => 'fdSupannStateName',
        ]
       ],
      'plSearchAttrs' => ['description', 'fdSupannStateName'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE)
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Ressource State'),
        'attrs' => [
          new StringAttribute(
            _('State Name'), _('The name of the Supann State'),
            'fdSupannStateName', TRUE
          ),
          new StringAttribute(
            _('State Label'), _('The label of the Supann State'),
            'fdSupannRessourceLabel', TRUE
          ),
          new StringAttribute(
            _('State Description'), _('The description explaining this Supann State'),
            'description', TRUE
          ),
        ]
      ],
    ];
  }
}
