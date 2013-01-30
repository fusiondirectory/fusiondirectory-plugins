<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2013  FusionDirectory

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

class supann
{
  static function get_all_choices_for($type)
  {
    //TODO
  }

  static function get_choices_for($type, $prefix = '')
  {
    $entity_codes   = array();
    $entity_labels  = array();
    if (file_exists(SUPANN_DIR.'/'.$type)) {
      $entiteList = file(SUPANN_DIR.'/'.$type);
      foreach ($entiteList as $line) {
        if (!preg_match ('/^#/', $line)) {
          $entite_line = preg_split('/;/', trim($line));

          $entity_codes[]   = $prefix.$entite_line[0];
          $entity_labels[]  = self::truncate_label($entite_line[1]);
        }
      }
    }
    return array($entity_codes, $entity_labels);
  }

  /* return the 64 first chars and "…" after if text is longer */
  static function truncate_label($str, $len = 64)
  {
    if (strlen($str) > $len) {
      return substr($str, 0, $len).'…';
    } else {
      return $str;
    }
  }
}

class entite extends simplePlugin
{
  var $objectclasses = array('top','supannEntite','organizationalUnit');

  var $mainTab = TRUE;

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Entite'),
      'plDescription' => _('SUPANN Entite Settings'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('supann'),
      'plObjectType'  => array('entite' => array('name' => _('SUPANN Entite'),
                                                'filter' => '(&(objectClass=supannEntite)(objectClass=organizationalUnit))')),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Entite'),
        'attrs' => array(
          new StringAttribute(
            _('Name'), _('The name to write in the ou attribute for this entite'),
            'ou', TRUE
          )
        )
      ),
      'admin' => array(
        'name'  => _('Administrative informations'),
        'attrs' => array(
          new StringAttribute(
            _('Telephone'), _('Phone number for this entite'),
            'telephoneNumber', FALSE
          ),
          new StringAttribute(
            _('Fax'), _('Fax number for this entite'),
            'facsimileTelephoneNumber', FALSE
          ),
          new StringAttribute(
            _('Postal address'), _('Postal address of this entite'),
            'postalAddress', FALSE
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of this entite'),
            'description', FALSE
          ),
        )
      ),
      'supann' => array(
        'name'  => _('SUPANN informations'),
        'attrs' => array(
          new SelectAttribute(
            _('Entite type'), _('The best SUPANN type that fits this entite'),
            'supannTypeEntite', FALSE
          ),
          new StringAttribute(
            _('Entite code'), _('The SUPANN code of this entite'),
            'supannCodeEntite', TRUE,
            '', '', '/[a-z0-9_-]/'.(strict_uid_mode()?'':'i')
          ),
          new SelectAttribute(
            _('Parent entite'), _('The parent entite of this entite'),
            'supannCodeEntiteParent', FALSE
          ),
          new StringAttribute(
            _('Reference ID'), _('SUPANN reference ID of this entite'),
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

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'supannCodeEntite='.$this->supannCodeEntite.','.get_ou('supannStructuresRDN').$this->config->current['BASE'];
  }
}

?>
