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
        'name'  => _('SupAnn top'),
        'attrs' => [
          new StringAttribute(
            _('SupAnn Structures RDN'), _('Branch in which SupAnn structures will be stored'),
            'fdSupannStructuresRDN', TRUE,
            'ou=structures'
          ),
          new StringAttribute(
            _('SupAnn Lists RDN'), _('Branch in which SupAnn Lists will be stored'),
            'fdSupannListsRDN', TRUE,
            'ou=supannlists'
          ),
          new StringAttribute(
            _('SupAnn Objects RDN'), _('Branch in which SupAnn Objects will be stored'),
            'fdSupannObjectsRDN', TRUE,
            'ou=supannobjects'
          ),
        ],
      ],
      'global' => [
        'name'  => _('SupAnn main'),
        'attrs' => [
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
          new StringAttribute(
            _('SupAnn Prefix RDN'), _('Branch in which SupAnn Prefix Objects will be stored'),
            'fdSupannPrefixRDN', TRUE,
            'ou=prefix,ou=supannlists'
          ),
          new StringAttribute(
            _('SupAnn Entite RDN'), _('Branch in which SupAnn Entite Objects will be stored'),
            'fdSupannEntiteRDN', TRUE,
            'ou=entite,ou=supannlists'
          ),
        ],
      ],
      'employee' => [
        'name'  => _('SupAnn Employee'),
        'attrs' => [
          new StringAttribute(
            _('SupAnn Activite RDN'), _('Branch in which SupAnn Activite Objects will be stored'),
            'fdSupannActiviteRDN', TRUE,
            'ou=activite,ou=supannlists'
          ),
          new StringAttribute(
            _('SupAnn Corps RDN'), _('Branch in which SupAnn Corps Objects will be stored'),
            'fdSupannCorpsRDN', TRUE,
            'ou=corps,ou=supannlists'
          ),
          new StringAttribute(
            _('SupAnn Role RDN'), _('Branch in which SupAnn Role Objects will be stored'),
            'fdSupannRoleRDN', TRUE,
            'ou=role,ou=supannlists'
          ),
         ],
       ],
      'etudiant' => [
        'name'  => _('SupAnn Etudiant'),
        'attrs' => [
          new StringAttribute(
            _('SupAnn Discipline RDN'), _('Branch in which SupAnn Discipline Objects will be stored'),
            'fdSupannDisciplineRDN', TRUE,
            'ou=discipline,ou=supannlists'
          ),
          new StringAttribute(
            _('SupAnn EtuRegimeInscription RDN'), _('Branch in which SupAnn EtuRegimeInscription Objects will be stored'),
            'fdSupannEtuRegimeInscriptionRDN', TRUE,
            'ou=eturegimeinscription,ou=supannlists'
          ),
          new StringAttribute(
            _('SupAnn TypeDiplome RDN'), _('Branch in which SupAnn TypeDiplome Objects will be stored'),
            'fdSupannTypeDiplomeRDN', TRUE,
            'ou=typediplome,ou=supannlists'
          ),
          new StringAttribute(
            _('SupAnn Diplome RDN'), _('Branch in which SupAnn Diplome Objects will be stored'),
            'fdSupannDiplomeRDN', TRUE,
            'ou=diplome,ou=supannlists'
          ),
        ]
      ],
      'resources' => [
        'name'  => _('SupAnn Ressources'),
        'attrs' => [
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
         ],
       ],
      'prive' => [
        'name'  => _('SupAnn Prive'),
        'attrs' => [
          new StringAttribute(
              _('SupAnn Prive RDN'), _('Branch in which SupAnn Prive Objects will be stored'),
              'fdSupannPriveRDN', TRUE,
              'ou=prive,ou=supannobjects'
          ),
          new StringAttribute(
              _('SupAnn Prive Mail RDN'), _('Branch in which SupAnn Prive Mail Objects will be stored'),
              'fdSupannPriveMailRDN', TRUE,
              'ou=mail,ou=prive,ou=supannobjects'
          ),
          new StringAttribute(
              _('SupAnn Prive Telephone RDN'), _('Branch in which SupAnn Prive Telephone Objects will be stored'),
              'fdSupannPriveTelephoneRDN', TRUE,
              'ou=telephone,ou=prive,ou=supannobjects'
          ),
          new StringAttribute(
              _('SupAnn Prive AdressePostale RDN'), _('Branch in which SupAnn Prive Adresse Objects will be stored'),
              'fdSupannPriveAdressePostaleRDN', TRUE,
              'ou=adressePostale,ou=prive,ou=supannobjects'
          ),
         ],
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
          new StringAttribute(
            _('Amount of days for locking manual locking'), _('Amount of days for locking manual locking'),
            'fdSupannUnlockingDays', TRUE,
            '365'
          ),
        ],
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
          new BooleanAttribute(
            _('Supann code Population Mode'), _('Enable Supann code Population Mode'),
            'fdSupannCodePopulationMode', TRUE,
            TRUE
          ),
        ],
      ],
    ];
  }
}
