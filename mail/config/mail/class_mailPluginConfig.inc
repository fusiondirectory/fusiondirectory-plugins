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
    return [
      'plShortName'     => _('Mail'),
      'plDescription'   => _('Mail plugin configuration'),
      'plPriority'      => 6,
      'plObjectClass'   => ['fdMailPluginConf'],
      'plObjectType'    => ['configuration'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'mail' => [
        'name'  => _('Mail settings'),
        'attrs' => [
          new SelectAttribute(
            _('Account identification attribute'),
            _('Which attribute will be used to create accounts.'),
            'fdMailAttribute', TRUE,
            ['mail', 'uid']
          ),
          new StringAttribute(
            _('User account template'),
            _('Override the user account creation syntax. Default is %PREFIX%%UATTRIB%.'),
            'fdMailUserCreation'
          ),
          new StringAttribute(
            _('Group account template'),
            _('Override the group account creation syntax. Default is %PREFIX%%UATTRIB%.'),
            'fdMailFolderCreation'
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
        ]
      ],
    ];
  }
}
