<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Entity'),
      'plDescription' => _('SUPANN Entity Settings'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('entite' => array(
        'name'        => _('SUPANN Entity'),
        'filter'      => '(&(objectClass=supannEntite)(objectClass=organizationalUnit))',
        'ou'          => get_ou('supannStructuresRDN'),
        'mainAttr'    => 'supannCodeEntite',
        'icon'        => 'geticon.php?context=applications&icon=supann-entite&size=16',
        'nameAttr'    => 'ou',
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
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
        'name'  => _('Administrative informations'),
        'attrs' => array(
          new StringAttribute(
            _('Telephone'), _('Phone number for this entity'),
            'telephoneNumber', FALSE
          ),
          new StringAttribute(
            _('Fax'), _('Fax number for this entity'),
            'facsimileTelephoneNumber', FALSE
          ),
          new StringAttribute(
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
        'name'  => _('SUPANN informations'),
        'attrs' => array(
          new SelectAttribute(
            _('Entity type'), _('The best SUPANN type that fits this entity'),
            'supannTypeEntite', FALSE
          ),
          new StringAttribute(
            _('Entity code'), _('The SUPANN code of this entity'),
            'supannCodeEntite', TRUE,
            '', '', '/^[a-z0-9_-]*$/'.(strict_uid_mode()?'':'i')
          ),
          new SelectAttribute(
            _('Parent entity'), _('The parent entity of this entity'),
            'supannCodeEntiteParent', FALSE
          ),
          new StringAttribute(
            _('Reference ID'), _('SUPANN reference ID of this entity'),
            'supannRefId', FALSE
          ),
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
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

    /* Generate list of entite */
    list ($entity_codes, $entity_labels) = supann::get_choices_for('entite');
    array_unshift($entity_codes, '');
    array_unshift($entity_labels, '');
    $this->attributesAccess['supannTypeEntite']->setChoices($entity_codes, $entity_labels);

    $this->attributesAccess['ou']->setUnique(TRUE);
    $this->attributesAccess['supannCodeEntite']->setUnique(TRUE);
  }
}

?>
