<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2010 Antoine Gallavardin
  Copyright (C) 2011-2019 FusionDirectory project

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

class opsiImport extends systemImport
{
  protected $types = ['workstation'];

  /* Return plugin information for acl handling  */
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('OPSI import'),
      'plDescription' => _('Import windows stations from OPSI into FD'),
      'plIcon'        => 'geticon.php?context=applications&icon=opsi&size=48',
      'plObjectType'  => [
        'opsiImport' => [
          'name'      => _('OPSI import'),
          'tabClass'  => 'simpleTabs_noSpecial',
          'mainAttr'  => FALSE,
        ]
      ],
      'plSection'     => 'conf',
      'plPriority'    => 15,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (string $filter = 'objectClass=opsiServer'): array
  {
    $attributesInfo = parent::getAttributesInfo($filter);
    /* Add our checkbox just before the import button */
    array_splice(
      $attributesInfo['import']['attrs'],
      -1,
      0,
      [
        new StringAttribute(
          _('Interface name'), _('How to name the network interface created on imported systems'),
          'networkInterfaceName', TRUE,
          'eth0'
        ),
        new BooleanAttribute(
          _('Import localboot products'), _('Import localboot product list configured in OPSI in the FD OPSI tab'),
          'importLocalboots', FALSE,
          FALSE
        ),
      ]
    );
    return $attributesInfo;
  }

  protected function getExtraTabs ($server_dn, $host): array
  {
    $extraTabs = [
      'sambaSystemTab' => [
        'is_account'      => TRUE,
      ],
      'opsiClient' => [
        'is_account'      => TRUE,
        'fdOpsiServerDn'  => $server_dn,
      ]
    ];

    if ($this->importLocalboots) {
      if (!empty($host['localboots'])) {
        $extraTabs['opsiClient']['fdOpsiLocalbootProduct'] = $host['localboots'];
      }
    }

    return $extraTabs;
  }

  protected function getHosts ($server_dn)
  {
    global $config;
    $s_daemon = new supportDaemon();
    if (!$s_daemon->is_available()) {
      $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
      $error->display();
      return FALSE;
    }
    try {
      $macAddress = opsiProfile::getOpsiServerMac($server_dn);
    } catch (FusionDirectoryError $error) {
      $error->display();
      return FALSE;
    }
    $hosts = $s_daemon->append_call('OPSI.host_getObjects', $macAddress, ['args' => [[], ['type' => 'OpsiClient']]]);
    if ($s_daemon->is_error()) {
      $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
      $error->display();
      return FALSE;
    }

    if ($this->importLocalboots) {
      foreach ($hosts as &$host) {
        $res = $s_daemon->append_call(
          'OPSI.productOnClient_getObjects',
          $macAddress,
          ['args' => [
            [],
            [
              'clientId'    => $host['id'],
              'type'        => 'ProductOnClient',
              'productType' => 'LocalbootProduct',
            ]
          ]]
        );
        if ($s_daemon->is_error()) {
          $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
          $error->display();
          return FALSE;
        }
        $host['localboots'] = [];
        foreach ($res as $productOnClient) {
          if ($productOnClient['productId'] == 'opsi-winst') {
            /* opsi-winst cannot be installed and has no actions */
            continue;
          }
          if ($productOnClient['actionRequest'] != 'none') {
            $host['localboots'][] = $productOnClient['productId'].'|'.$productOnClient['actionRequest'];
          } elseif ($productOnClient['installationStatus'] == 'installed') {
            $host['localboots'][] = $productOnClient['productId'].'|setup';
          }
        }
      }
      unset($host);
    }

    foreach ($hosts as $host) {
      if (!empty($host['hardwareAddress'])) {
        if (empty($host['ipAddress'])) {
          $host['ipAddress'] = [];
        } elseif (!is_array($host['ipAddress'])) {
          $host['ipAddress'] = [$host['ipAddress']];
        }
        if (!is_array($host['hardwareAddress'])) {
          $host['hardwareAddress'] = [$host['hardwareAddress']];
        }
        $host['interfaces'] = [
          [
            'cn'            => [$this->networkInterfaceName],
            'ipHostNumber'  => $host['ipAddress'],
            'macAddress'    => $host['hardwareAddress'],
          ]
        ];
      }
      unset($host['ipAddress']);
      unset($host['hardwareAddress']);
    }

    return $hosts;
  }
}
