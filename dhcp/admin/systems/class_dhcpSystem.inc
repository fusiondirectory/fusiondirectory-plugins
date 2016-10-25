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
          '', _('The DHCP configuration or subsection in which this host should be added'),
          $ldapName.'_parent', TRUE
        ),
        new SelectAttribute(
          '', _('The Mac address to use in the DHCP host record'),
          $ldapName.'_mac', TRUE
        ),
        new SelectAttribute(
          '', _('The IP address to use in the DHCP host record'),
          $ldapName.'_ip', TRUE
        ),
        new HiddenAttribute(
          'dn'
        )
      ),
      FALSE,
      FALSE,
      $acl,
      $label
    );
    parent::__construct($attribute, FALSE, $values, TRUE);
  }

  protected function loadAttrValue ($attrs)
  {
  }

  protected function getAttributeArrayValue($key, $value)
  {
    return array_slice($value, 0, 3);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      if (!$this->plugin->is_template) {
        $this->loadChoices();
        $this->loadRecords();
        $this->plugin->is_account = !empty($this->value);
      } elseif (isset($this->plugin->attrs[$this->getLdapName()]['count'])) {
        $this->value = array();
        // TODO $this->loadAdditionalTemplatesValues();
      }
    }
  }

  function loadChoices()
  {
    $nodes = dhcpSystem::getDhcpParentNodes();
    $macs   = $this->plugin->parent->getBaseObject()->macAddress;
    if (!is_array($macs)) {
      $macs = array($macs);
    }
    $ips    = $this->plugin->parent->getBaseObject()->ipHostNumber;
    if (!is_array($ips)) {
      $ips = array($ips);
    }
    $this->setChoices($nodes, $macs, $ips);
  }

  function setChoices(array $parentNodes, array $macs, array $ips)
  {
    $this->attribute->attributes[0]->setChoices(array_keys($parentNodes), array_values($parentNodes));
    $this->attribute->attributes[1]->setChoices($macs);
    $this->attribute->attributes[2]->setChoices($ips);
  }

  function loadRecords()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=dhcpHost)(cn='.$this->plugin->parent->getBaseObject()->cn.')(dhcpHWAddress=*))', array('dhcpStatements','dhcpHWAddress','dn'));
    while ($attrs = $ldap->fetch()) {
      $ip = '';
      foreach ($attrs['dhcpStatements'] as $statement) {
        if (preg_match('/fixed-address ([0-9\.]+)$/', $statement, $m)) {
          $ip = $m[1];
          break;
        }
      }
      $this->value[] = array(
        preg_replace('/^[^,]+,/', '', $attrs['dn']),
        preg_replace('/ethernet (([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2}))/', '\\1', $attrs['dhcpHWAddress'][0]),
        $ip,
        $attrs['dn']
      );
    }
    $this->initialValue = $this->getValue();
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap, $fullrewrite, $ipchanged, $oldips, $newips, $macchanged, $oldmacs, $newmacs)
  {
    global $config;

    if ($this->plugin->is_template) {
      return;
    }
    /* FIXME - We should lock the object and check if we don’t erase changes */
    $values   = $this->getValue();
    $iValues  = $this->getInitialValue();
    /* React to changes on main tab */
    $cn       = $this->plugin->parent->getBaseObject()->cn;
    if ($ipchanged) {
      if ((count($oldips) == 1) && (count($newips) == 1)) {
        foreach ($values as &$value) {
          if ($value[2] == reset($oldips)) {
            $value[2] = reset($newips);
          }
        }
        unset($value);
      } else {
        $removedips = array_diff($oldips, $newips);
        foreach ($values as $key => $value) {
          if (in_array($value, $removedips)) {
            unset($values[$key]);
          }
        }
      }
    }
    if ($macchanged) {
      if ((count($oldmacs) == 1) && (count($newmacs) == 1)) {
        foreach ($values as &$value) {
          if ($value[2] == reset($oldmacs)) {
            $value[2] = reset($newmacs);
          }
        }
        unset($value);
      } else {
        $removedmacs = array_diff($oldmacs, $newmacs);
        foreach ($values as $key => $value) {
          if (in_array($value, $removedmacs)) {
            unset($values[$key]);
          }
        }
      }
    }

    $valueHashes    = array_flip(array_map('join', $values));
    //~ $iValueHashes   = array_flip(array_map('join', $iValues));
    foreach ($iValues as $value) {
      $hash = join($value);
      if (isset($valueHashes[$hash]) && !$fullrewrite) {
        unset($values[$valueHashes[$hash]]);
        continue;
      }
      $configDn = $value[0];
      if (preg_match('/([^,]+,'.preg_quote(get_ou('dhcpRDN')).'.*'.preg_quote($config->current['BASE']).')$/', $value[0], $m)) {
        $configDn = $m[1];
      } else {
        trigger_error('Failed to parse '.$value[0].' to find DHCP configuration DN');
      }
      $dhcpTabs = objects::open($configDn, 'dhcpConfiguration');
      $dhcpTabs->getBaseObject()->attributesAccess['dhcpSections']->delHost($value[3]);
      $errors = $dhcpTabs->save();
      msg_dialog::displayChecks($errors);
    }
    foreach ($values as $value) {
      $configDn = $value[0];
      if (preg_match('/([^,]+,'.preg_quote(get_ou('dhcpRDN')).'.*'.preg_quote($config->current['BASE']).')$/', $value[0], $m)) {
        $configDn = $m[1];
      } else {
        trigger_error('Failed to parse '.$value[0].' to find DHCP configuration DN');
      }
      $dhcpTabs = objects::open($configDn, 'dhcpConfiguration');
      $dhcpTabs->getBaseObject()->attributesAccess['dhcpSections']->addHost($value[0], $cn, $value[1], $value[2]);
      $errors = $dhcpTabs->save();
      msg_dialog::displayChecks($errors);
    }
    /* TODO
     * ? handle modification somehow (if we want to keep statements when changing IP, not sure about this)
     * We should also look into what should be possible or not, like IP, Mac or cn duplication.
     * */
  }
}

class dhcpSystem extends simplePlugin
{
  var $objectclasses = array();

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
            '', _('DHCP hosts declared for this system'),
            'dhcpHosts'
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->ignore_account = FALSE;
  }

  function is_this_account($attrs)
  {
    /* Will not work when call from constructor (or when $attrs is not us)
     * See DhcpHostsAttribute::setParent which sets is_account later
     * */
    return (count($this->dhcpHosts) > 0);
  }

  function save_object()
  {
    parent::save_object();
    $this->is_account = $this->is_this_account($this->attrs);
  }

  static function getDhcpParentNodes()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(|(objectClass=dhcpService)(objectClass=dhcpGroup)'.
                    '(objectClass=dhcpSubnet)(objectClass=dhcpSharedNetwork))', array('cn'));

    $nodes = array();
    while ($attr = $ldap->fetch()) {
      $subdn = preg_replace('/,'.preg_quote(get_ou('dhcpRDN')).'.+$/', '', $attr['dn']);
      $parts = ldap_explode_dn($subdn, 1);
      if ($parts === FALSE) {
        /* Fallback */
        $nodes[$attr['dn']] = $subdn;
      } else {
        unset($parts['count']);
        $nodes[$attr['dn']] = implode('>', array_reverse($parts));
      }
    }
    return $nodes;
  }

  function ldap_save()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $baseObject = $this->parent->getBaseObject();
    return $this->attributesAccess['dhcpHosts']->postLdapSave(
      $ldap,
      $baseObject->attributeHaveChanged('cn'),
      $baseObject->attributeHaveChanged('ipHostNumber'),
      $baseObject->attributeInitialValue('ipHostNumber'),
      $baseObject->attributeValue('ipHostNumber'),
      $baseObject->attributeHaveChanged('macAddress'),
      array($baseObject->attributeInitialValue('macAddress')),
      array($baseObject->attributeValue('macAddress'))
    );
  }
}
