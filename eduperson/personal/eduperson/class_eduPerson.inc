<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class eduPerson extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('eduPerson'),
      'plDescription' => _('Eduperson'),
      'plIcon'        => 'geticon.php?context=applications&icon=eduperson&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=eduperson&size=16',
      'plObjectType'  => ['user'],
      'plObjectClass' => ['eduPerson','fdEduPerson'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'identity' => [
        'name'  => _('Identity'),
        'attrs' => [
          new StringAttribute(
            _('EPPN'), _('eduPersonPrincipalName - A name that looks like <id>@<domain> which is unique for this domain, and has not be assigned to anyone else recently'),
            'eduPersonPrincipalName', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
            _('Prior EPPN'), _('eduPersonPrincipalNamePrior - History of EPPN'),
            'eduPersonPrincipalNamePrior', FALSE
            )
          ),
          new StringAttribute(
            _('Pronouns'), _('eduPersonDisplayPronouns - a person pronouns'),
            'eduPersonDisplayPronouns', FALSE
          ),
          new StringAttribute(
            _('Nickname'), _('eduPersonNickname - Can contain a nickname for this user'),
            'eduPersonNickname', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              _('ORCID identifiers'), _('eduPersonOrcid - ORCID researcher identifiers belonging to the principal'),
              'eduPersonOrcid', FALSE,
              '', '',
              // Validation regexp: it must be XXXX-XXXX-XXXX-XXXX when X are digits
              '/^[0-9]{4}-[0-9]{4}-[0-9]{4}-[0-9]{3}[0-9X]$/',
              'ORCID account IDs must look like XXXX-XXXX-XXXX-XXXX where X are digits'
            )
          )
        ]
       ],
      'Federation' => [
        'name'  => _('Federation and Access'),
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
          new SetAttribute(
            new StringAttribute(
              _('Entitlement'), _('eduPersonEntitlement - Access right to external applications'),
              'eduPersonEntitlement', FALSE
            )
          ),
          new StringAttribute(
            _('Scoped Affiliation'), _('eduPersonScopedAffiliation -  Affiliation within a particular security domain'),
            'eduPersonScopedAffiliation', FALSE
          ),
          new StringAttribute(
            _('Assurance'), _('eduPersonAssurance -   identity assurance profiles (IAPs)'),
            'eduPersonAssurance', FALSE
          ),
        ]
      ],
      'Orgnisation' => [
        'name'  => _('Organisation'),
        'attrs' => [
          new StringAttribute(
            _('Organisation DN'), _('eduPersonOrgDN - Main organisation'),
            'eduPersonOrgDN', FALSE
          ),
          new StringAttribute(
            _('Primary Organisation DN'), _('eduPersonPrimaryOrgUnitDN - Main attachment Unit'),
            'eduPersonPrimaryOrgUnitDN', FALSE
          ),
          new StringAttribute(
            _('Unit DN'), _('eduPersonOrgUnitDN - Main Unit'),
            'eduPersonOrgUnitDN', FALSE
          ),
         ]
       ],
      'affiliation' => [
        'name'  => _('Affiliation'),
        'attrs' => [
          new SelectAttribute(
            _('Primary affiliation'), _('eduPersonPrimaryAffiliation - Main status of the person'),
            'eduPersonPrimaryAffiliation', FALSE
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Affiliations'), _('eduPersonAffiliation - Status of the person: student, BIATOSS, teacher, contract, retired, hosted staff (CNRS, INSERM, etc.), a former student, etc.'),
              'eduPersonAffiliation', FALSE
            )
          )
         ]
       ],

    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['eduPersonPrincipalName']->setUnique('whole');


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
    $this->eduPersonInit();
  }

  protected function eduPersonInit ()
  {
    global $config;

    // to be changed to a supann object !!
    list ($codes, $labels) = supann::get_choices_for('affiliation');
    $this->attributesAccess['eduPersonAffiliation']->attribute->setChoices($codes, $labels);

    $this->updateFieldsChoicesAndValues();
  }

  /* Update choices of fields which depends on other fields values */
  protected function updateFieldsChoicesAndValues ()
  {
    global $config;

    $this->attributesAccess['eduPersonPrimaryAffiliation']->setChoices(
      $this->attributesAccess['eduPersonAffiliation']->getValue(),
      $this->attributesAccess['eduPersonAffiliation']->getDisplayValues()
    );
  }

  function check (): array
  {
    $errors = parent::check();

    $affiliations = $this->eduPersonAffiliation;
    if (in_array('member', $affiliations) && in_array('affiliate', $affiliations)) {
      $errors[] = new SimplePluginCheckError(
        $this->attributesAccess['eduPersonAffiliation'],
        htmlescape(_('"member" and "affiliate" values are incompatible for eduPersonAffiliation. Please remove one of them.'))
      );
    }

    $orcids = $this->eduPersonOrcid;
    if (is_array($orcids)) {
      foreach ($orcids as $orcid) {
        // phpstan:ignore
        if (!tests::is_orcid($orcid)) {
          $errors[] = new SimplePluginCheckError(
            $this->attributesAccess['eduPersonOrcid'],
            htmlescape(_('The value "'.$orcid.'" is not a valid orcid id. Please remove it.'))
          );
        }
      }
    }

    return $errors;
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
