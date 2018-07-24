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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class etablissement extends simplePlugin
{
  var $objectclasses = array('top','supannEntite','organization','supannOrg','eduOrg');

  var $mainTab = TRUE;

  /* Return plugin information */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Establishment'),
      'plDescription' => _('SUPANN Establishment Settings'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('etablissement' => array(
        'name'        => _('SUPANN Establishment'),
        'filter'      => '(&(objectClass=supannEntite)(objectClass=organization))',
        'ou'          => get_ou('supannStructuresRDN'),
        'mainAttr'    => 'supannCodeEntite',
        'icon'        => 'geticon.php?context=applications&icon=supann-etablissement&size=16',
        'nameAttr'    => 'o',
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new BooleanAttribute(
            _('Root Establishment'), _('Set this Establishment as the root one'),
            'set_root'
          ),
          new StringAttribute(
            _('Name'), _('The name to write in the o attribute for this Establishment'),
            'o', TRUE
          ),
          new TextAreaAttribute(
            _('Description'), _('A short description of this Establishment'),
            'description', FALSE
          )
        )
      ),
      'location' => array(
        'name'  => _('Location'),
        'attrs' => array(
          new StringAttribute(
            _('Location'), _('Usually the city where this Establishment is'),
            'l', FALSE
          ),
          new PostalAddressAttribute(
            _('Address'), _('The postal address of this Establishment'),
            'postalAddress', FALSE
          ),
          new StringAttribute(
            _('Telephone'), _('Phone number for this Establishment'),
            'telephoneNumber', FALSE
          ),
          new StringAttribute(
            _('Fax'), _('Fax number for this Establishment'),
            'facsimileTelephoneNumber', FALSE
          ),
        )
      ),
      'supann' => array(
        'name'  => _('SUPANN properties'),
        'attrs' => array(
          new StringAttribute(
            _('Establishment code'), _('The code of this Establishment (must have a prefix between {})'),
            'supannEtablissement', TRUE,
            '', '', '/^{[^}]+}.*$/'
          ),
          new StringAttribute(
            _('SUPANN code'), _('The SUPANN code for this Establishment'),
            'supannCodeEntite', TRUE,
            '', '', '/^[a-z0-9_-]*$/i'
          ),
          new StringAttribute(
            _('Legal name'), _('The legal name of this Establishment'),
            'eduOrgLegalName', TRUE
          ),
          new StringAttribute(
            _('Home page URI'), _('The URI of this Establishment website home page'),
            'eduOrgHomePageURI', FALSE
          ),
          new StringAttribute(
            _('Institution URI'), _('The URI of this Establishment institution website'),
            'eduOrgSuperiorURI', FALSE
          ),
          new StringAttribute(
            _('White pages URI'), _('The URI of this Establishment white pages'),
            'eduOrgWhitePagesURI', FALSE
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['o']->setUnique(TRUE);
    $this->attributesAccess['supannCodeEntite']->setUnique(TRUE);
    $this->attributesAccess['supannEtablissement']->setUnique(TRUE);
    $this->attributesAccess['eduOrgLegalName']->setUnique(TRUE);

    $root_code = $this->get_root_code();
    $this->attributesAccess['set_root']->setInLdap(FALSE);
    $this->attributesAccess['set_root']->setDisabled(($root_code !== FALSE) && ($root_code != $this->supannEtablissement));
    $this->set_root = (($root_code !== FALSE) && ($root_code == $this->supannEtablissement));
  }

  protected function ldap_remove()
  {
    global $config;
    $errors = parent::ldap_remove();
    if (!empty($errors)) {
      return $errors;
    }

    /* If we're the root etablissement, delete it too */
    $root_code = $this->get_root_code();
    if (($root_code !== FALSE) && ($root_code == $this->supannEtablissement)) {
      $dn   = 'o='.$this->o.','.$config->current['BASE'];
      $ldap = $config->get_ldap_link();
      $ldap->rmdir($dn);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_DEL, get_class()));
      }
    }
    return array();
  }

  function ldap_save()
  {
    global $config;
    $errors = parent::ldap_save();

    $ldap = $config->get_ldap_link();

    $root_mode = FALSE;
    $root_code = $this->get_root_code();
    if (($root_code === FALSE) && ($this->set_root)) {
      /* Set this etablissement as root one */
      $ldap->cat($this->dn);
      $root_attrs = $ldap->fetch();
      unset($root_attrs['count']);
      unset($root_attrs['dn']);
      foreach ($root_attrs as $key => $value) {
        if (is_numeric($key)) {
          unset($root_attrs[$key]);
          continue;
        }
        if (is_array($root_attrs[$key])) {
          unset($root_attrs[$key]['count']);
        }
      }
      $root_mode  = 'add';
    } elseif (($root_code !== FALSE) && ($root_code == $this->supannEtablissement) && $this->set_root) {
      /* We are the root etablissement, we need to update it */
      $root_attrs = $this->attrs;
      $root_mode  = 'modify';
    } elseif (($root_code !== FALSE) && ($root_code == $this->supannEtablissement) && !$this->set_root) {
      /* We are the root etablissement, we want to delete it */
      $root_mode = 'delete';
    }

    if ($root_mode) {
      $root_attrs['objectClass'] = array('top','dcObject','organization','supannOrg','eduOrg');
      $root_attrs['dc'] = $root_attrs['o'];
      unset($root_attrs['supannCodeEntite']);
      $dn = 'o='.$this->o.','.$config->current['BASE'];
      if ($root_mode == 'delete') {
        $ldap->rmdir($dn);
      } else {
        $ldap->cd($dn);
        $ldap->$root_mode($root_attrs);
      }
      if (!$ldap->success()) {
        $errors[] = msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class());
      }
    }
    return $errors;
  }

  function get_root_code()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=supannOrg)', array('*'), 'one');

    if ($ldap->count() > 1) {
      msg_dialog::display(_('LDAP error'), 'There are several Establishment at root!');
      return FALSE;
    } elseif ($ldap->count() > 0) {
      $attr = $ldap->fetch();
      return $attr['supannEtablissement'][0];
    } else {
      return FALSE;
    }
  }
}
?>
