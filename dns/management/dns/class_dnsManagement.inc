<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2015-2019  FusionDirectory

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

class dnsManagement extends management
{
  public $neededAttrs = ['sOARecord' => '*'];

  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn',     []],
    ['LinkColumn',           ['attributes' => 'nameAttr',             'label' => 'Name']],
    ['ActionsColumn',        ['label' => 'Actions']],
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('DNS'),
      'plTitle'       => _('DNS Management'),
      'plDescription' => _('Manage DNS zones and views'),
      'plIcon'        => 'geticon.php?context=applications&icon=dns&size=48',
      'plSection'     => ['systems' => ['name' => _('Systems'), 'priority' => 3]],
      'plPriority'    => 1,
      'plManages'     => ['dnsZone', 'dnsView', 'dnsAcl'],
    ];
  }

  public static function findServerByFQDN ($fqdn, $zoneDn = NULL): array
  {
    global $config;
    list ($serverCn, $serverZone) = explode('.', $fqdn, 2);
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ips = [];
    $ldap->search('(&(|(aRecord=*)(aAAARecord=*))(relativeDomainName='.$serverCn.')(zoneName='.$serverZone.'))', ['aRecord', 'aAAARecord']);
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['aRecord'])) {
        unset($attrs['aRecord']['count']);
        $ips = array_merge($ips, $attrs['aRecord']);
      }
      if (isset($attrs['aAAARecord'])) {
        unset($attrs['aAAARecord']['count']);
        $ips = array_merge($ips, $attrs['aAAARecord']);
      }
    }
    if (!empty($ips)) {
      $filter = '(|(ipHostNumber='.implode(')(ipHostNumber=', $ips).'))';
      if ($zoneDn !== NULL) {
        $filter = '(&'.$filter.'(fdDNSZoneDn='.ldap_escape_f($zoneDn).'))';
      }
      return objects::ls('server', NULL, NULL, $filter);
    }
    return [];
  }
}
