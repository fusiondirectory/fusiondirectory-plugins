<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2020  FusionDirectory

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

class etablissement extends simplePlugin
{
  protected static $rootAttributes = [
    'o','description','telephoneNumber','facsimileTelephoneNumber','postalAddress','l',
    'eduOrgHomePageURI','eduOrgLegalName','eduOrgSuperiorURI','eduOrgWhitePagesURI',
    'supannEtablissement'
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Establishment'),
      'plDescription' => _('SupAnn Establishment Settings'),
      'plObjectClass' => ['supannEntite','organization','supannOrg','eduOrg'],
      'plFilter'      => '(&(objectClass=supannEntite)(objectClass=organization))',
      'plObjectType'  => ['etablissement' => [
        'name'        => _('SupAnn Establishment'),
        'ou'          => get_ou('supannStructuresRDN'),
        'mainAttr'    => 'supannCodeEntite',
        'icon'        => 'geticon.php?context=applications&icon=supann-etablissement&size=16',
        'nameAttr'    => 'o',
      ]],
      'plSearchAttrs' => ['description', 'supannEtablissement', 'eduOrgLegalName'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE)
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BooleanAttribute(
            _('Root establishment'), _('Set this establishment as the root one'),
            'set_root'
          ),
          new StringAttribute(
            _('Name'), _('The name to write in the o attribute for this establishment'),
            'o', TRUE
          ),
          new TextAreaAttribute(
            _('Description'), _('A short description of this establishment'),
            'description', FALSE
          ),
        ]
      ],
      'location' => [
        'name'  => _('Location'),
        'attrs' => [
          new PhoneNumberAttribute(
            _('Telephone'), _('Phone number of this establishment'),
            'telephoneNumber', FALSE
          ),
          new PhoneNumberAttribute(
            _('Fax'), _('Fax number of this establishment'),
            'facsimileTelephoneNumber', FALSE
          ),
          new StringAttribute(
            _('Location'), _('Usually the city where this establishment is'),
            'l', FALSE
          ),
          new PostalAddressAttribute(
            _('Address'), _('The postal address of this establishment'),
            'postalAddress', FALSE
          ),
        ]
      ],
      'supann' => [
        'name'  => _('SupAnn properties'),
        'attrs' => [
          new SetAttribute(
            new StringAttribute(
              _('Establishment code'), _('The code of this establishment (must have a prefix between {})'),
              'supannEtablissement', TRUE,
              '', '', '/^{[^}]+}.*$/'
            )
          ),
          new SupannPrefixedSelectAttribute(
            _('Establishment type'), _('The SupAnn type that best fits this Establishment'),
            'supannTypeEntite', FALSE, 'entite'
          ),
          new StringAttribute(
            _('SupAnn code'), _('The SupAnn code for this establishment'),
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
          new StringAttribute(
            _('Legal name'), _('The legal name of this establishment'),
            'eduOrgLegalName', TRUE
          ),
          new URLAttribute(
            _('Home page URI'), _('The URI of this establishment website home page'),
            'eduOrgHomePageURI', FALSE
          ),
          new URLAttribute(
            _('Institution URI'), _('The URI of this establishment institution website'),
            'eduOrgSuperiorURI', FALSE
          ),
          new URLAttribute(
            _('White pages URI'), _('The URI of this establishment white pages'),
            'eduOrgWhitePagesURI', FALSE
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    list($code_entites, $label_entites) = entite::listPossibleParentEntities();

    $this->attributesAccess['supannCodeEntiteParent']->attribute->setChoices($code_entites, $label_entites);

    $this->attributesAccess['o']->setUnique('one');
    $this->attributesAccess['supannCodeEntite']->setUnique('whole', '(objectClass=supannEntite)');
    $this->attributesAccess['supannRefId']->setUnique('whole', '(objectClass=supannEntite)');
    $this->attributesAccess['supannEtablissement']->setUnique('one');
    $this->attributesAccess['eduOrgLegalName']->setUnique('one');

    $rootCode = $this->get_root_code();
    $this->attributesAccess['set_root']->setInLdap(FALSE);
    $this->set_root = (($rootCode !== FALSE) && in_array($rootCode, $this->supannEtablissement));
    $this->attributesAccess['set_root']->setInitialValue($this->set_root);
  }

  protected function shouldSave (): bool
  {
    /* We need to call ldap_save if set_root is or was checked, even if nothing else changed */
    return (parent::shouldSave() || $this->set_root || $this->attributesAccess['set_root']->getInitialValue());
  }

  protected function ldap_remove (): array
  {
    global $config;

    $errors = parent::ldap_remove();
    if (!empty($errors)) {
      return $errors;
    }

    /* If we're the root etablissement, remove information */
    $rootCode = $this->get_root_code();
    if (($rootCode !== FALSE) && in_array($rootCode, $this->supannEtablissement)) {
      /* Remove SupAnn attributes from root node */
      $ldap = $config->get_ldap_link();
      $ldap->cat($config->current['BASE'], array_merge(static::$rootAttributes, ['objectClass']));
      $baseAttrs = $ldap->fetch();
      unset($baseAttrs['objectClass']['count']);
      $rootAttrs = [];
      $rootAttrs['objectClass'] = array_remove_entries(['supannOrg','eduOrg'], $baseAttrs['objectClass']);
      foreach (static::$rootAttributes as $rootAttribute) {
        $rootAttrs[$rootAttribute] = [];
      }
      $rootAttrs['o'] = $baseAttrs['dc'][0];
      $ldap->cd($config->current['BASE']);
      $ldap->modify($rootAttrs);
      if (!$ldap->success()) {
        $errors[] = msgPool::ldaperror($ldap->get_error(), $config->current['BASE'], 0, get_class());
      }
    }

    return $errors;
  }

  function ldap_save (): array
  {
    global $config;

    $errors = parent::ldap_save();

    $ldap = $config->get_ldap_link();

    $wasRoot  = $this->attributesAccess['set_root']->getInitialValue();
    $rootMode = FALSE;
    $rootCode = $this->get_root_code();
    if ($this->set_root) {
      /* Set this etablissement as root one */
      $rootMode = 'add';
    } elseif (($rootCode !== FALSE) && $wasRoot && !$this->set_root) {
      /* We were the root etablissement, we want to delete it */
      $rootMode = 'remove';
    }

    if ($rootMode) {
      $ldap->cat($config->current['BASE'], array_merge(static::$rootAttributes, ['objectClass','dc']));
      $baseAttrs = $ldap->fetch();
      unset($baseAttrs['objectClass']['count']);
      if ($rootMode == 'remove') {
        /* Remove SupAnn attributes from root node */
        $rootAttrs = [];
        $rootAttrs['objectClass'] = array_remove_entries(['supannOrg','eduOrg'], $baseAttrs['objectClass']);
        foreach (static::$rootAttributes as $rootAttribute) {
          $rootAttrs[$rootAttribute] = [];
        }
        $rootAttrs['o'] = $baseAttrs['dc'][0];
      } else {
        /* Add/modify SupAnn attributes of root node */
        $ldap->cat($this->dn, static::$rootAttributes);
        $rootAttrs = $ldap->fetch();
        unset($rootAttrs['count']);
        unset($rootAttrs['dn']);
        foreach ($rootAttrs as $key => $value) {
          if (is_numeric($key)) {
            unset($rootAttrs[$key]);
            continue;
          }
          if (is_array($rootAttrs[$key])) {
            unset($rootAttrs[$key]['count']);
          }
        }
        $rootAttrs['objectClass'] = array_merge_unique($baseAttrs['objectClass'], ['dcObject','organization','supannOrg','eduOrg']);
      }
      $ldap->cd($config->current['BASE']);
      $ldap->modify($rootAttrs);
      if (!$ldap->success()) {
        $errors[] = msgPool::ldaperror($ldap->get_error(), $config->current['BASE'], 0, get_class());
      }
    }

    return $errors;
  }

  function get_root_code ()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=supannOrg)', ['*'], 'base');

    if ($ldap->count() > 1) {
      throw new FusionDirectoryException('There are several establishments at root!');
    } elseif ($ldap->count() > 0) {
      $attr = $ldap->fetch();
      return $attr['supannEtablissement'][0];
    } else {
      return FALSE;
    }
  }
}
