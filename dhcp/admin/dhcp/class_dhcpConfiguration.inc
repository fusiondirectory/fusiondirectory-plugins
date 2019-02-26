<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class dhcpConfiguration extends simplePlugin
{
  public $types = [];

  static $sectionMap = [
    'dhcpService'       => [
      'dhcpSharedNetwork','dhcpSubnet','dhcpGroup',
      'dhcpHost','dhcpClass','dhcpTSigKey','dhcpDnsZone','dhcpFailOverPeer'
    ],
    'dhcpClass'         => ['dhcpSubClass'],
    'dhcpSubClass'      => [],
    'dhcpHost'          => [],
    'dhcpGroup'         => ['dhcpHost'],
    'dhcpPool'          => [],
    'dhcpSubnet'        => [
      'dhcpPool','dhcpGroup',
      'dhcpHost','dhcpClass','dhcpTSigKey','dhcpDnsZone','dhcpFailOverPeer'
    ],
    'dhcpSharedNetwork' => ['dhcpSubnet', 'dhcpPool','dhcpTSigKey','dhcpDnsZone','dhcpFailOverPeer'],
    'dhcpFailOverPeer'  => [],
    'dhcpTSigKey'       => [],
    'dhcpDnsZone'       => []
  ];

  static $quote_option = ['domain-name'];

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('DHCP configuration'),
      'plDescription' => _('DHCP configuration'),
      'plObjectClass' => ['dhcpService'],
      'plObjectType'  => ['dhcpConfiguration' => [
        'name'        => _('DHCP configuration'),
        'ou'          => get_ou('dhcpRDN'),
        'icon'        => 'geticon.php?context=applications&icon=dhcp&size=16'
      ]],
      'plForeignKeys'   => [
        'dhcpSections' => [
          ['serverGeneric', 'dn', '(|(dhcpPrimaryDN=%oldvalue%)(dhcpSecondaryDN=%oldvalue%))'],
        ],
      ],
      'plSearchAttrs' => ['description'],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('DHCP Objects'),
        'class' => ['fullwidth'],
        'attrs' => [
          new BaseSelectorAttribute(get_ou('dhcpRDN')),
          new HiddenAttribute('cn'),
          new DhcpSectionsAttribute(
            '', _('The DHCP sections handled by this server'),
            'dhcpSections', FALSE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    $this->types = [
      'dhcpLog'           => _('Logging'),
      'dhcpService'       => _('Global options'),
      'dhcpClass'         => _('Class'),
      'dhcpSubClass'      => _('Subclass'),
      'dhcpHost'          => _('Host'),
      'dhcpGroup'         => _('Group'),
      'dhcpPool'          => _('Pool'),
      'dhcpSubnet'        => _('Subnet'),
      'dhcpFailOverPeer'  => _('Failover peer'),
      'dhcpSharedNetwork' => _('Shared network'),
      'dhcpTSigKey'       => _('DNS update key'),
      'dhcpDnsZone'       => _('DNS update zones')
    ];

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->cn = $this->attributesAccess['dhcpSections']->getGlobalCn();
  }

  function save_object ()
  {
    parent::save_object();
    $this->cn = $this->attributesAccess['dhcpSections']->getGlobalCn();
  }

  function save (): array
  {
    $this->cn = $this->attributesAccess['dhcpSections']->getGlobalCn();
    return parent::save();
  }

  protected function shouldSave (): bool
  {
    return TRUE;
  }

  protected function ldap_save (): array
  {
    global $config;
    $this->ldap_error = 'Success';
    /* Save dhcp settings */
    $ldap   = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
    $cache  = $this->attributesAccess['dhcpSections']->getCache();
    $errors = [];
    $new    = ($this->orig_dn == 'new');
    foreach ($cache as $dn => $data) {
      if (($this->dn != $this->orig_dn) && !$new) {
        $dn = preg_replace('/'.preg_quote($this->orig_dn, '/').'$/i', $this->dn, $dn);
      }

      /* Remove entry? */
      if (count($data) == 0) {
        if (!$new) {
          /* Check if exists, then remove... */
          if ($ldap->cat($dn)) {
            $ldap->rmdir_recursive($dn);
            if (!$ldap->success()) {
              $this->ldap_error = $ldap->get_error();
              $errors[] = msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class());
            }
          }
        }
        continue;
      }

      /* Modify existing entry? */
      if (isset($data['MODIFIED']) || ($this->orig_dn != $this->dn)) {
        if ($ldap->cat($dn)) {
          $modify = TRUE;
        } else {
          $modify = FALSE;
        }

        /* Build new entry */
        $attrs = [];
        foreach ($data as $attribute => $values) {
          if ($attribute == 'MODIFIED' || $attribute == 'dn') {
            continue;
          }

          if (in_array($attribute, ['dhcpServerDN','dhcpFailOverPeerDN'])) {
            foreach ($values as $v_key => $value) {
              $values[$v_key] = preg_replace('/'.preg_quote($this->orig_dn, '/').'$/i', $this->dn, $value);
            }
          }

          if (count($values)) {
            if ($attribute == 'dhcpOption') {
              foreach ($values as $key => $value) {
                $option_name  = trim(preg_replace('/[^ ]*$/', '', $value));
                $option_value = trim(preg_replace('/^[^ ]*/', '', $value));
                if (in_array($option_name, static::$quote_option)) {
                  $values[$key] = $option_name.' "'.$option_value.'"';
                }
              }
            }
            if (is_array($values) && (count($values) == 1)) {
              $attrs[$attribute] = $values[0];
            } else {
              $attrs[$attribute] = $values;
            }
          } elseif ($modify) {
            $attrs[$attribute] = [];
          }
        }

        $ldap->cd($dn);
        if ($modify) {
          $ldap->modify($attrs);
          if (!$ldap->success()) {
            $this->ldap_error = $ldap->get_error();
            $errors[] = msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class());
          }
        } else {
          $ldap->add($attrs);
          if (!$ldap->success()) {
            $this->ldap_error = $ldap->get_error();
            $errors[] = msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class());
          }
        }
      }
    }

    return $errors;
  }
}
