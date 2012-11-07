<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011-2012 FusionDirectory project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class boardNetwork extends simplePlugin
{
  var $dhcp_infos = array();
  var $dns_infos  = array();

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->dhcp_infos = $this->dhcp_infos();
    if (empty($this->dhcp_infos)) {
      unset($this->attributesInfo['dhcp']);
    }

    $this->dns_infos = $this->dns_infos();
    if (empty($this->dns_infos)) {
      unset($this->attributesInfo['dns']);
    }
  }

  function dhcp_infos()
  {
    $ldap = $this->config->get_ldap_link();

    $ldap->cd(get_ou("systemRDN").$this->config->current['BASE']);
    $ldap->search("(objectClass=dhcpServer)", array("cn","dhcpServiceDN"));

    $servers = array();
    while ($attrs = $ldap->fetch()) {
      $zones = array();
      $ldap_zone = $this->config->get_ldap_link();
      $ldap_zone->cd($attrs['dhcpServiceDN'][0]);
      $ldap_zone->search("(objectClass=dhcpSubnet)", array("cn","dhcpNetMask"));
      while ($attrs_zone = $ldap_zone->fetch()) {
        $zones[] = $attrs_zone['cn'][0]."/".$attrs_zone['dhcpNetMask'][0];
      }

      $servers[] = array(
        'name'  => $attrs['cn'][0],
        'zones' => $zones
      );
    }

    return $servers;
  }

  function dns_infos()
  {
    $ldap = $this->config->get_ldap_link();

    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=dNSZone)(zoneName=*))", array("dn","zoneName"));

    $servers = array();
    while ($attrs = $ldap->fetch()) {
      $dn = preg_replace("/^zoneName=[^,]+,/", "", $attrs['dn']);
      if (preg_match("/^cn=/", $dn)) {
        $cn = preg_replace("/^cn=([^,]+),.*$/", "$1", $dn);
        if (!isset($servers[$dn])) {
          $servers[$dn] = array('name' => $cn, 'zones' => array());
        }
        $servers[$dn]['zones'][$attrs['zoneName'][0]] = $attrs['zoneName'][0];
      } elseif (preg_match("/^zoneName=/", $dn)) {
        $reverseName  = preg_replace("/^zoneName=([^,]+),.*$/", "$1", $attrs['dn']);
        $zoneName     = preg_replace("/^zoneName=([^,]+),.*$/", "$1", $dn);
        $dn = preg_replace("/^zoneName=[^,]+,/", "", $dn);
        $IP = DNS::FlipIp(str_replace(".in-addr.arpa.", "", $reverseName));
        for ($dots = count(explode(".", $IP));$dots < 4;$dots++) {
          $IP .= ".0";
        }
        $servers[$dn]['zones'][$zoneName] .= " ($IP)";
      }
    }

    return array_values($servers);
  }

  static function getAttributesInfo()
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

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Board - Network"),
      "plDescription" => _("Statistics and various informations"),
      "plSelfModify"  => FALSE,
      "plSection"     => array("addon"),
      "plCategory"    => array("board"),
      "plObjectType"  => array("board"),

      "plProvidedAcls"    => array()
    );
  }
}
?>
