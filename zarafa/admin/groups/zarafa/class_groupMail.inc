<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2009-2010 Bas Tichelaar
 * Copyright (C) 2013 FusionDirectory
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

class ZarafaMailsAttribute extends MailsAttribute
{
  function getForbiddenValues ()
  {
    $forbidden = array_values($this->getValue());
    $forbidden = array_merge($forbidden, array_values($this->plugin->attributesAccess['gosaMailAlternateAddress']->getValue()));
    $forbidden[] = $this->plugin->attributesAccess['mail']->getValue();
    return $forbidden;
  }

  function getFilterBlackList ()
  {
    return array('mail' => $this->getForbiddenValues());
  }
}

class zarafamailgroup extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('gosaMailAccount');
  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'    => _('Zarafa'),
      'plDescription'  => _('Zarafa Group mail'),
      'plSelfModify'   => FALSE,
      'plPriority'     => 10,
      'plCategory'     => array('groups'),
      'plObjectType'   => array('group'),
      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=goMailServer)", array('cn'));
    $servercn = array();
    while ($attrs = $ldap->fetch()) {
        $servercn[] = $attrs['cn'][0];
    }
    $this->attributesAccess['gosaMailServer']->setChoices($servercn);
  }


  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'section1' => array(
        'name'  => _('Generic'),
        'attrs' => array(
          new MailAttribute (
            _('Primary address'), _('The primary mail address'),
            'mail', TRUE
          ),
          new SelectAttribute (
            _('Server'), _('Server mail for zarafa'),
            'gosaMailServer', TRUE,
            array('')
          ),
          new SetAttribute (
            new StringAttribute (
              _('Alternative addresses'), _('Alternative mail addresses for zarafa group'),
              'gosaMailAlternateAddress'
            )
          ),
          new ZarafaMailsAttribute (
            _('Forward messages to non group members'), _(''),
            'gosaMailForwardingAddress'
          )
        )
      )
    );
  }
}
