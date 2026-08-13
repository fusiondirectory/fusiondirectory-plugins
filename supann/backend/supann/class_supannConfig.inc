<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2026 FusionDirectory

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

  protected $populationCodeMaster;

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
    return [
      'rdns' => [
        'name'  => _('LDAP tree for supann'),
        'attrs' => [
          new StringAttribute(
            _('SupAnn Structures RDN'), _('Branch in which SupAnn structures will be stored'),
            'fdSupannStructuresRDN', TRUE,
            'ou=structures'
          ),
          new StringAttribute(
            _('SupAnn Objects RDN'), _('Branch in which SupAnn Objects will be stored'),
            'fdSupannObjectsRDN', TRUE,
            'ou=supannobjects'
          ),
          new StringAttribute(
            _('SupAnn Resources RDN'), _('Branch in which SupAnn Ressource Objects will be stored'),
            'fdSupannRessourceRDN', TRUE,
            'ou=ressources,ou=supannobjects'
          ),
          new StringAttribute(
            _('SupAnn States RDN'), _('Branch in which SupAnn State Objects will be stored'),
            'fdSupannStateRDN', TRUE,
            'ou=states,ou=supannobjects'
          ),
          new StringAttribute(
            _('SupAnn Sub States RDN'), _('Branch in which SupAnn SubState Objects will be stored'),
            'fdSupannSubStateRDN', TRUE,
            'ou=substates,ou=supannobjects'
          ),
          new StringAttribute(
            _('SupAnn Civility'), _('Branch in which SupAnn Civility Objects will be stored'),
            'fdSupannCiviliteRDN', TRUE,
            'ou=civilite,ou=supannobjects'
          ),
          new StringAttribute(
            _('SupAnn Consent RDN'), _('Branch in which SupAnn Consent Objects will be stored'),
            'fdSupannConsentRDN', TRUE,
            'ou=consent,ou=supannobjects'
          ),
          new StringAttribute(
            _('SupAnn Population Code RDN'), _('Branch in which SupAnn Population Code Objects will be stored'),
            'fdSupannPopulationCodeRDN', TRUE,
            'ou=populationcodes,ou=supannobjects'
          ),
        ]
      ],

      'main' => [
        'name'  => _('SupAnn'),
        'class' => ['fullwidth'],
        'attrs' => [
          new BooleanAttribute(
            _('SupannRessourceEtatDate integrity'), _('Perform supannRessourceEtatDate integrity'),
            'fdSupannRessourceEtatDateIntegrity', TRUE,
            TRUE
          ),
          new BooleanAttribute(
            _('SupAnn mail for recovery'), _('Allow the use of mail addresses from the personal mail address field from SupAnn account for password recovery'),
            'fdSupannPasswordRecovery', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('Show mailForwardingAddress field'), _('Show the field mailForwardingAddress as an editable field'),
            'fdSupannShowMailForwardingAddress', FALSE,
            FALSE
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Custom types and their labels'),
              'fdSupannMailPriveTypes',
              [
                new StringAttribute(
                  _('Type'), _('String that will be stored between {} in supannMailPrive'),
                  'fdSupannMailPriveTypes_type', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this type'),
                  'fdSupannMailPriveTypes_label', TRUE
                ),
              ],
              ':',
              '',
              _('Custom supannMailPrive types')
            ),
            FALSE,
            [],
            TRUE
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Custom types and their labels'),
              'fdSupannTelephonePriveTypes',
              [
                new StringAttribute(
                  _('Type'), _('String that will be stored between {} in supannTelephonePrive'),
                  'fdSupannTelephonePriveTypes_type', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this type'),
                  'fdSupannTelephonePriveTypes_label', TRUE
                ),
              ],
              ':',
              '',
              _('Custom supannTelephonePrive types')
            ),
            FALSE,
            [],
            TRUE
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Custom types and their labels'),
              'fdSupannAdressePostalePriveeTypes',
              [
                new StringAttribute(
                  _('Type'), _('String that will be stored between {} in supannAdressePostalePrivee'),
                  'fdSupannAdressePostalePriveeTypes_type', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this type'),
                  'fdSupannAdressePostalePriveeTypes_label', TRUE
                ),
              ],
              ':',
              '',
              _('Custom supannAdressePostalePrivee types')
            ),
            FALSE,
            [],
            TRUE
          ),
          new StringAttribute(
            _('Amount of days for locking manual locking'), _('Amount of days for locking manual locking'),
            'fdSupannUnlockingDays', TRUE,
            '365'
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

      'features' => [
        'name'  => _('Features'),
        'attrs' => [
          new BooleanAttribute(
            _('Student Profile'), _('Show Student tab'),
            'fdSupannShowStudent', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('Employee Profile'), _('Show Employee tab'),
            'fdSupannShowEmployee', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('SupAnn status'), _('Show SupAnn status tab'),
            'fdSupannShowAccountStatus', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('Multiservice card'), _('Show Multiservice card tab'),
            'fdSupannShowCMS', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('France Connect'), _('Show France Connect tab'),
            'fdSupannShowFranceConnect', FALSE,
            TRUE
          ),
        ],
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
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
    $ldap->search('(&(fdSupannPluginConf))');

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
      '{SUPANN}P'    => _('{SUPANN}P'),
      '{SUPANN}PX'    => _('{SUPANN}PX'),
      '{SUPANN}PXE'    => _('{SUPANN}PXE'),
      '{SUPANN}PXL'    => _('{SUPANN}PXL'),
      '{SUPANN}PXR'    => _('{SUPANN}PXR'),
      '{SUPANN}PXSP'    => _('{SUPANN}PXSP'),
      '{SUPANN}PXU'    => _('{SUPANN}PXU'),
      '{SUPANN}R'    => _('{SUPANN}R'),
      '{SUPANN}RG'   => _('{SUPANN}RG'),
      '{SUPANN}RGI' => _('{SUPANN}RGI'),
      '{SUPANN}RGIE' => _('{SUPANN}RGIE'),
      '{SUPANN}RGIS' => _('{SUPANN}RGIS'),
      '{SUPANN}RGN' => _('{SUPANN}RGN'),
      '{SUPANN}RGNC' => _('{SUPANN}RGNC'),
      '{SUPANN}RGNCC' => _('{SUPANN}RGNCC'),
      '{SUPANN}RGNCD' => _('{SUPANN}RGNCD'),
      '{SUPANN}RGNE' => _('{SUPANN}RGNE'),
      '{SUPANN}RGNF' => _('{SUPANN}RGNF'),
      '{SUPANN}RGNFA' => _('{SUPANN}RGNFA'),
      '{SUPANN}RGNFC' => _('{SUPANN}RGNFC'),
      '{SUPANN}RGNFD' => _('{SUPANN}RGNFD'),
      '{SUPANN}RGNS' => _('{SUPANN}RGNS'),
      '{SUPANN}RGNSP' => _('{SUPANN}RGNSP'),
      '{SUPANN}RGP'  => _('{SUPANN}RGP'),
      '{SUPANN}RGPE'  => _('{SUPANN}RGPE'),
      '{SUPANN}RGPET'  => _('{SUPANN}RGPET'),
      '{SUPANN}RGPF' => _('{SUPANN}RGPF'),
      '{SUPANN}RGPFT' => _('{SUPANN}RGPFT'),
      '{SUPANN}RGPST' => _('{SUPANN}RGPST'),
      '{SUPANN}RHTC' => _('{SUPANN}RHTC'),
      '{SUPANN}RHTCE' => _('{SUPANN}RHTCE'),
      '{SUPANN}RHJCF' => _('{SUPANN}RHJCF'),
      '{SUPANN}RHJSG' => _('{SUPANN}RHJSG'),
      '{SUPANN}RHLE' => _('{SUPANN}RHLE'),
      '{SUPANN}RHLS' => _('{SUPANN}RHLS'),
      '{SUPANN}RHMF' => _('{SUPANN}RHMF'),
      '{SUPANN}RHTSO' => _('{SUPANN}RHTSO'),
      '{SUPANN}TER' => _('{SUPANN}TER'),
    ];

    //Getting values from LDAP already registered and add the default ones, making array unique if already defined.
    $this->populationCodeMaster += $this->attributesAccess['fdMainPopulationCodeConf']->getValue();
    $this->populationCodeMaster = array_unique($this->populationCodeMaster);

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

    return TRUE;
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
