<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2022 FusionDirectory

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

class PartageConf extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Partage'),
      'plTitle'         => _('Renater-Partage configuration page'),
      'plDescription'   => _('FusionDirectory Partage plugin configuration'),
      'plObjectClass'   => ['fdRenaterPartageConf'],
      'plObjectType'    => ['configuration'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'partageConf' => [
        'name'  => _('Partage Plugin Configuration'),
        'attrs' => [
           new SelectAttribute(
              _('Mailbox actions upon user mail tab removal'),    _('Option to take in case of user mail tab removal'),
              'fdRenaterPartageConfDeleteOption', TRUE,
               [  'delete',   'disable'], '',
              [_('Delete'),_('Disable')]
           ),
           new BooleanAttribute(
              _('Trust LDAP and override Partage'),    _('Option to override Partage with LDAP data'),
              'fdRenaterPartageConfOverride', FALSE
          ),
           new BooleanAttribute(
              _('Re-activate mailbox upon LDAP updates'),    _('Option to re-activate mailbox if updates are pushed from LDAP'),
              'fdRenaterPartageConfActivateAccount', FALSE
          ),
        ]
      ],
      'partageConfToken' => [
        'name'  => _('Partage Plugin Token Configuration'),
        'attrs' => [
          new HiddenArrayAttribute(
            'fdRenaterPartageConfToken', FALSE
          ),
          new IntAttribute(
            _('Token lifetime'), _('Defines lifetime of the access token in seconds. Max 4min59'/*5min is server token expiry*/),
            'fdRenaterPartageConfTokenLifetime', TRUE,
            1, 299, 270
          ),
        ]
      ],
      'partageAdvanceConf' => [
        'name'  => _('Partage Plugin Advanced Configuration'),
        'attrs' => [
          new IntAttribute(
            _('Port protection against parallel script execution'), _('Set the TCP port to be used for the protective socket against parallel script execution'),
            'fdRenaterPartageConfPortProtection', TRUE,
            1025, 65535, 20825
          ),
          new IntAttribute(
            _('Timelapse between parallel detection (sec)'), _('Set the required timelapse to respect after a parallel script detection'),
            'fdRenaterPartageConfTimeProtection', TRUE,
            1, 60, 5
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);
  }

}
