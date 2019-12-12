<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019 FusionDirectory

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

class totpConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('TOTP'),
      'plTitle'         => _('TOTP configuration'),
      'plDescription'   => _('FusionDirectory TOTP plugin configuration'),
      'plObjectClass'   => ['fdTotpPluginConf'],
      'plCategory'      => ['configuration'],
      'plObjectType'    => ['smallConfig'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;

    return [
      'main' => [
        'name'  => _('TOTP'),
        'attrs' => [
          new SelectAttribute(
            _('Digest'), _('Digest to use for TOTP tokens'),
            'fdTotpDigest', TRUE,
            hash_algos(), 'sha1'
          ),
          new IntAttribute(
            _('Period'), _('Validity period for TOTP tokens'),
            'fdTotpPeriod', TRUE,
            1, FALSE, 30
          ),
          new IntAttribute(
            _('Digits'), _('Number of digits for TOTP tokens'),
            'fdTotpDigits', TRUE,
            1, FALSE, 6
          ),
        ]
      ],
    ];
  }
}
