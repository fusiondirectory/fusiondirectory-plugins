<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014  FusionDirectory

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

class systemImport
{
  static function massImport ($hosts, $objectType, $base = NULL, $otherTabs = array())
  {
    global $config;
    if ($base === NULL) {
      $base = $config->current['BASE'];
    }

    $messages = array();
    $dhcps    = array();
    $dns      = DNS::getAvailableZones($config);
    foreach ($hosts as $host) {
      if (!preg_match('/^([^.]+)\.(.*)$/', $host['id'], $m)) {
        $messages[$host['id']] = array(sprintf(_('Could not parse %s'), $host['id']));
        continue;
      }
      $cn   = $m[1].'$';
      $zone = $m[2].'.';

      $tabObject = objects::create($objectType);
      $tabObject->set_acl_base($base);
      $baseObject = $tabObject->getBaseObject();
      $baseObject->cn   = $cn;
      $baseObject->uid  = $baseObject->cn;
      $baseObject->description                = $host['description'];
      $baseObject->netConfigDNS->ipHostNumber = $host['ipAddress'];
      $baseObject->netConfigDNS->macAddress   = $host['hardwareAddress'];

      /* Get existing DHCP servers */
      if (empty($dhcps)) {
        $dhcpParentNodes = $baseObject->netConfigDNS->dhcpParentNodes;
        $ldap = $config->get_ldap_link();
        foreach ($dhcpParentNodes as $dn => $value) {
          $ldap->cat($dn);
          if ($attrs = $ldap->fetch()) {
            if (isset($attrs['dhcpNetMask'])) {
              $dhcps[$dn] = array($attrs['cn'][0],normalize_netmask($attrs['dhcpNetMask'][0]));
            }
          }
        }
      }
      /* Activate DHCP if we find a matching server */
      if (!empty($baseObject->netConfigDNS->ipHostNumber)) {
        $baseObject->netConfigDNS->dhcp_is_Account = FALSE;
        foreach ($dhcps as $dn => $infos) {
          list($ip, $mask) = $infos;
          if (tests::is_in_network($ip, $mask, $baseObject->netConfigDNS->ipHostNumber)) {
            $baseObject->netConfigDNS->dhcp_is_Account  = TRUE;
            $baseObject->netConfigDNS->dhcpParentNode   = $dn;
            break;
          }
        }
        if ($baseObject->netConfigDNS->dhcp_is_Account == FALSE) {
          $messages[$host['id']] = array(sprintf(_('No DHCP server found for ip %s'), $baseObject->netConfigDNS->ipHostNumber));
          continue;
        }
      }
      /* Activate DNS if we find a matching server */
      $baseObject->netConfigDNS->DNS_is_account = FALSE;
      foreach ($dns as $key => $value) {
        if (preg_match('/'.$zone.'$/', $value)) {
          $baseObject->netConfigDNS->DNS_is_account       = TRUE;
          $baseObject->netConfigDNS->dnsEntry['zoneName'] = $value;
          break;
        }
      }
      if ($baseObject->netConfigDNS->DNS_is_account == FALSE) {
        $messages[$host['id']] = array(sprintf(_('No DNS server found for zone %s'), $zone));
        continue;
      }

      /* Fill other tabs as asked */
      foreach ($otherTabs as $tab => $fields) {
        if (isset($tabObject->by_object[$tab])) {
          foreach ($fields as $field => $value) {
            $tabObject->by_object[$tab]->$field = $value;
          }
        } else {
          die("Tab $tab is not existing");
        }
      }

      /* Perform checks */
      $message = $tabObject->check();

      if (count($message) == 0) {
        /* No errors, save object */
        $messages[$host['id']] = TRUE;
        $tabObject->save();
      } else {
        /* Errors found */
        $messages[$host['id']] = $message;
      }
    }
    return $messages;
  }
}
?>
