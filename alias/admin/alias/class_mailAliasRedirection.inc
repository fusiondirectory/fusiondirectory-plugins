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

class mailAliasRedirection extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Temporary mail redirection'),
      'plDescription' => _('Temporary mail redirection'),
      'plObjectClass' => ['mailAliasRedirection'],
      'plObjectType'  => ['mailAliasRedirection' => [
        'name'        => _('Temporary mail redirection'),
        'aclCategory' => 'alias',
        'icon'        => 'geticon.php?context=applications&icon=alias-redirection&size=16',
        'ou'          => get_ou('aliasRDN'),
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      // Attributes are grouped by section
      'main' => [
        'name'  => _('Mail redirection'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('aliasRDN')),
          new HostNameAttribute(_('Name'), _('Name to identify this redirection'), 'cn', TRUE),
          new TextAreaAttribute(
            _('Description'), _('Description of this redirection'),
            'description', FALSE
          ),
          new SetAttribute(
            new MailAttribute(
              _('Redirect from'), _('Mail address from which you want to redirect'),
              'mail', TRUE
            )
          ),
          new SetAttribute(
            new MailAttribute(
              _('Redirect to'), _('Destination of this redirection'),
              'gosaMailForwardingAddress', TRUE
            )
          ),
          new DateAttribute(
            _('Expiration date'), _('Date after which the redirection should be deleted. Leave empty for no deletion.'),
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
