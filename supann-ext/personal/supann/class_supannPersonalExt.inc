<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2013-2022 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class supannPersonalExt extends simplePlugin
{
  protected $displayHeader = TRUE;

  public $aUniqueId;
  public $fromTemplate;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SUPANN-Ext'),
      'plDescription' => _('SUPANN Non-Official Extension'),
      'plObjectType'  => ['user'],
      'plObjectClass' => ['fdSupannExt'],
      'plDepends'     => ['supannAccount'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'populationCode' => [
        'name'  => _('Population Code'),
        'attrs' => [
          new SetAttribute(
            new SelectAttribute(
              'Main Code', _('Main Population Code'),
              'supannCodePopulation', FALSE,
            ),
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $aCodePopulation = $this->getLdapPopulationCodes();
    if (!empty($aCodePopulation)) {
      $this->attributesAccess['supannCodePopulation']->attribute->setChoices($aCodePopulation);
    }
  }

  /*
   * Return table of existing Population Codes containing main's and local's
   */
  protected function getLdapPopulationCodes (): array
  {
    global $config;
    $result = [];
    $populationCodes = [];

    $ldap = $config->get_ldap_link();
    $dn = 'cn=config,'.$config->data['OBJECTS']['CONFIGURATION']['ou'].$config->current['BASE'];
    $ldap->cd($dn);
    $ldap->search('(objectClass=fdSupannExt)');

    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['fdMainPopulationCodeConf'][0])) {
        $populationCodes[] = $attrs['fdMainPopulationCodeConf'];
      }
      if (isset($attrs['fdLocalPopulationCodeConf'][0])) {
        $populationCodes[] = $attrs['fdLocalPopulationCodeConf'];
      }
    }

    //Verification if the values within the multidim assoc array match the patterns
    for ($i = 0; $i < count($populationCodes); $i++) {
      foreach ($populationCodes[$i] as $key => $value) {
        if (preg_match('/^{/', $value)) {
          $result[] = $value;
        }
      }
    }

    return $result;
  }
}
