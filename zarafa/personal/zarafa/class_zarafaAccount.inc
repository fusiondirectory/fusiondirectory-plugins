<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2009-2010 Bas Tichelaar
  Copyright (C) 2013  FusionDirectory

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

class zarafaAccount extends simplePlugin
{
  var $displayHeader = TRUE;
  var $objectclasses = array("gosaMailAccount", "zarafa-user");

  /*!
   * \brief Plugin information
   */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Zarafa'),
      'plDescription'   => _('Zarafa settings'),
      'plIcon'          => _('plugins/zarafa/images/zarafa.png'),
      'plSelfModify'    => TRUE,                              // Does this plugin have an owner that might be able to edit its entry
      'plCategory'      => array('user'),
      'plObjectType'    => array('user'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }


  /*!
   * \brief Constructor of the ZarafaAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=goMailServer)",array('cn'));
    $servercn = array();
    while ($attrs = $ldap->fetch()) {
        $servercn[] = $attrs['cn'][0];
    }
    $this->attributesAccess['gosaMailServer']->setChoices($servercn);
    $this->attributesAccess['zarafaAccount']->setManagedAttributes(
      array(
        'erase' => array(
          FALSE => array (
            'zarafaSharedStoreOnly','zarafaHidden','zarafaAdmin',
            'zarafaResourceType','zarafaResourceCapacity',
          )
        )
      )
    );
    $this->attributesAccess['zarafaSharedStoreOnly']->setManagedAttributes(
      array(
        'erase' => array(
          FALSE => array(
            'zarafaAdmin',
          )
        )
      )
    );
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
            array("")
          ),
          new IntAttribute (
            _('Hard quota size'), _(''),
            'zarafaQuotaHard', FALSE,
            0,                              // [FloatAttribute] Minimum value
            FALSE,                          // [FloatAttribute] No maximum value
            ''                              // [FloatAttribute] Default value
          ),
          new IntAttribute (
            _('Soft quota size'), _(''),
            'zarafaQuotaSoft', FALSE,                          // Not mandatory
            0,                              // [FloatAttribute] Minimum value
            FALSE,                          // [FloatAttribute] No maximum value
            ''                              // [FloatAttribute] Default value
          ),
          new IntAttribute (
            _('Warn quota size'), _(''),
            'zarafaQuotaWarn', FALSE,
            0,                              // [FloatAttribute] Minimum value
            FALSE,                          // [FloatAttribute] No maximum value
            ''                              // [FloatAttribute] Default value
          ),
          new SetAttribute (
            new StringAttribute (
              _('Alternative addresses'), _('Alternative mail addresses for zarafa group'),
              'gosaMailAlternateAddress', FALSE,
              ''
            )
          ),
          new MailsAttribute (
            _('Forward messages to non group members'), _(''),
            'gosaMailForwardingAddress', FALSE,
            ''
          )
        )
      ),
      'section2' => array (
        'name'  => _('Zarafa specific settings'),
        'attrs' => array(
          new BooleanAttribute (
            _('Enable Zarafa'), _('Enable Zarafa Account for this user'),
            'zarafaAccount', FALSE,
            FALSE,
            '',
            1,
            0
          ),
          new BooleanAttribute (
            _('Zarafa Administrator'), _('Make this user a Zarafa Administrator'),
            'zarafaAdmin', FALSE,
            FALSE,
            '',
            1,
            0
          ),
          new BooleanAttribute (
            _('Zarafa shared store'), _('Make this user a Zarafa shared store'),
            'zarafaSharedStoreOnly', FALSE,
            FALSE,
            '',
            1,
            0
          ),
          new BooleanAttribute (
            _('Hide from addressbook'), _('Hide from Zarafa addressbook'),
            'zarafaHidden', FALSE,
            FALSE,
            '',
            1,
            0
          ),
          new SelectAttribute (
            _('Resource type'), _(''),
            'zarafaResourceType', FALSE,
            array('user','room','equipment'),
            "",
            array(_('User'),_('Room'),_('Equipment'))
          ),
          new IntAttribute (
            _('Resource capacity'), _(''),
            'zarafaResourceCapacity', FALSE,
            0,
            FALSE,
            ''
          ),
          new MailsAttribute (
            _('Send as privileges'), _(''),
            'zarafaSendAsPrivilege', FALSE,
            ''
          )
        )
      )
    );
  }
}
