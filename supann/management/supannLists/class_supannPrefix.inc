<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2026  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannPrefix extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn Prefix'),
      'plDescription' => _('SupAnn Prefix'),
      'plObjectClass' => ['fdSupannPrefix'],
      'plFilter'      => '(objectClass=fdSupannPrefix)',
      'plObjectType'  => [
        'supannPrefix' => [
          'name'         => _('SupAnn Prefix'),
          'description'  => _('SupAnn Prefix'),
          'ou'           => get_ou('SupannPrefixRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannName',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannName'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Prefix'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannPrefixRDN')),
          new StringAttribute(
            _('Prefix Name'), _('The name of this Supann Prefix'),
            'fdSupannName', TRUE
          ),
          new StringAttribute(
            _('Prefix Label'), _('The label of this Supann Prefix'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('Prefix Description'), _('The description explaining this Supann Prefix'),
            'description', FALSE
          ),
        ],
      ],
      'lists' => [
        'name'  => _('Available Lists'),
        'attrs' => [
          new ObjectsAttribute(
            _('Available Lists'),
            _('Select one or more available Lists for this Prefix'),
            'fdSupannList',
            FALSE,
            [
              'supannActivite'
            ],
          ),
        ]
      ],
      'type' => [
        'name'  => _('Type Lists'),
        'attrs' => [
          new SelectAttribute(
            _('Type Lists'),
            _('Select one type for the Prefix'),
            'fdSupannTypeObjectclass',
            TRUE,
            [
              'Aucun',
              'Activite',
              'Corps',
              'Diplome',
              'Discipline',
              'Entite',
              'Etablissement',
              'EtuRegimeInscription',
              'Role',
              'TypeDiplome'
            ],
            NULL,
          ),
        ]
      ],
    ];
  }

  public function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    foreach ($this->attrs as $attr => $value) {
      if ($attr == 'objectClass') {
        // Get the objectClass that is not fdSupannPrefix
        $oc = array_diff($value, ['fdSupannPrefix'])[0];
        break;
      }
    }

    // Set the value of oc as choice for fdSupannTypeObjectclass without fdSupann
    if (!empty($oc)) {
      $this->attributesAccess['fdSupannTypeObjectclass']->setValue(
        str_replace("fdSupann", "", $oc)
      );
    }
  }

  public function mergeObjectClasses ($oc) : array
  {
    $oc = ['fdSupann' . $this->attributesAccess['fdSupannTypeObjectclass']->getValue()];
    return parent::mergeObjectClasses($oc);
  }

  public function prepare_save (): array
  {
    // Get the value from fdSupannTypeObjectclass
    $oc = $this->attributesAccess['fdSupannTypeObjectclass']->getValue();
    
    if ($oc == 'Aucun') {
      return [_('Select a Type for the Supann prefix')];
    }
    
    return parent::prepare_save();
  }

  public function save () : array
  {
    // Disable save in LDAP for fdSupannTypeObjectclass
    $this->attributesAccess['fdSupannTypeObjectclass']->setInLdap(FALSE);

    // Save and return errors
    $errors = parent::save();
    return $errors;
  }
}
