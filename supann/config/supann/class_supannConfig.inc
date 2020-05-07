<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2019 FusionDirectory

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

class supannConfig extends simplePlugin
{
  protected $officialSubstates;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('SupAnn'),
      'plTitle'         => _('SupAnn configuration'),
      'plDescription'   => _('FusionDirectory SupAnn plugin configuration'),
      'plObjectClass'   => ['fdSupannPluginConf'],
      'plPriority'      => 10,
      'plObjectType'    => ['configuration'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    $officialStates = supannAccountStatus::getOfficialSubstates();
    $defaultStates  = [];
    foreach ($officialStates as $state => $substates) {
      foreach (array_keys($substates) as $substate) {
        $defaultStates[] = $state.':'.$substate;
      }
    }
    return [
      'main' => [
        'name'  => _('SupAnn'),
        'class' => ['fullwidth'],
        'attrs' => [
          new StringAttribute(
            _('SupAnn RDN'), _('Branch in which SupAnn structures will be stored'),
            'fdSupannStructuresRDN', TRUE,
            'ou=structures'
          ),
          new BooleanAttribute(
            _('SupAnn mail for recovery'), _('Allow the use of mail addresses from the personal mail address field from SupAnn account for password recovery'),
            'fdSupannPasswordRecovery', FALSE,
            TRUE
          ),
        ],
      ],
      'status' => [
        'name'  => _('Status'),
        'class' => ['fullwidth'],
        'attrs' => [
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Custom resources and their labels'),
              'fdSupannRessourceLabels',
              [
                new StringAttribute(
                  _('Resource'), _('String that will be stored between {} in supannRessourceEtatDate'),
                  'fdSupannRessourceLabels_resource', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this resource'),
                  'fdSupannRessourceLabels_label', TRUE
                ),
              ],
              ':',
              '',
              _('Custom resources')
            ),
            FALSE,
            [],
            TRUE
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Allowed substates for an account'),
              'fdSupannRessourceSubStates',
              [
                new SelectAttribute(
                  _('Status'), _('Active status this substatus is tied to'),
                  'fdSupannRessourceSubStates_status', TRUE,
                  ['A','I','S'], 'A',
                  [_('Active'), _('Inactive'), _('Suspended')]
                ),
                new StringAttribute(
                  _('Substatus'), _('LDAP value for this substatus'),
                  'fdSupannRessourceSubStates_substatus', TRUE
                ),
              ],
              ':',
              '',
              _('Substates')
            ),
            TRUE,
            $defaultStates
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Labels for custom substates'),
              'fdSupannRessourceSubStatesLabels',
              [
                new SelectAttribute(
                  _('Substatus'), _('LDAP value for this substatus'),
                  'fdSupannRessourceSubStatesLabels_substatus', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this substatus'),
                  'fdSupannRessourceSubStatesLabels_label', TRUE
                ),
              ],
              ':',
              '',
              _('Custom labels')
            ),
            FALSE,
            [],
            TRUE
          ),
        ],
      ],
      'cms' => [
        'name'  => _('Multiservice card'),
        'class' => ['fullwidth'],
        'attrs' => [
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Card types and their labels. Please prefix with {ORIGIN} for non standard types.'),
              'fdSupannCMSTypes',
              [
                new StringAttribute(
                  _('Type'), _('Card type'),
                  'fdSupannCMSTypes_type', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this type'),
                  'fdSupannCMSTypes_label', TRUE
                ),
              ],
              ';',
              '',
              _('Card types')
            ),
            FALSE,
            [
              'etudiant;étudiant',
              'personnel;professionnelle',
              'visiteur;visiteur, anonyme',
              'invite;invité, nominative',
              'lecteur;lecteur de bibliothèque',
              'prestataire;prestataire de services',
              'vehicule;véhicule',
              'externe;externe',
            ],
            TRUE
          ),
          new SetAttribute(
            new StringAttribute(
              _('Card sources'), _('Multi service card possible sources, with format system@domain'),
              'fdSupannCMSSources', FALSE,
              '', '',
              '/^.+@.+$/', 'unicampus@univ.example.com'
            )
          ),
          new OrderedArrayAttribute(
            new CompositeAttribute(
              _('Allowed formats for multiservice cards'),
              'fdSupannCMSFormats',
              [
                new StringAttribute(
                  _('Technology'), _('Technology name in uppercase'),
                  'fdSupannCMSFormats_techno', TRUE,
                  '', '',
                  '/^[A-Z]+$/', 'MIFARE'
                ),
                new StringAttribute(
                  _('Encoding'), _('Encoding name in uppercase'),
                  'fdSupannCMSFormats_encoding', TRUE,
                  '', '',
                  '/^[A-Z]+$/', 'XLSB'
                ),
                new StringAttribute(
                  _('Label'), _('Format label'),
                  'fdSupannCMSFormats_label', TRUE,
                  '', '',
                  '', 'Mifare XLSB'
                ),
              ],
              '/^([A-Z]*):([A-Z]*);(.*)$/',
              '%s:%s;%s',
              '',
              _('Card formats')
            ),
            FALSE,
            ['MIFARE:XLSB;Mifare XLSB'],
            TRUE
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Domains for card applications'),
              'fdSupannCMSAppDomains',
              [
                new StringAttribute(
                  _('Option'), _('Attribute option to use, starting with x-'),
                  'fdSupannCMSAppDomains_option', TRUE,
                  '', '',
                  '/^x-.+$/', 'x-biblio-xlsb'
                ),
                new StringAttribute(
                  _('Domain'), _('Domain to use'),
                  'fdSupannCMSAppDomains_domain', TRUE,
                  '', '',
                  '', 'xlsb.biblio.univ.example.com'
                ),
              ],
              ';',
              '',
              _('Card application domains')
            ),
            FALSE,
            [],
            TRUE
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['fdSupannRessourceSubStates']->setSubmitForm('updateFieldsChoices');
    $this->officialSubstates = array_merge(...array_values(array_map('array_keys', supannAccountStatus::getOfficialSubstates())));
    $this->updateFieldsChoices();
  }

  function updateFieldsChoices ()
  {
    $substateChoices = [];
    foreach ($this->fdSupannRessourceSubStates as $line) {
      list($state, $substate) = explode(':', $line, 2);
      if (!in_array($substate, $this->officialSubstates)) {
        $substateChoices[] = $substate;
      }
    }
    $this->attributesAccess['fdSupannRessourceSubStatesLabels']->attribute->attributes[0]->setChoices(
      $substateChoices
    );
  }
}
