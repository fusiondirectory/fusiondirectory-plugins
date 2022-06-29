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
      'mainPopulationCodeConf' => [
        'name'  => _('Main Population Codes'),
        'attrs' => [
         new SetAttribute(
            new CompositeAttribute(
              _('Main Codes Configuration'),
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
      'localPopulationCodeConf' => [
        'name'  => _('Local Population Codes'),
        'attrs' => [
         new SetAttribute(
            new CompositeAttribute(
              _('Local Codes Configuration'),
              'fdLocalPopulationCodeConf',
              [
                new SelectAttribute(
                  'Main Code', _('Main Population Code'),
                  'fdMainPopulationCodeSet', FALSE,
                ),
                new StringAttribute(
                  _('Local Population Code'), _('Local Population Code'),
                  'fdLocalPopulationCode', FALSE,
                ),
              ],
              '/.*/',
              '%s:%s',
              '',
              _('Local Population Codes')
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

    $this->setBasicMainCodes();
  }

  /*
   * Return table of existing populationCodeMaster
   */
  protected function getLdapMainPopulationCodes (): array
  {
    global $config;
    $popCodeMaster = [];

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdSupannExt))');

    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['fdMainPopulationCodeConf'][0])) {
        $popCodeMaster[] = $attrs['fdMainPopulationCodeConf'];
      }
    }

    return $popCodeMaster;
  }

  /*
   * Required to make sure the basic template of population code is always available.
   * In the tabl of Main and Local sections.
   */
  protected function setBasicMainCodes () : void
  {
    $this->populationCodeMaster = [
      '{SUPANN}R'    => _('{SUPANN}R'),
      '{SUPANN}RG'   => _('{SUPANN}RG'),
      '{SUPANN}RGP'  => _('{SUPANN}RGP'),
      '{SUPANN}RGPF' => _('{SUPANN}RGPF'),
      '{SUPANN}RGPFT' => _('{SUPANN}RGPFT'),
    ];

    //Getting values from LDAP already registered and add the default ones, making array unique if already defined.
    $this->populationCodeMaster += $this->attributesAccess['fdMainPopulationCodeConf']->getValue();
    $this->populationCodeMaster = array_unique($this->populationCodeMaster);

    $this->attributesAccess['fdLocalPopulationCodeConf']->attribute->attributes[0]->setChoices(
      array_values($this->populationCodeMaster)
    );

    $this->attributesAccess['fdMainPopulationCodeConf']->setValue(
      array_values($this->populationCodeMaster)
    );
  }

  /*
   * Must return bool to be compliant with the interface
   */
  public function update (): bool
  {
    parent::update();
    $this->updateFieldsChoicesAndValues();
    $this->attributesAccess['fdLocalPopulationCodeConf']->setValue($this->deleteLocalPopuplationCode());

    return TRUE;
  }

  /*
   *  Local Population Code is deleted if the parent/main code is not existing.
   */
  protected function deleteLocalPopuplationCode () : array
  {
    $result = [];
    foreach ($this->attributesAccess['fdMainPopulationCodeConf']->getValue() as $mCodes) {
      foreach ($this->attributesAccess['fdLocalPopulationCodeConf']->getValue() as $lCodes) {
        preg_match('/^{.*}(.*)/', $mCodes, $mainMatch);
        preg_match('/^{.*}(.*):/', $lCodes, $localMatch);
        if (isset($mainMatch[1], $localMatch[1]) && $mainMatch[1] == $localMatch[1]) {
          $result[] = $lCodes;
        }
      }
    }
    return $result;
  }

  protected function updateFieldsChoicesAndValues ()
  {
    $this->attributesAccess['fdLocalPopulationCodeConf']->attribute->attributes[0]->setChoices(
      $this->attributesAccess['fdMainPopulationCodeConf']->getValue(),
      $this->attributesAccess['fdMainPopulationCodeConf']->getDisplayValues()
    );
  }

  /*
   * Save method, by interface rule, must remain public
   */
  public function save (): array
  {
    $this->setBasicMainCodes();

    $errors = parent::save();
    return $errors;
  }

}
