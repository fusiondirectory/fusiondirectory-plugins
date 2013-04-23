<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2012-2013 FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

class sympaAlias extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('sympaAlias');

  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Sympa list alias'),
      'plDescription' => _('Sympa list alias'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('alias'),
      'plObjectType'  => array('sympaAlias' => array('name'  => _('Sympa list alias'),
                                                      'filter'  => 'objectClass=sympaAlias')),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('Sympa list alias'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('aliasRDN')),
          new HostNameAttribute (_('Name'), _('Name to identify this alias'), 'cn', TRUE),
          new TextAreaAttribute (
            _('Description'), _('Description of this alias'),
            'description', FALSE
          ),
          new SetAttribute(
            new MailAttribute (
              _('Email address'), _('Email address'),
              'mail', TRUE
            )
          ),
          new SelectAttribute (
            _('Sympa server'), _('Sympa server fot this alias'),
            'sympaServerURL', TRUE,
            array()
          )
        )
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search('(objectClass=sympaServer)', array('sympaServerURL'));
    $server = array();
    while ($attrs = $ldap->fetch()) {
        $server[] = $attrs['sympaServerURL'][0];
    }
    $this->attributesInfo['main']['attrs']['sympaServerURL']->setChoices($server);
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'cn='.$this->cn.','.get_ou('aliasRDN').$this->base;
  }
}

?>
