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
      'personnalUniqueID' => [
        'name'  => _('Personal Unique ID'),
        'attrs' => [
         new BooleanAttribute(
            _('Activate Unique ID (Cannot Be Unset)'), _('Generate Unique ID for User'),
            'unique_ids', FALSE
          ),
          new StringAttribute(
            _('Generated User Unique Identifier'), _('eduPersonUniqueId - User Unique Identifier'),
            'eduPersonUniqueId', FALSE
          ),
          new StringAttribute(
            _('Establishment DNS'), _('DNS of the related users base establishment'),
            'fdEduPersonUniqueIdDns', FALSE
          ),
        ]
      ],
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

    $this->attributesAccess['eduPersonUniqueId']->setUnique('whole');
    $this->attributesAccess['eduPersonUniqueId']->setDisabled(TRUE);
    $this->attributesAccess['unique_ids']->setInLdap($this->is_template);
    $this->attributesAccess['unique_ids']->setManagedAttributes(
      [
        'disable' => [
          '' => [
            'fdEduPersonUniqueIdDns',
          ]
        ]
      ]
    );

    //Allows the ID of once set to always remain set
    if (!empty($this->attributesAccess['fdEduPersonUniqueIdDns']->getValue())) {
      $this->attributesAccess['unique_ids']->setValue(TRUE);
    }

    if (empty($this->attributesAccess['eduPersonUniqueId']->getValue())) {
      $this->setGeneratedUniqueID();
    }

    $aCodePopulation = $this->getLdapPopulationCodes();
    if (!empty($aCodePopulation)) {
      $this->attributesAccess['supannCodePopulation']->attribute->setChoices($aCodePopulation);
    }
  }

  /*
   * Verify the DNS entry contains "@" char
   */
  protected function shouldSave (): bool
  {
    if ($this->attributesAccess['unique_ids']->getValue() == TRUE ) {
      preg_match('/^@.*/', $this->attributesAccess['fdEduPersonUniqueIdDns']->getValue(), $match);
      if (empty($match[0]) || $this->attributesAccess['fdEduPersonUniqueIdDns']->getValue() !== $match[0]) {
        msg_dialog::display(_('Warning'), sprintf(_('Incorrect DNS format or entry. Example : "@example.com" !')), WARNING_DIALOG);
        return FALSE;
      }
    } else {
        $this->attributesAccess['eduPersonUniqueId']->setInLdap($this->is_template);
    }

    return parent::shouldSave();
  }

  /*
   * Save method, by interface rule, must remain public
   */
  public function save (): array
  {
    //Important to get attrs from ldap fdtemplate and to not register them within ldap afterwards
    $this->attributesAccess['unique_ids']->setInLdap($this->is_template);

    //Combine DNS & UniqueID
    $this->attributesAccess['eduPersonUniqueId']->setValue($this->createFinaleUniqueUserID());

    if ($this->attributesAccess['unique_ids']->getValue() == FALSE) {
      $this->attributesAccess['eduPersonUniqueId']->setInLdap(FALSE);
    }

    $errors = parent::save();
    return $errors;
  }

  /*
   * Form the unique ID and DNS together to create the proper userUniqueID.
   * If DNS already set, replace it with the update one.
   */
  protected function createFinaleUniqueUserID () : string
  {
    $dns = $this->attributesAccess['fdEduPersonUniqueIdDns']->getValue();
    $id  = $this->attributesAccess['eduPersonUniqueId']->getValue();

    preg_match('/@.*/', $this->attributesAccess['eduPersonUniqueId']->getValue(), $match);
    if (empty($match[0])) {
      return $finalID = $id.$dns;
    } else {
      $id = str_replace($match, $dns, $id);
      return $id;
    }
  }

  /*
   * Generate a Uniquer User ID of maximum 64 char
   * Generated by the existing user ID and a the configured salt key and microtime from uniqid.
   */
  protected function generateUniqueUserId (): string
  {
    $uniqueID = substr(base64_encode(uniqid('', TRUE)), 0, 63);
    return  trim($uniqueID, '=');
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
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdSupannExt))');

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

  function adapt_from_template (array $attrs, array $skip = [])
  {
    parent::adapt_from_template($attrs, $skip);

    //Allows the ID of once set to always remain set
    if (!empty($this->attributesAccess['fdEduPersonUniqueIdDns']->getValue())) {
      $this->attributesAccess['unique_ids']->setValue(TRUE);
    }

    //Make sure the ID cannot be re-used if adapted from template
    $this->setGeneratedUniqueID();

    $this->fromTemplate = TRUE;
  }

  protected function setGeneratedUniqueID () : void
  {
    $this->aUniqueId = $this->generateUniqueUserId();
    $this->attributesAccess['eduPersonUniqueId']->setValue($this->aUniqueId);
  }

}
