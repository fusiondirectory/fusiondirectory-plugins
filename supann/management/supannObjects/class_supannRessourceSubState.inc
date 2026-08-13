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

class supannRessourceSubState extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Sub State'),
      'plDescription' => _('SupAnn Sub State'),
      'plObjectClass' => ['fdSupannRessourceSubState'],
      'plFilter'      => '(objectClass=fdSupannRessourceSubState)',
      'plObjectType'  => ['supannRessourceSubState' =>
        [
        'name'        => _('SupAnn Sub State'),
        'description' => _('SupAnn Sub State'),
        'ou'          => get_ou('SupannSubStateRDN'),
        'icon'        => 'geticon.php?context=applications&icon=supann-entite&size=16',
        'mainAttr'    => 'fdSupannSubStateName',
        ]
       ],
      'plSearchAttrs' => ['description', 'fdSupannSubStateName'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE)
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Ressource Sub State'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannSubStateRDN')),
          new StringAttribute(
            _('Sub State Name'), _('The name of the Supann Sub State'),
            'fdSupannSubStateName', TRUE
          ),
          new StringAttribute(
            _('Sub State Label'), _('The label of the Supann Sub State'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('Sub State Description'), _('The description explaining this Supann Sub State'),
            'description', FALSE
          ),
        ]
      ],
    ];
  }
}
