<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
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

class mailAliasDistribution extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Temporary mail distribution'),
      'plDescription' => _('Temporary mail distribution'),
      'plObjectClass' => ['mailAliasDistribution'],
      'plObjectType'  => ['mailAliasDistribution' => [
        'name'        => _('Temporary mail distribution'),
        'aclCategory' => 'alias',
        'icon'        => 'geticon.php?context=applications&icon=alias-distribution&size=16',
        'ou'          => get_ou('aliasRDN'),
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Mail distribution'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('aliasRDN')),
          new HostNameAttribute(_('Name'), _('Name to identify this alias'), 'cn', TRUE),
          new TextAreaAttribute(
            _('Description'), _('Description of this alias'),
            'description', FALSE
          ),
          new MailAttribute(
            _('Email address'), _('Email address'),
            'mail', TRUE
          ),
          new SetAttribute(
            new MailAttribute(
              _('Email aliases'), _('Aliases for this email address'),
              'gosaMailAlternateAddress', TRUE
            )
          ),
          new SelectAttribute(
            _('Mail server'), _('Mail server for this alias'),
            'gosaMailServer', FALSE,
            array_keys(mailMethod::getMailServers())
          ),
          new DateAttribute(
            _('Expiration date'), _('Date after which the alias should be deleted. Leave empty for no deletion.'),
            'aliasExpirationDate', FALSE,
             // Unix timestamp
            'U',
            ''
          )
        ]
      ],
    ];
  }
}
