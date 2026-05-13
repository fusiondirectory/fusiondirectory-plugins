<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2010-2012 Antoine Gallavardin
  Copyright (C) 2013-2025 FusionDirectory

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

class supannAccountStudent extends simplePlugin
{
  protected $displayHeader = TRUE;

  protected $entitesCache = [];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn Student'),
      'plDescription' => _('SupAnn Student Account management'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 14,
      'plObjectClass' => ['fdSupannStudent'],
      'plObjectType'  => ['user'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
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
          )
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
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

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

    $this->attributesAccess['supannEtuInscription']->attribute->attributes[6]->setChoices($code_entites, $label_entites);
    $this->attributesAccess['supannEtuInscription']->attribute->attributes[0]->setChoices($code_etablissements, $label_etablissements);
    $this->supannStudentInit();
  }

  protected function supannStudentInit ()
  {
    global $config;

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
  }

  function adapt_from_template (array $attrs, array $skip = [])
  {
    parent::adapt_from_template($attrs, $skip);
  }

  protected function prepare_save (): array
  {
    $this->attributesAccess['supannEtuInscription']->supannPrepareSave();
    return parent::prepare_save();
  }
}
