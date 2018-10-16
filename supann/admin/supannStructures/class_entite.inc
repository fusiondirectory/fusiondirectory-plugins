<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2018  FusionDirectory

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
  var $objectclasses = array('top','supannEntite','organizationalUnit');

  var $mainTab = TRUE;

  /* Return plugin information */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Entity'),
      'plDescription' => _('SupAnn Entity Settings'),
      'plObjectType'  => array('entite' => array(
        'name'        => _('SupAnn Entity'),
        'filter'      => '(&(objectClass=supannEntite)(objectClass=organizationalUnit))',
        'ou'          => get_ou('supannStructuresRDN'),
        'mainAttr'    => 'supannCodeEntite',
        'icon'        => 'geticon.php?context=applications&icon=supann-entite&size=16',
        'nameAttr'    => 'ou',
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Entity'),
        'attrs' => array(
          new StringAttribute(
            _('Name'), _('The name to write in the ou attribute for this entity'),
            'ou', TRUE
          )
        )
      ),
      'admin' => array(
        'name'  => _('Administrative information'),
        'attrs' => array(
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
          new TextAreaAttribute(
            _('Description'), _('Short description of this entity'),
            'description', FALSE
          ),
        )
      ),
      'supann' => array(
        'name'  => _('SupAnn information'),
        'attrs' => array(
          new SupannPrefixedSelectAttribute(
            _('Entity type'), _('The SupAnn type that best fits this entity'),
            'supannTypeEntite', FALSE, 'entite'
          ),
          new StringAttribute(
            _('Entity code'), _('The SupAnn code of this entity'),
            'supannCodeEntite', TRUE,
            '', '', '/^[a-z0-9_-]*$/i'
          ),
          new SelectAttribute(
            _('Parent entity'), _('The parent entity of this entity'),
            'supannCodeEntiteParent', FALSE
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
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=supannEntite)', array('supannCodeEntite','ou','o'));

    $entity_codes   = array('');
    $entity_labels  = array('');
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['supannCodeEntite'][0])) {
        $entity_codes[] = $attrs['supannCodeEntite'][0];
        if (isset($attrs['ou'][0])) {
          $entity_labels[] = $attrs['ou'][0]." (".$attrs['supannCodeEntite'][0].")";
        } else {
          $entity_labels[] = $attrs['o'][0]." (".$attrs['supannCodeEntite'][0].")";
        }
      }
    }
    $this->attributesAccess['supannCodeEntiteParent']->setChoices($entity_codes, $entity_labels);

    $this->attributesAccess['ou']->setUnique(TRUE);
    $this->attributesAccess['supannCodeEntite']->setUnique(TRUE);
  }
}

?>
