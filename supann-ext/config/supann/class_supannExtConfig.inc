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
      'localPopulationCodeConf' => [
        'name'  => _('Local Population Codes'),
        'attrs' => [
         new SetAttribute(
            new CompositeAttribute(
              _('Code Configuration'),
              'fdLocalPopulationCodeConf',
              [
                new SelectAttribute(
                  'Main Code', _('Main Population Code'),
                  'fdMainPopulationCode', FALSE,
                ),
                new StringAttribute(
                  _('Local Population Code'), _('Local Population Code'),
                  'fdLocalPopulationCode', FALSE, ''
                ),
              ],
              '/^{/',
              '{%s}:%s',
              '',
              _('Local Population Codes')
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
               new SelectAttribute(
                  _('Prefix'),    _('Prefix Of The Main Population Code'),
                  'fdsupannMainPopulationCodePrefix', FALSE,
                  ['SUPANN']
                ),
               new StringAttribute(
                  _('Main Population Code'), _('Main Population Code'),
                  'fdMainPopulationCode', FALSE, ''
                ),
              ],
              '/.*/',
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
      '{SUPANN}R'    => _('{SUPANN}R'),
      '{SUPANN}RG'   => _('{SUPANN}RG'),
      '{SUPANN}RGP'  => _('{SUPANN}RGP'),
      '{SUPANN}RGPF' => _('{SUPANN}RGPF'),
      '{SUPANN}RGPFT' => _('{SUPANN}RGPFT'),
    ];

    $populationCodeMaster += $this->attributesAccess['fdMainPopulationCodeConf']->getValue();
    $populationCodeMaster = array_unique($populationCodeMaster);

    $this->attributesAccess['fdLocalPopulationCodeConf']->attribute->attributes[0]->setChoices(
      array_keys($populationCodeMaster),
      array_values($populationCodeMaster)
    );
  }

  /*
   * Return table of existing populationCodeMaster
   */
  protected function getLdapMainPopulationCodes (): array
  {
    global $config;
    $populationCodeMaster = [];

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdSupannExt))');

    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['fdMainPopulationCodeConf'][0])) {
        $populationCodeMaster[] = $attrs['fdMainPopulationCodeConf'];
      }
    }

    return $populationCodeMaster;
  }

}
