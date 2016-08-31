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

class DhcpHostsAttribute extends OrderedArrayAttribute
{
  function __construct ($label, $description, $ldapName, array $values = array(), $acl = '')
  {
    $attribute = new CompositeAttribute(
      $description, $ldapName,
      array(
        new SelectAttribute(
          '', '',
          $ldapName.'_parent', TRUE
        ),
        new SelectAttribute(
          '', '',
          $ldapName.'_mac', TRUE
        ),
        new SelectAttribute(
          '', '',
          $ldapName.'_ip', TRUE
        ),
      ),
      FALSE,
      FALSE,
      $acl,
      $label
    );
    parent::__construct($attribute, FALSE, $values, TRUE);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      if (!$this->plugin->is_template) {
        $this->loadChoices();
        $this->loadRecords();
      } elseif (isset($this->plugin->attrs[$this->getLdapName()]['count'])) {
        $this->value = array();
        // TODO $this->loadAdditionalTemplatesValues();
      }
    }
  }

  function loadChoices()
  {
    $nodes = dhcpHost::getDhcpParentNodes();
    $macs   = $this->plugin->parent->getBaseObject()->macAddress;
    if (!is_array($macs)) {
      $macs = array($macs);
    }
    $ips    = $this->plugin->parent->getBaseObject()->ipHostNumber;
    if (!is_array($ips)) {
      $ips = array($ips);
    }
    $this->setChoices($nodes, $macs, $ips);
    /* TODO - update mac and ip if main tab is modified */
  }

  function setChoices(array $parentNodes, array $macs, array $ips)
  {
    $this->attribute->attributes[0]->setChoices(array_keys($parentNodes), array_values($parentNodes));
    $this->attribute->attributes[1]->setChoices($macs);
    $this->attribute->attributes[2]->setChoices($ips);
  }

  function loadRecords()
  {
    /* TODO */
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    if ($this->plugin->is_template) {
      return;
    }
    /* FIXME - We should lock the object and check if we don’t erase changes */
    //~ $zoneTabs = objects::open($this->getZoneDn(), 'dnsZone');
    //~ $zoneTabs->getBaseObject()->attributesAccess['dnsRecords']->setValue($this->getValue());
    //~ $zoneTabs->save();
    /* TODO - replace this by adding/removing of dhcpHosts in the appropriate way */
  }
}

class dhcpHost extends simplePlugin
{
  var $objectclasses = array('fdDHCPHost');

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
      'main' => array(
        'name'  => _('DHCP zones'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new DhcpHostsAttribute(
            '', _('DHCP nodes for this host'),
            'fdDHCPParentNodes'
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
  }

  static function getDhcpParentNodes()
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
      $tmp2 = array_merge($tmp2, static::createTree($tmp, preg_replace('/^[^,]+,/i', '', $dn), '['.$cn.'] '));
    }
    return $tmp2;
  }

  static function createTree($arr, $base, $current = '')
  {
    $ret = array();
    foreach ($arr as $r => $name) {
      $base_part = str_replace($base, '', $r);
      if (preg_match('/^[a-z]*='.preg_quote($name, '/').'(|,)$/i', $base_part)) {
        $ret[$r] = $current.$name;
        $tmp = static::createTree($arr, $r, $current.'. ');
        foreach ($tmp as $sub_key => $sub_name) {
          $ret[$sub_key] = $sub_name;
        }
      }
    }
    return $ret;
  }

  function save()
  {
    /* TODO */
  }
}
