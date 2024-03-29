<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019  FusionDirectory

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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class entite extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Entity'),
      'plDescription' => _('SupAnn Entity Settings'),
      'plObjectClass' => ['supannEntite','organizationalUnit'],
      'plObjectType'  => ['entite' => [
        'name'        => _('SupAnn Entity'),
        'ou'          => get_ou('supannStructuresRDN'),
        'mainAttr'    => 'supannCodeEntite',
        'icon'        => 'geticon.php?context=applications&icon=supann-entite&size=16',
        'nameAttr'    => 'ou',
      ]],
      'plSearchAttrs' => ['description', 'supannCodeEntiteParent'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE)
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Entity'),
        'attrs' => [
          new StringAttribute(
            _('Name'), _('The name to write in the ou attribute for this entity'),
            'ou', TRUE
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of this entity'),
            'description', FALSE
          ),
        ]
      ],
      'admin' => [
        'name'  => _('Administrative information'),
        'attrs' => [
          new PhoneNumberAttribute(
            _('Telephone'), _('Phone number of this entity'),
            'telephoneNumber', FALSE
          ),
          new PhoneNumberAttribute(
            _('Fax'), _('Fax number of this entity'),
            'facsimileTelephoneNumber', FALSE
          ),
          new PostalAddressAttribute(
            _('Postal address'), _('Postal address of this entity'),
            'postalAddress', FALSE
          ),
        ]
      ],
      'supann' => [
        'name'  => _('SupAnn information'),
        'attrs' => [
          new SupannPrefixedSelectAttribute(
            _('Entity type'), _('The SupAnn type that best fits this entity'),
            'supannTypeEntite', FALSE, 'entite'
          ),
          new StringAttribute(
            _('Entity code'), _('The SupAnn code of this entity'),
            'supannCodeEntite', TRUE,
            '', '', '|^[a-zA-Z0-9_/-]*$|'
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Parent entities'), _('The parent entities of this entity'),
              'supannCodeEntiteParent', FALSE
            )
          ),
          new SetAttribute(
            new StringAttribute(
              _('Reference IDs'), _('supannRefId - IDs/links for this entity on other systems'),
              'supannRefId', FALSE,
              '', '',
               // Validation regexp: it must have a prefix
              '/^{[^}]+}.+$/'
            )
          ),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    list($code_entites, $label_entites) = static::listPossibleParentEntities();

    $this->attributesAccess['supannCodeEntiteParent']->attribute->setChoices($code_entites, $label_entites);

    $this->attributesAccess['supannCodeEntite']->setUnique('whole', '(objectClass=supannEntite)');
    $this->attributesAccess['supannRefId']->setUnique('whole', '(objectClass=supannEntite)');
  }

  static public function listPossibleParentEntities (): array
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=supannEntite)', ['supannCodeEntite','ou','o','description']);

    $code_entites   = [''];
    $label_entites  = [''];
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
      }
    }

    return [$code_entites, $label_entites];
  }
}
