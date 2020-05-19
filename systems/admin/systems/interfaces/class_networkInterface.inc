<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2019-2020 FusionDirectory

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

class networkInterface extends simplePlugin
{
  public $base;

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Interface'),
      'plDescription' => _('Network interface'),
      'plObjectClass' => ['fdNetworkInterface'],
      'plObjectType'  => ['networkInterface' => [
        'name'        => _('Network interface'),
        'ou'          => '',
        'icon'        => 'images/empty.png',
        'tabClass'    => 'simpleTabs_noSpecial',
      ]],
      'plForeignKeys' => [
        'fdNetworkInterfaceVlanDN'    => 'ipamVlan',
        'fdNetworkInterfaceSubnetDN'  => 'ipamSubnet',
      ],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    $attributesInfo = [
      'main' => [
        'name'  => _('General information'),
        'attrs' => [
          new HostNameAttribute(
            _('Name'), _('Interface name'),
            'cn', TRUE
          ),
          new MacAddressAttribute(
            _('MAC address'), _('MAC address of this system'),
            'macAddress', FALSE
          ),
          new SetAttribute(
            new IPAttribute(
              _('IP address'), _('IP addresses this system uses (v4 or v6)'),
              'ipHostNumber', FALSE
            )
          ),
        ],
      ],
    ];
    if (class_available('ipamVlan')) {
      $attributesInfo['ipam'] = [
        'name'  => _('IPAM'),
        'attrs' => [
          new ObjectSelectAttribute(
            _('VLAN'), _('VLAN'),
            'fdNetworkInterfaceVlanDN', FALSE,
            ['ipamVlan']
          ),
          new BooleanAttribute(
            _('VLAN tag'), _('Whether this interface is going to send/receive tagged frames'),
            'fdNetworkInterfaceVlanTag', FALSE
          ),
          new SelectAttribute(
            _('Subnet'), _('Subnet'),
            'fdNetworkInterfaceSubnetDN', FALSE
          ),
          new ButtonAttribute(
            _('Add free IP from subnet'), _('Adds a new IP from selected subnet'),
            'fdAddFreeIp', _('Add IP'),
            'addFreeIpFromSubnet'
          ),
        ]
      ];
    }
    return $attributesInfo;
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    if (isset($this->attributesAccess['fdNetworkInterfaceVlanDN'])) {
      $this->attributesAccess['fdNetworkInterfaceVlanDN']->setSubmitForm('vlanChanged');
      $this->vlanChanged();
    }
  }

  function vlanChanged ()
  {
    if ($this->fdNetworkInterfaceVlanDN != '') {
      /* List subnets associated to our VLAN */
      $subnets  = objects::ls('ipamSubnet', ['dn' => 'raw','cn' => 1], NULL, '(fdIpamVlanDN='.ldap_escape_f($this->fdNetworkInterfaceVlanDN).')');
    } else {
      /* List all the subnets when no VLAN is selected */
      $subnets  = objects::ls('ipamSubnet', ['dn' => 'raw','cn' => 1]);
    }
    $this->attributesAccess['fdNetworkInterfaceSubnetDN']->setChoices(array_column($subnets, 'dn'), array_column($subnets, 'cn'));
  }

  public function addFreeIpFromSubnet ()
  {
    global $config;

    if (empty($this->fdNetworkInterfaceSubnetDN)) {
      msg_dialog::display(_('Error'), _('No subnet selected'), ERROR_DIALOG);
      return;
    }
    $subnets = objects::ls('ipamSubnet', ['dn' => 'raw','fdIpamSubnetCIDR' => 1], $this->fdNetworkInterfaceSubnetDN, '', FALSE, 'base');
    if (empty($subnets)) {
      msg_dialog::display(_('Error'), _('Could not find selected subnet'), ERROR_DIALOG);
      return;
    }
    $subnet = reset($subnets);
    if (!isset($subnet['fdIpamSubnetCIDR'])) {
      msg_dialog::display(_('Error'), _('Selected subnet has no IP'), ERROR_DIALOG);
      return;
    }
    $ldap = $config->get_ldap_link();
    $base = $config->current['BASE'];
    $ldap->cd($base);
    $ips        = $this->ipHostNumber;
    $parentIps  = array_merge_unique($this->parent->parent->getUsedIps(), $ips);
    foreach (static::ipRange($subnet['fdIpamSubnetCIDR']) as $ip) {
      if (in_array($ip, $parentIps)) {
        continue;
      }
      $filter = '(&(objectClass=ipHost)(ipHostNumber='.ldap_escape_f($ip).'))';
      $ldap->search($filter, ['ipHostNumber']);
      if ($ldap->count() === 0) {
        $ips[]              = $ip;
        $this->ipHostNumber = $ips;
        return;
      }
    }
    msg_dialog::display(_('Error'), _('Could not find any free IP in the selected subnet'), ERROR_DIALOG);
  }

  /*!
   * \brief Generator for IP range
   */
  static public function ipRange (string $cidr)
  {
    // Get mask bits
    list($net, $maskBits) = explode('/', $cidr);

    // v4 or v6?
    $size = ((strpos($net, ':') === FALSE) ? 4 : 16);

    // Convert to binary
    $net  = inet_pton($net);
    if (!$net) {
      throw new InvalidArgumentException('Invalid IP address');
    }

    // Build mask
    $solid      = floor($maskBits / 8);
    $solidBits  = $solid * 8;
    $mask       = str_repeat(chr(255), $solid);
    for ($i = $solidBits; $i < $maskBits; $i += 8) {
      $bits = max(0, min(8, $maskBits - $i));
      $mask .= chr((pow(2, $bits) - 1) << (8 - $bits));
    }
    $mask = str_pad($mask, $size, chr(0));

    $net  = ($net & $mask);
    $long = ($net & $mask);
    do {
      yield inet_ntop($long);
      $long = static::incrementIp($long);
    } while (($long & $mask) == $net);
  }

  /*!
   * \brief Takes an IPv4/IPv6 address in binary format, and increments it by given value
   */
  static protected function incrementBinaryIp (string $addr, int $increment = 1): string
  {
    for ($i = strlen($addr) - 1; $increment > 0 && $i >= 0; --$i) {
      $val        = ord($addr[$i]) + $increment;
      $increment  = $val / 256;
      $addr[$i]   = chr($val % 256);
    }

    return $addr;
  }
}
