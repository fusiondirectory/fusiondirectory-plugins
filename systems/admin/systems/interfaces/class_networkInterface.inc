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
            _('Mac address'), _('Mac address of this system'),
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
            'fdNetworkInterfaceVlanId', FALSE,
            ['ipamVlan'], 'fdIpamVlanInnerId'
          ),
          new StringAttribute(
            _('VLAN tag'), _('VLAN tag'),
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

    if (isset($this->attributesAccess['fdNetworkInterfaceVlanId'])) {
      $this->attributesAccess['fdNetworkInterfaceVlanId']->setSubmitForm('vlanChanged');
      $this->vlanChanged();
    }
  }

  function vlanChanged ()
  {
    if ($this->fdNetworkInterfaceVlanId != '') {
      /* Get the DN of our current VLAN */
      $vlans    = objects::ls('ipamVlan', ['fdIpamVlanInnerId' => 1], NULL, '(fdIpamVlanInnerId='.ldap_escape_f($this->fdNetworkInterfaceVlanId).')');
      $vlanDN   = key($vlans);
      $subnets  = objects::ls('ipamSubnet', ['dn' => 'raw','cn' => 1], NULL, '(fdIpamVlanDN='.$vlanDN.')');
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
    $subnets = objects::ls('ipamSubnet', ['dn' => 'raw','ipHostNumber' => 1,'fdIpamNetmask' => 1], $this->fdNetworkInterfaceSubnetDN, '', FALSE, 'base');
    if (empty($subnets)) {
      msg_dialog::display(_('Error'), _('Could not find selected subnet'), ERROR_DIALOG);
      return;
    }
    $subnet = reset($subnets);
    if (!isset($subnet['ipHostNumber'])) {
      msg_dialog::display(_('Error'), _('Selected subnet has no IP'), ERROR_DIALOG);
      return;
    }
    if (!isset($subnet['fdIpamNetmask'])) {
      msg_dialog::display(_('Error'), _('Selected subnet has no netmask'), ERROR_DIALOG);
      return;
    }
    $ldap = $config->get_ldap_link();
    $base = $config->current['BASE'];
    $ldap->cd($base);
    $ips        = $this->ipHostNumber;
    $parentIps  = array_merge_unique($this->parent->parent->getUsedIps(), $ips);
    foreach (static::ipRange($subnet['ipHostNumber'], $subnet['fdIpamNetmask']) as $ip) {
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

  /* Generator for IP range */
  static public function ipRange ($ip, $netmask)
  {
    $mask = ip2long($netmask);
    if ($mask === FALSE) {
      throw new FusionDirectoryException('Invalid mask');
    }
    $ip   = ip2long($ip);
    if ($ip === FALSE) {
      throw new FusionDirectoryException('Invalid IP');
    }
    $long = ($ip & $mask);
    do {
      yield long2ip($long);
      $long++;
    } while (($long & $mask) == $ip);
  }
}
