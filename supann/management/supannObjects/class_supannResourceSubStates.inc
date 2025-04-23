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
      'plShortName'   => _('SupannSubState'),
      'plDescription' => _('Supann Sub State'),
      'plObjectClass' => ['fdSupannResourceSubStates'],
      'plFilter'      => '(objectClass=fdSupannResourceSubStates)',
      'plObjectType'  => [
        'subresource' => [
          'name'        => _('SupannSubState'),
          'description' => _('SupannSubState'),
          'icon'        => 'geticon.php?context=devices&icon=telephone&size=16',
          'ou'          => get_ou('fdSupannObjectsRDN'),
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
          new BaseSelectorAttribute(get_ou('fdSupannObjectsRDN')),
          new StringAttribute(
            _('Name'), _('The name of the supann resource'),
            'fdSupannSubStateName', TRUE
          ),
          new SelectAttribute (
            _('Status'),                     // Label of the attribute
            _('Supann Status'),         // Description
            'fdSupannRessourceState',                    // LDAP name
            TRUE,                           // Mandatory
            ['A','S','I'],      // [SelectAttribute] Choices
            '', // We don't set any default value, it will be the first one
            ['Active','Sursis','Inactive']       // [SelectAttribute] Output choices
          ),
          new StringAttribute(
            _('Label'), _('The label of the supann resource'),
            'fdSupannSubStateLabel', FALSE
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
