<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class serviceCyrus extends simpleService
{

  /* This plugin only writes its objectClass */
  var $objectclasses = array("fdCyrusServer");

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Cyrus (IMAP/POP3)'),
      'plDescription'   => _('Cyrus (IMAP/POP3)').' ('._('Services').')',
      'plIcon'          => 'plugins/cyrus/images/iconMini.png',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
      return array (
      'cyrus' => array (
        'name'  => _('Cyrus settings'),
        'attrs' => array (
          new CompositeAttribute (
            _('Connect URL for Cyrus server'),
            'fdCyrusConnect',
            array(
              new StringAttribute(_('URL'), _('URL to contact Cyrus server'), 'cyrusConnectURL'),
              new IntAttribute(_('Port'), _('Port number on which Cyrus server should be contacted'), 'cyrusConnectPort', FALSE, 0 /*min*/, FALSE /*max*/),
              new SelectAttribute(_('Option'), _('Options for contacting Cyrus server'), 'cyrusConnectOptions1', TRUE, array('notls', 'tls', 'ssl'), 'notls'),
              new SelectAttribute(_('Valide certificats'), _('Weither or not to validate server certificate on connexion'), 'cyrusConnectOptions2', FALSE, array('', '/validate-cert', '/novalidate-cert'), '', array('', 'validate', 'no-validate'))
            ),
            '/^{(.+):(\\d+)\\/([^\\/]+)(.+)}$/',
            '{%s:%d/%s%s}'
          ),
          new StringAttribute (
            _('Admin user'), _('Imap server admin user'),
            'fdCyrusAdmin', TRUE
          ),
          new StringAttribute (
            _('Password'), _('Admin user password'),
            'fdCyrusPassword', TRUE
          ),
        )
      ),
      'sieve' => array (
        'name'  => _('Sieve settings'),
        'attrs' => array (
          new CompositeAttribute (
            _('Sieve connect URL for Cyrus server'),
            'fdCyrusSieveServer',
            array(
              new StringAttribute(_('URL'), _('URL to contact Cyrus sieve server'), 'cyrusSieveConnectURL'),
              new IntAttribute(_('Port'), _('Port number on which Cyrus sieve server should be contacted'), 'cyrusSieveConnectPort', FALSE, 0 /*min*/, FALSE /*max*/),
              new SelectAttribute(_('Option'), _('Options for contacting Cyrus sieve server'), 'cyrusSieveConnectOptions', TRUE, array('notls', 'tls', 'ssl'), 'notls'),
            ),
            '/^{(.+):(\\d+)\\/([^/]+)}$/',
            '{%s:%d/%s}'
          )
        )
      )
    );
  }

}

?>
