<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2026 FusionDirectory

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

class macroConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Macro'),
      'plTitle'         => _('Macro configuration'),
      'plDescription'   => _('FusionDirectory macro plugin configuration'),
      'plObjectClass'   => ['fdMacroPluginConf'],
      'plPriority'      => 18,
      'plObjectType'    => ['configuration'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'ask' => [
        'name'  => _('Macro'),
        'attrs' => [
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Definition of the attributes to be asked'),
              'fdMacroAttributesList',
              [
                new SelectAttribute(
                  _('Attribute class'),
                  _('Class name of the attribute'),
                  'fdMacroAttributeClass',
                  'FALSE',
                  [
                    'StringAttribute'
                  ],
                ),
                new StringAttribute(
                  _('Attribute name'),
                  _('Name of the attribute'),
                  'fdMacroAttributeDescription',
                  FALSE,
                  'AttributeName',
                ),
                new StringAttribute(
                  _('Attribute name label'),
                  _('Label of the attribute'),
                  'fdMacroAttributeLabel',
                  FALSE,
                  'AttributeLabel',
                ),
              ],
              '',
              'List of asked attributes',
            ),
            $order = TRUE,
            $values = [],
            $edit_enabled = TRUE
          )
        ]
      ],
    ];
  }
}
