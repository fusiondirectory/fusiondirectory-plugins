<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016  FusionDirectory

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

class dhcpHost extends simplePlugin
{
  var $objectclasses = array('fdDHCPHost');
  //~ var $displayHeader = TRUE;

  protected $zonesCache;
  protected $loaded = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('DHCP'),
      'plDescription'   => _('Edit the DHCP zones of a system'),
      'plIcon'          => 'geticon.php?context=applications&icon=dhcp&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=dhcp&size=16',
      'plObjectType'    => array('server','workstation','terminal','printer','component','winstation','phone','mobilePhone'),
      'plPriority'      => 4,
      //~ 'plForeignKeys'  => array(
        //~ 'fdDNSZoneDn' => 'dnsZone'
      //~ ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'zones' => array(
        'name'  => _('DHCP zones'),
        'attrs' => array(
          new SetAttribute(
            new SelectAttribute(
              '', _('DHCP nodes for this host'),
              'fdDHCPParentNodes', TRUE
            )
          )
        )
      ),
      //~ 'soa' => array(
        //~ 'name'  => _('SOA records'),
        //~ 'attrs' => array()
      //~ ),
      //~ 'records' => array(
        //~ 'name'  => _('DNS Records'),
        //~ 'class' => array('fullwidth'),
        //~ 'attrs' => array()
      //~ ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    //~ $this->zonesCache = objects::ls('dnsZone');
    //~ $this->attributesAccess['fdDNSZoneDn']->attribute->setChoices(array_keys($this->zonesCache), array_values($this->zonesCache));
    //~ $this->attributesAccess['fdDNSZoneDn']->setSubmitForm('zoneChanged');
    print_r($this->get_dhcp_parent_nodes());
  }

  function get_dhcp_parent_nodes()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=dhcpService)', array('cn'));

    $dhcp_dns = array();
    while ($attr = $ldap->fetch()) {
      $dhcp_dns[$attr['dn']] = $attr['cn'][0];
    }

    $tmp = $tmp2 = array();
    foreach ($dhcp_dns as $dn => $cn) {
      $ldap->cd($dn);
      $ldap->search('(|(objectClass=dhcpService)(objectClass=dhcpGroup)'.
                    '(objectClass=dhcpSubnet)(objectClass=dhcpSharedNetwork))', array('cn'));
      while ($attr = $ldap->fetch()) {
        $tmp[$attr['dn']] = $attr['cn'][0];
      }
      $tmp2 = array_merge($tmp2, $this->create_tree($tmp, preg_replace('/^[^,]+,/i', '', $dn), '('.$cn.')&nbsp;'));
    }
    return $tmp2;
  }

  function create_tree($arr, $base, $current = '')
  {
    $ret = array();
    foreach ($arr as $r => $name) {
      $base_part = str_replace($base, '', $r);
      if (preg_match('/^[a-z]*='.preg_quote($name, '/').'(|,)$/i', $base_part)) {
        $ret[$r] = $current.$name;
        $tmp = $this->create_tree($arr, $r, $current.'.&nbsp;');
        foreach ($tmp as $sub_key => $sub_name) {
          $ret[$sub_key] = $sub_name;
        }
      }
    }
    return $ret;
  }
}
