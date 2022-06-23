<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2022 FusionDirectory

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

class supannExtConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('SupannExt'),
      'plTitle'         => _('Supann Ext configuration'),
      'plDescription'   => _('FusionDirectory SupAnn Extension plugin configuration'),
      'plObjectClass'   => ['fdSupannExt'],
      'plObjectType'    => ['configuration'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'populationCodeConf' => [
        'name'  => _('Population Codes'),
        'attrs' => [
         new SetAttribute(
            new CompositeAttribute(
              _('Code Configuration'),
              'fdLocalPopulationCodeConf',
              [
                new SelectAttribute(
                  '', _('Main Population Code'),
                  'fdMasterPopulationCodeM', TRUE
                ),
               new StringAttribute(
                  _('Local Population Code'), _('Local Population Code'),
                  'fdLocalPopulationCode', TRUE, ''
                ),
              ],
              '/^{(.+)}(.*)$/',
              '{%s}%s',
              '',
              _('Population Codes')
            )
          ),
        ]
      ],
      'mainPopulationCodeConf' => [
        'name'  => _('Main Population Codes'),
        'attrs' => [
         new SetAttribute(
            new CompositeAttribute(
              _('Code Configuration'),
              'fdMainPopulationCodeConf',
              [
               new StringAttribute(
                  _('Main Population Code'), _('Main Population Code'),
                  'fdMainPopulationCode', FALSE, ''
                ),
              ],
              '/^{(.+)}(.*)$/',
              '{%s}%s',
              '',
              _('Main Population Codes')
            )
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $populationCodeMaster = [
      'R'    => _('R'),
      'RG'   => _('RG'),
      'RGP'  => _('RGP'),
      'RGPF' => _('RGPF'),
      'RGPFT' => _('RGPFT'),
    ];

    $this->attributesAccess['fdLocalPopulationCodeConf']->attribute->attributes[0]->setChoices(
      array_keys($populationCodeMaster),
      array_values($populationCodeMaster)
    );

    $this->attributesAccess['fdLocalPopulationCodeConf']->setValue(
      array_keys($populationCodeMaster),
      array_values($populationCodeMaster)
    );

  }

}
