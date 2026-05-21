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

class supannAccountEmployee extends simplePlugin
{
  protected $displayHeader = TRUE;

  protected $entitesCache = [];

  static function plInfo (): array
  {
    global $config;
    if ($config->get_cfg_value('SupannShowEmployee', 'TRUE') !== 'TRUE') {
      return [];
    }

    return [
      'plShortName'   => _('SupAnn Employee'),
      'plDescription' => _('SupAnn Employee Account Management'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 14,
      'plObjectClass' => ['fdSupannEmployee'],
      'plObjectType'  => ['user'],
      'plDepends' => ['supannAccount'],

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
      'roles' => [
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

  /* Update choices of fields which depends on other fields values */
  protected function updateFieldsChoicesAndValues ()
  {
    global $config;

    /* Values for  entite affectation*/
    if (($supannTab = $this->parent->getTabOrServiceObject('supannAccount')) !== FALSE) {
      $supannEntiteAffectation  = $this->parent->getTabOrServiceObject('supannAccount')->attributesAccess['supannEntiteAffectation'];
      $code_ent = $supannEntiteAffectation->getValue();
      $label_ent  = $supannEntiteAffectation->getDisplayValues();

      $supannTypeEntiteAffectation = $this->parent->getTabOrServiceObject('supannAccount')->attributesAccess['supannTypeEntiteAffectation'];
      $code_tent = $supannTypeEntiteAffectation->getValue();
      $label_tent  = $supannTypeEntiteAffectation->getDisplayValues();

      $this->attributesAccess['supannRoleEntite']->attribute->attributes[2]->setChoices($code_ent, $label_ent);
      $this->attributesAccess['supannRoleEntite']->attribute->attributes[1]->setChoices($code_tent, $label_tent);
    }
  }

  public function update (): bool
  {
    parent::update();
    $this->updateFieldsChoicesAndValues();
    return TRUE;
  }

  function adapt_from_template (array $attrs, array $skip = [])
  {
    parent::adapt_from_template($attrs, $skip);
    $this->updateFieldsChoicesAndValues();
  }

  protected function prepare_save (): array
  {
    $this->attributesAccess['supannRoleEntite']->supannPrepareSave();
    return parent::prepare_save();
  }
}
