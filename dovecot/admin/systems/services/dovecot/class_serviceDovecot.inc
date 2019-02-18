<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class serviceDovecot extends simpleMailMethodService
{
  static function plInfo (): array
  {
    return array(
      'plShortName'     => _('Dovecot (IMAP/POP3)'),
      'plDescription'   => _('Dovecot (IMAP/POP3)').' ('._('Services').')',
      'plIcon'          => 'geticon.php?context=applications&icon=dovecot&size=16',
      'plObjectClass'   => array('fdDovecotServer'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
      return array (
      'connection' => array (
        'name'  => _('Dovecot connection'),
        'attrs' => array (
          new CompositeAttribute (
            _('Connect URL for Dovecot server'),
            'fdDovecotConnect',
            array(
              new StringAttribute(
                _('Hostname'), _('Hostname of the Dovecot server'),
                'dovecotConnectURL', TRUE
              ),
              new IntAttribute(
                _('Port'), _('Port number on which Dovecot server should be contacted'),
                'dovecotConnectPort', TRUE,
                0 /*min*/, FALSE /*max*/, 143
              ),
              new SelectAttribute(
                _('Option'), _('Options for contacting Dovecot server'),
                'dovecotConnectOptions1', TRUE,
                array('notls', 'tls', 'ssl'), 'notls'
              ),
              new SelectAttribute(
                _('Validate TLS/SSL Certificates'), _('Weither or not to validate server certificates on connection'),
                'dovecotConnectOptions2', FALSE,
                array('', '/validate-cert', '/novalidate-cert'), '',
                array('', 'validate',       'no-validate')
              )
            ),
            '/^{(.*):(\\d+)\\/([^\\/]+)(.*)}$/',
            '{%s:%d/%s%s}'
          ),
        )
      ),
      'credentials' => array (
        'name'  => _('Master credentials'),
        'attrs' => array (
          new StringAttribute (
            _('Admin user'), _('Dovecot server admin user'),
            'fdDovecotAdmin', TRUE
          ),
          new StringAttribute (
            _('Password'), _('Admin user password'),
            'fdDovecotPassword', TRUE
          ),
          new StringAttribute (
            _('Mail directory'), _('Path to the directory in which user maildirs must be created'),
            'fdDovecotMailDir', TRUE,
            '/var/mail'
          ),
        )
      ),
      'options' => array (
        'name'  => _('Options'),
        'attrs' => array (
          new BooleanAttribute (
            _('Create the user folder via Argonaut'), _('Use argonaut to create the user mailbox on the Dovecot server'),
            'fdDovecotArgonautMkdir', TRUE,
            TRUE
          ),
        )
      )
    );
  }
}
?>
