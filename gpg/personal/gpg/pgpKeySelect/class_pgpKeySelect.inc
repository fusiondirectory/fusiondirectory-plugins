<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2019  FusionDirectory

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

class pgpKeySelect extends selectManagement
{
  /* Create pqpKey object type */
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('PGP Key selection'),
      'plDescription' => _('PGP Key selection for a user'),
      'plObjectType'  => ['pgpKey' => [
        'name'        => _('PGP Key'),
        'description' => _('PGP Key information'),
        'filter'      => '(objectClass=pgpKeyInfo)',
        'icon'        => 'geticon.php?context=applications&icon=gpg&size=16',
        'mainAttr'    => 'pgpCertID',
        'ou'          => static::getPgpKeysOu(),
      ]],

      'plProvidedAcls'  => static::getPgpFields(),
    ];
  }

  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn',  []],
    ['LinkColumn',        ['attributes' => 'pgpCertID',         'label' => 'Cert ID']],
    ['LinkColumn',        ['attributes' => 'pgpKeyID',          'label' => 'Key ID']],
    ['LinkColumn',        ['attributes' => 'pgpUserID',         'label' => 'User ID']],
    ['LinkColumn',        ['attributes' => 'pgpKeyCreateTime',  'label' => 'Creation time']],
    ['LinkColumn',        ['attributes' => 'pgpKeyExpireTime',  'label' => 'Expiration']],
    ['LinkColumn',        ['attributes' => 'pgpKeyType',        'label' => 'Algorithm']],
    ['LinkColumn',        ['attributes' => 'pgpKeySize',        'label' => 'Size']],
    ['LinkColumn',        ['attributes' => 'pgpRevoked',        'label' => 'Revoked']],
    ['LinkColumn',        ['attributes' => 'pgpDisabled',       'label' => 'Disabled']],
  ];

  /* Disable and hide configuration system */
  protected $skipConfiguration = TRUE;

  protected function setUpListing ()
  {
    /* Set baseMode to FALSE */
    $this->listing  = new managementListing($this, FALSE, $this->multiSelect);
  }

  public static function getPgpKeysOu (): string
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cat('cn=pgpServerInfo,'.$config->current['BASE']);
    if ($pgpServerInfo = $ldap->fetch()) {
      return preg_replace(
        '/'.$config->current['BASE'].'$/', '',
        $pgpServerInfo['pgpBaseKeySpaceDN'][0]
      );
    } else {
      msg_dialog::display(_('Configuration error'), _('You need to configure GPG base dn through the addons section first'));
      return '';
    }
  }

  public static function getPgpFields (): array
  {
    return [
      'pgpCertID'         => _('Cert ID'),
      'pgpKeyID'          => _('Key ID'),
      'pgpUserID'         => _('User ID'),
      'pgpKeyCreateTime'  => _('Creation time'),
      'pgpKeyExpireTime'  => _('Expiration'),
      'pgpKeyType'        => _('Algorithm'),
      'pgpKeySize'        => _('Size'),
      'pgpRevoked'        => _('Revoked'),
      'pgpDisabled'       => _('Disabled')
    ];
  }

  static function pgpDate ($date)
  {
    if (empty($date)) {
      return $date;
    }
    return DateTime::createFromFormat('Ymd', substr($date, 0, 8))->format('d.m.Y');
  }

  static function pgpSize ($size)
  {
    // Remove useless 0 at the beginning
    return strval(intval($size));
  }

  static function pgpRevoked ($value)
  {
    return ($value == 0 ? 'No' : 'Yes');
  }
}
