<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016 FusionDirectory

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

class mailPluginConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return array(
      'plShortName'     => _('Mail'),
      'plDescription'   => _('Mail plugin configuration'),
      'plPriority'      => 3,
      'plObjectClass'   => array('fdMailPluginConf'),
      'plObjectType'    => array('configuration'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'mail' => array(
        'name'  => _('Mail settings'),
        'attrs' => array(
          new SelectAttribute(
            _('Account identification attribute'),
            _('Which attribute will be used to create accounts.'),
            'fdMailAttribute', TRUE,
            array('mail', 'uid')
          ),
          new StringAttribute(
            _('Mail user template'),
            _('Override the user account creation syntax.'),
            'fdMailUserCreation'
          ),
          new StringAttribute(
            _('Mail folder template'),
            _('Override the methods default account creation syntax.'),
            'fdMailFolderCreation'
          ),
          new BooleanAttribute(
            _('Use cyrus UNIX style'),
            _("Determines if 'foo/bar' or 'foo.bar' should be uses as namespaces in IMAP."),
            'fdCyrusUseSlashes'
          ),
          new BooleanAttribute(
            _('Delete mailbox on account deletion'),
            _('Determines if the mailbox should be removed from your IMAP server after the account is deleted in LDAP.'),
            'fdCyrusDeleteMailbox'
          ),
          new SetAttribute(
            new StringAttribute(
              _('Cyrus autocreate folders'),
              _('List of personal IMAP folders that should be created along initial account creation.'),
              'fdCyrusAutocreateFolders'
            )
          ),
          new IntAttribute(
            _('IMAP timeout'),
            _('Sets the connection timeout for imap actions.'),
            'fdImapTimeout', FALSE,
            0, FALSE
          ),
          new StringAttribute(
            _('Shared prefix'),
            _('Prefix to add for mail shared folders.'),
            'fdMailSharedPrefix'
          ),
        )
      ),
    );
  }
}
?>
