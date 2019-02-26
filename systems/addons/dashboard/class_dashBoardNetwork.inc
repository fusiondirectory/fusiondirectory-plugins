<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2010 Antoine Gallavardin
  Copyright (C) 2011-2016 FusionDirectory project

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

class dashboardNetwork extends simplePlugin
{
  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Network'),
      'plDescription' => _('Statistics and various information'),
      'plObjectType'  => array('dashboard'),
      'plPriority'    => 22,

      'plProvidedAcls'  => array()
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'dhcp' => array(
        'name'  => _('DHCP'),
        'attrs' => array(new FakeAttribute('dhcp_infos')),
        'template' => get_template_path('network_dhcp.tpl', TRUE, dirname(__FILE__)),
      ),
      'dns' => array(
        'name'  => _('DNS'),
        'attrs' => array(new FakeAttribute('dns_infos')),
        'template' => get_template_path('network_dhcp.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->dhcp_infos = $this->dhcp_infos();
    if (empty($this->dhcp_infos)) {
      unset($this->attributesInfo['dhcp']);
    }

    $this->dns_infos = $this->dns_infos();
    if (empty($this->dns_infos)) {
      unset($this->attributesInfo['dns']);
    }
  }

  function dhcp_infos ()
  {
    global $config, $ui;

    if (!class_available('dhcpService')) {
      return array();
    }

    try {
      $objects = objects::ls('server', array('cn' => 'raw', 'dhcpServiceDN' => 1), NULL, '(objectClass=dhcpServer)', TRUE);
    } catch (FusionDirectoryException $e) {
      $objects = array();
      msg_dialog::display(
        _('Error'),
        sprintf(_('Statistics for DHCP could not be computed because of the following error: %s'), $e->getMessage()),
        ERROR_DIALOG
      );
    }

    $servers = array();
    foreach ($objects as $dn => $attrs) {
      $zones = array();
      if (strpos($ui->get_permissions($attrs['dhcpServiceDN'], 'dhcpConfiguration/dhcpSubnet', 'dhcpNetMask'), 'r') !== FALSE) {
        $ldap_zone = $config->get_ldap_link();
        $ldap_zone->cd($attrs['dhcpServiceDN']);
        $ldap_zone->search('(objectClass=dhcpSubnet)', array('cn','dhcpNetMask'));
        while ($attrs_zone = $ldap_zone->fetch()) {
          $zones[] = array(
            'text' => $attrs_zone['cn'][0]."/".$attrs_zone['dhcpNetMask'][0]
          );
        }
      }

      $servers[] = array(
        'name'  => $attrs['cn'][0],
        'link'  => objects::link($dn, 'server', 'service_serviceDHCP', $attrs, FALSE),
        'zones' => $zones
      );
    }

    return $servers;
  }

  function dns_infos ()
  {
    if (!class_available('dnsZone')) {
      return array();
    }
    $servers  = array();
    $zones    = objects::ls('dnsZone', array('zoneName' => 1, 'sOARecord' => 1), NULL, '', TRUE);
    foreach ($zones as $zonedn => $zone) {
      list ($fqdn)  = explode(' ', $zone['sOARecord']);
      $search = dnsManagement::findServerByFQDN($fqdn, $zonedn);
      foreach ($search as $dn => $name) {
        if (!isset($servers[$dn])) {
          $servers[$dn] = array(
            'name'  => $name,
            'link'  => objects::link($dn, 'server', 'tab_dnsHost', $name, FALSE),
            'zones' => array()
          );
        }
        $servers[$dn]['zones'][] = array(
          'link' => objects::link($zonedn, 'dnsZone', '', $zone['zoneName'], FALSE)
        );
      }
    }

    return array_values($servers);
  }
}
