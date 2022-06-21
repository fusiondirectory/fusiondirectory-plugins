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

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [

      'personnal' => [
        'name'  => _('Personal profile'),
        'attrs' => [
         new BooleanAttribute(
            _('Activate Unique ID'), _('Generate Unique ID for User'),
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
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['eduPersonUniqueId']->setUnique('whole');
    $this->attributesAccess['eduPersonUniqueId']->setDisabled(!$this->is_template);
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

    if (empty($this->attributesAccess['eduPersonUniqueId']->getValue())) {
      $this->aUniqueId = $this->generateUniqueUserId();
      if (!$this->isUserUniqueIdExist($this->aUniqueId)) {
        $this->attributesAccess['eduPersonUniqueId']->setValue($this->aUniqueId);
      }
    }
  }

  /*
   * Verify the DNS entry contains "@" char
   */
  protected function shouldSave (): bool
  {
    preg_match('/^@.*/', $this->attributesAccess['fdEduPersonUniqueIdDns']->getValue(), $match);
    if (empty($match[0]) || $this->attributesAccess['fdEduPersonUniqueIdDns']->getValue() !== $match[0]) {
      msg_dialog::display(_('Warning'), sprintf(_('Incorrect DNS format or entry. Example : "@example.com" !')), WARNING_DIALOG);
      return FALSE;
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
    $this->attributesAccess['eduPersonUniqueId']->setValue($this->createFinaleUniqueUserID());

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
      error_log('before_replace: ' .print_r($id, TRUE));
      $id = str_replace($match, $dns, $id);
      error_log('after_replace: ' .print_r($id, TRUE));
      return $id;
    }
  }

  /*
   * Generate a Uniquer User ID of maximum 64 char
   */
  protected function generateUniqueUserId (): string
  {
    $uniqueID = substr(base64_encode(uniqid('', TRUE)), 0, 63);
    return  trim($uniqueID, '=');
  }

  /*
   * Verify if the generated uniqueID is not alread existing
   */
  protected function isUserUniqueIdExist (string $uniqueID): bool
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdSupannExt))');
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['eduPersonUniqueId'][0])) {
        if ($attrs['eduPersonUniqueId'][0] == $uniqueID) {
          error_log('TRUE');
          return TRUE;
        }
      }
    }

    return FALSE;
  }


}
