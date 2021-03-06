<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class serviceCyrus extends simpleMailMethodService
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Cyrus (IMAP/POP3)'),
      'plDescription'   => _('Cyrus (IMAP/POP3)').' ('._('Services').')',
      'plIcon'          => 'geticon.php?context=applications&icon=cyrus&size=16',
      'plObjectClass'   => ['fdCyrusServer'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    global $config;

    return  [
      'cyrus' => [
        'name'  => _('Connection'),
        'attrs' => [
          new CompositeAttribute(
            _('Connect URL for Cyrus server'),
            'fdCyrusConnect',
            [
              new StringAttribute(
                _('Hostname'), _('Hostname of the Cyrus server'),
                'cyrusConnectURL', TRUE
              ),
              new IntAttribute(
                _('Port'), _('Port number on which Cyrus server should be contacted'),
                'cyrusConnectPort', TRUE,
                0 /*min*/, FALSE /*max*/, 143
              ),
              new SelectAttribute(
                _('Option'), _('Options for contacting Cyrus server'),
                'cyrusConnectOptions1', TRUE,
                ['notls', 'tls', 'ssl'], 'notls'
              ),
              new SelectAttribute(
                _('Valide certificats'), _('Weither or not to validate server certificate on connexion'),
                'cyrusConnectOptions2', FALSE,
                ['', '/validate-cert', '/novalidate-cert'], '',
                ['', 'validate',       'no-validate']
              )
            ],
            '/^{(.*):(\\d+)\\/([^\\/]+)(.*)}$/',
            '{%s:%d/%s%s}'
          ),
          new StringAttribute(
            _('Admin user'), _('Imap server admin user'),
            'fdCyrusAdmin', TRUE
          ),
          new StringAttribute(
            _('Password'), _('Admin user password'),
            'fdCyrusPassword', TRUE
          ),
        ]
      ],
      'sieve' => [
        'name'  => _('Sieve'),
        'attrs' => [
          new CompositeAttribute(
            _('Sieve connect URL for Cyrus server'),
            'fdCyrusSieveServer',
            [
              new StringAttribute(
                _('Hostname'), _('Hostname of the Cyrus sieve server'),
                'cyrusSieveConnectURL', TRUE
              ),
              new IntAttribute(
                _('Port'), _('Port number on which Cyrus sieve server should be contacted'),
                'cyrusSieveConnectPort', TRUE,
                0 /*min*/, FALSE /*max*/, 4190
              ),
              new SelectAttribute(
                _('Option'), _('Options for contacting Cyrus sieve server'),
                'cyrusSieveConnectOptions', TRUE,
                ['notls', 'tls', 'ssl'], 'notls'
              ),
            ],
            '/^{(.*):(\\d+)\\/([^\\/]+)}$/',
            '{%s:%d/%s}'
          )
        ],
      ],
      'settings' => [
        'name'  => _('Settings'),
        'attrs' => [
          new BooleanAttribute(
            _('Use UNIX style'), _('Determines if "foo/bar" or "foo.bar" should be used as namespaces in IMAP'),
            'fdCyrusUseSlashes', FALSE,
            ($config->get_cfg_value('CyrusUseSlashes') === 'TRUE')
          ),
          new SetAttribute(
            new StringAttribute(
              _('Autocreate folders'), _('List of personal IMAP folders that should be created along initial account creation.'),
              'fdCyrusAutocreateFolders', FALSE
            ),
            $config->get_cfg_value('CyrusAutocreateFolders', [])
          ),
        ],
      ],
    ];
  }
}
