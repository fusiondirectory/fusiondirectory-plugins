<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2010-2012 Antoine Gallavardin
  Copyright (C) 2013-2020 FusionDirectory

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

class supannAccount extends simplePlugin
{
  protected $displayHeader = TRUE;

  protected $entitesCache = [];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn'),
      'plDescription' => _('SupAnn information management plugin'),
      'plFilter'      => '(&(objectClass=eduPerson)(objectClass=supannPerson))',
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 14,
      'plObjectClass' => ['eduPerson','supannPerson'],
      'plObjectType'  => ['user'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'identity' => [
        'name'  => _('Identity'),
        'attrs' => [
          new SelectAttribute(
            _('Civilite'), _('supannCivilite - Civility for this person'),
            'supannCivilite', FALSE,
            ['', 'M.', 'Mme', 'Mlle']
          ),
          new StringAttribute(
            _('Alias login'), _('supannAliasLogin - An alias for the login of this user'),
            'supannAliasLogin', FALSE
          ),
          new StringAttribute(
            _('eduPersonPrincipalName'), _('eduPersonPrincipalName - A name that looks like <id>@<domain> which is unique for this domain, and has not be assigned to anyone else recently'),
            'eduPersonPrincipalName', FALSE
          ),
          new StringAttribute(
            _('Nickname'), _('eduPersonNickname - Can contain a nickname for this user'),
            'eduPersonNickname', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              _('Ref ids'), _('supannRefId - IDs/links for this user on other systems'),
              'supannRefId', FALSE,
              '', '',
               // Validation regexp: it must have a prefix
              '/^{[^}]+}.+$/'
            )
          ),
        ]
      ],
      'contact' => [
        'name'  => _('Contact'),
        'attrs' => [
          new SetAttribute(
            new PhoneNumberAttribute(
              _('Other phone numbers'), _('supannAutreTelephone - Other phone numbers for this user'),
              'supannAutreTelephone', FALSE
            )
          ),
          new SetAttribute(
            new CompositeAttribute(
              _('supannTelephonePrive - Private phone numbers for this user'),
              'supannTelephonePrive',
              [
                new SelectAttribute(
                  '', _('Phone number type'),
                  'supannTelephonePrive_prefix', TRUE,
                  ['MOBPERSO','FIXEPERSO','MOBPARENTS','FIXEPARENTS','MOBPRO','FIXEPRO','SECOURS'],
                  '',
                  [_('Personal mobile'),_('Personal landline'),_('Legal representative mobile'),_('Legal representative landline'),_('Professional mobile'),_('Professional landline'),_('Secours')]
                ),
                new PhoneNumberAttribute(
                  '', _('Phone number'),
                  'supannTelephonePrive_number', TRUE
                ),
              ],
              '/^{(.+)}(.*)$/',
              '{%s}%s',
              '',
              _('Private phone numbers')
            )
          ),
          new SetAttribute(
            new MailAttribute(
              _('Other email addresses'), _('supannAutreMail - Other email addresses for this user. Each must be unique'),
              'supannAutreMail', FALSE
            )
          ),
          new SetAttribute(
            new MailAttribute(
              _('Personal email addresses'), _('supannMailPerso - Personal email addresses for this user'),
              'supannMailPerso', FALSE
            )
          ),
          new SetAttribute(
            new CompositeAttribute(
              _('supannMailPrive - Private email addresses for this user'),
              'supannMailPrive',
              [
                new SelectAttribute(
                  '', _('Email address type'),
                  'supannMailPrive_prefix', TRUE,
                  ['PERSO','PARENTS','PRO','SECOURS'],
                  '',
                  [_('Personal'),_('Legal representative'),_('Professional'),_('Secours')]
                ),
                new MailAttribute(
                  '', _('Email address'),
                  'supannMailPrive_email', TRUE
                ),
              ],
              '/^{(.+)}(.*)$/',
              '{%s}%s',
              '',
              _('Private email addresses')
            )
          ),
          new SetAttribute(
            new CompositeAttribute(
              _('supannAdressePostalePrivee - Private addresses for this user'),
              'supannAdressePostalePrivee',
              [
                new SelectAttribute(
                  '', _('Address type'),
                  'supannAdressePostalePrivee_prefix', TRUE,
                  ['PERSO','PARENTS','PRO','TEMP'],
                  '',
                  [_('Personal'),_('Legal representative'),_('Professional'),_('Temporary')]
                ),
                new PostalAddressAttribute(
                  '', _('Address'),
                  'supannAdressePostalePrivee_address', TRUE
                ),
              ],
              '/^{(.+)}(.*)$/',
              '{%s}%s',
              '',
              _('Private addresses')
            )
          ),
          new BooleanAttribute(
            _('Red list'), _('supannListeRouge - Should this person be on the red list'),
            'supannListeRouge', TRUE
          )
        ]
      ],
      'affectation' => [
        'name'  => _('Assignment'),
        'attrs' => [
          new SelectAttribute(
            _('Primary assignment'), _('supannEntiteAffectationPrincipale - Main assignment of the person'),
            'supannEntiteAffectationPrincipale', FALSE
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Assignments'), _('supannEntiteAffectation - Represents assignments of the person in an institution, a component, service, etc.'),
              'supannEntiteAffectation', FALSE
            )
          ),
          new SetAttribute(
            new SupannPrefixedSelectAttribute(
              _('Entity types'), _('supannTypeEntiteAffectation - Types of the entities this person is assigned to'),
              'supannTypeEntiteAffectation', FALSE, 'entite'
            )
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
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Establishment'), _('supannEtablissement - Institution or unit of administrative attachment of the person'),
              'supannEtablissement', FALSE
            )
          ),
          new ObjectsAttribute(
            _('Parrain'), _('supannParrainDN - Person or structure responsible for this user'),
            'supannParrainDN', FALSE,
            ['user','entite','ogroup','group']
          )
        ]
      ],
      'student' => [
        'name'  => _('Student profile'),
        'attrs' => [
          new StringAttribute(
            _('INE code'), _('supannCodeINE - INE code of this student'),
            'supannCodeINE', FALSE
          ),
          new StringAttribute(
            _('Student ID'), _('supannEtuId - Scolarity id'),
            'supannEtuId', FALSE
          ),
        ]
      ],
      'student2' => [
        'name'      => _('Student registrations'),
        'class'     => ['fullwidth'],
        'template'  => get_template_path('student_subscription.tpl', TRUE, dirname(__FILE__)),
        'attrs'     => [
          /* These attributes are handled by the SupannEtuInscriptionAttribute */
          new HiddenAttribute('supannEtuAnneeInscription'),
          new HiddenAttribute('supannEtuRegimeInscription'),
          new HiddenAttribute('supannEtuSecteurDisciplinaire'),
          new HiddenAttribute('supannEtuTypeDiplome'),
          new HiddenAttribute('supannEtuCursusAnnee'),
          new HiddenAttribute('supannEtuDiplome'),
          new HiddenAttribute('supannEtuEtape'),
          new HiddenAttribute('supannEtuElementPedagogique'),
          new SupannOrderedArrayAttribute(
            new SupannEtuInscriptionAttribute(
              _('supannEtuInscription - Registrations for this student'),
              'supannEtuInscription'
            ),
            // no order
            FALSE,
            [],
            // no edit button
            FALSE
          )
        ]
      ],
      'personnal' => [
        'name'  => _('Personal profile'),
        'attrs' => [
          new StringAttribute(
            _('Personal ID'), _('supannEmpId - Employee identifier'),
            'supannEmpId', FALSE
          ),
          new SupannPrefixedSelectAttribute(
            _('Personal corps'), _('supannEmpCorps'),
            'supannEmpCorps', FALSE, 'corps'
          ),
          new SetAttribute(
            new SupannPrefixedSelectAttribute(
              _('Activity'), _('supannActivite - Category of profession'),
              'supannActivite', FALSE, 'activite'
            )
          )
        ]
      ],
      'personnal2' => [
        'name'      => _('Roles'),
        'class'     => ['fullwidth'],
        'attrs'     => [
          /* These attributes are handled by the SupannOrderedArrayAttribute */
          new HiddenAttribute('supannRoleGenerique'),
          new SupannOrderedArrayAttribute(
            new SupannCompositeAttribute(
              _('supannRoleEntite'),
              'supannRoleEntite',
              [
                new SupannPrefixedSelectAttribute(
                  _('Generic role'), _('supannRoleGenerique - Generic role of the person in the facility'),
                  'supannRoleGenerique_role', TRUE, 'role'
                ),
                new SelectAttribute(
                  _('Entity type'), _('supannTypeEntiteAffectation - type of the assigned entity'),
                  'supannTypeEntiteAffectation_type', TRUE
                ),
                new SelectAttribute(
                  _('Entity'), _('supannEntiteAffectation - Represents assignments of the person in an institution, a component, service, etc.'),
                  'supannEntiteAffectation_code', FALSE
                ),
              ],
              '',
              // no label
              ''
            ),
            // no order
            FALSE,
            [],
            // no edit button
            FALSE
          )
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['supannRefId']->setUnique('whole');
    $this->attributesAccess['eduPersonPrincipalName']->setUnique('whole');
    $this->attributesAccess['supannAutreMail']->setUnique('whole');

    /* list of entity stored in LDAP tree */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=supannEntite)', ['supannCodeEntite','supannTypeEntite','ou','o','supannEtablissement','description']);

    $code_entites   = [];
    $label_entites  = [];
    $code_etablissements  = [];
    $label_etablissements = [];
    $this->entitesCache   = [];
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['supannCodeEntite'][0])) {
        $code_entites[] = $attrs['supannCodeEntite'][0];
        if (isset($attrs['description'][0])) {
          $description = $attrs['description'][0];
          if (mb_strlen($description, 'UTF-8') > 30) {
            $description = mb_substr($description, 0, 27, 'UTF-8').'...';
          }
          $description = ' ('.$description.')';
        } else {
          $description = '';
        }
        if (isset($attrs['ou'][0])) {
          $label_entites[] = $attrs['ou'][0].$description;
        } else {
          $label_entites[] = $attrs['o'][0].$description;
        }
        if (isset($attrs['supannEtablissement'])) {
          for ($i = 0; $i < $attrs['supannEtablissement']['count']; $i++) {
            $code_etablissements[]  = $attrs['supannEtablissement'][$i];
            $label_etablissements[] = $attrs['o'][0].' ('.$attrs['supannEtablissement'][$i].')';
          }
        }
        $this->entitesCache[$attrs['supannCodeEntite'][0]] = $attrs;
      }
    }
    array_multisort($label_entites,         $code_entites);
    array_multisort($label_etablissements,  $code_etablissements);

    $this->attributesAccess['supannEntiteAffectation']->attribute->setChoices($code_entites, $label_entites);
    $this->attributesAccess['supannEtuInscription']->attribute->attributes[6]->setChoices($code_entites, $label_entites);
    $this->attributesAccess['supannEtablissement']->attribute->setChoices($code_etablissements, $label_etablissements);
    $this->attributesAccess['supannEtuInscription']->attribute->attributes[0]->setChoices($code_etablissements, $label_etablissements);
    $this->supannInit();
  }

  protected function supannInit ()
  {
    list ($codes, $labels) = supann::get_choices_for('affiliation');
    $this->attributesAccess['eduPersonAffiliation']->attribute->setChoices($codes, $labels);

    $this->attributesAccess['supannEtuInscription']->setHeaders([
      _('Establishment'),
      _('Year'),
      _('Registration type'),
      _('Sector'),
      _('Diploma type'),
      _('Curriculum'),
      _('Entity'),
      _('Diploma'),
      _('Step'),
      _('Element'),
      ''
    ]);
    $this->attributesAccess['supannEtuInscription']->setLinearRendering(FALSE);

    $this->updateFieldsChoicesAndValues();
  }

  /* Update choices of fields which depends on other fields values */
  protected function updateFieldsChoicesAndValues ()
  {
    global $config;

    /* Values */
    $supannEtablissement      = $this->supannEtablissement;
    $supannEntiteAffectation  = $this->supannEntiteAffectation;
    foreach ($this->supannEtuInscription as $inscription) {
      $values = $this->attributesAccess['supannEtuInscription']->attribute->readValues($inscription);
      $supannEtablissement[]      = $values[0];
      $supannEntiteAffectation[]  = $values[6];
    }
    $this->supannEtablissement      = array_values(array_unique($supannEtablissement));
    $this->supannEntiteAffectation  = array_values(array_unique($supannEntiteAffectation));

    $supannTypeEntiteAffectation = $this->supannTypeEntiteAffectation;
    foreach ($this->supannEntiteAffectation as $code) {
      if (isset($this->entitesCache[$code]['supannTypeEntite'][0])) {
        $supannTypeEntiteAffectation[] = $this->entitesCache[$code]['supannTypeEntite'][0];
      }
    }
    $this->supannTypeEntiteAffectation = array_values(array_unique($supannTypeEntiteAffectation));

    /* Choices */
    $code_ent   = $this->attributesAccess['supannEntiteAffectation']->getValue();
    $label_ent  = $this->attributesAccess['supannEntiteAffectation']->getDisplayValues();
    $this->attributesAccess['supannEntiteAffectationPrincipale']->setChoices(
      $code_ent, $label_ent
    );
    // supannEntiteAffectation
    $this->attributesAccess['supannRoleEntite']->attribute->attributes[2]->setChoices($code_ent, $label_ent);
    $code_tent   = $this->attributesAccess['supannTypeEntiteAffectation']->getValue();
    $label_tent  = $this->attributesAccess['supannTypeEntiteAffectation']->getDisplayValues();
    // supannTypeEntiteAffectation
    $this->attributesAccess['supannRoleEntite']->attribute->attributes[1]->setChoices($code_tent, $label_tent);

    $this->attributesAccess['eduPersonPrimaryAffiliation']->setChoices(
      $this->attributesAccess['eduPersonAffiliation']->getValue(),
      $this->attributesAccess['eduPersonAffiliation']->getDisplayValues()
    );
  }

  function check (): array
  {
    $message = parent::check();

    $affiliations = $this->eduPersonAffiliation;
    if (in_array('member', $affiliations) && in_array('affiliate', $affiliations)) {
      $message[] = _('"member" and "affiliate" values are incompatible for eduPersonAffiliation. Please remove one of them.');
    }

    return $message;
  }

  function save_object ()
  {
    parent::save_object();
    $this->updateFieldsChoicesAndValues();
  }

  function adapt_from_template (array $attrs, array $skip = [])
  {
    parent::adapt_from_template($attrs, $skip);
    $this->updateFieldsChoicesAndValues();
  }

  protected function prepare_save (): array
  {
    $this->attributesAccess['supannEtuInscription']->supannPrepareSave();
    $this->attributesAccess['supannRoleEntite']->supannPrepareSave();
    return parent::prepare_save();
  }
}
