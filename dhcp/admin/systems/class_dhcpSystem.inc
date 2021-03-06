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

class DhcpHostsAttribute extends OrderedArrayAttribute
{
  private $nodesCache = [];
  private static $templateValueSeparator = '^';

  function __construct ($label, $description, $ldapName, array $values = [], $acl = '')
  {
    $attribute = new CompositeAttribute(
      $description, $ldapName,
      [
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
      ],
      FALSE,
      FALSE,
      $acl,
      $label
    );
    parent::__construct($attribute, FALSE, $values, TRUE);
    $this->setHeaders([
      _('Parent'),
      _('Mac'),
      _('IP')
    ]);
    $this->setInLdap(FALSE);
  }

  protected function loadAttrValue (array $attrs)
  {
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    return array_slice($value, 0, 3);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->loadChoices();
      if ($this->plugin->is_template) {
        $this->value = [];
        $this->loadAdditionalTemplatesValues();
      } else {
        $this->loadRecords();
      }
      $this->plugin->is_account = !empty($this->value);
      $this->plugin->initially_was_account = $this->plugin->is_account;
    }
  }

  function loadAdditionalTemplatesValues ()
  {
    if (isset($this->plugin->attrs[$this->getLdapName()]['count'])) {
      for ($i = 0; $i < $this->plugin->attrs[$this->getLdapName()]['count']; $i++) {
        $this->value[] = explode(static::$templateValueSeparator, $this->plugin->attrs[$this->getLdapName()][$i]);
      }
    }
  }

  function loadChoices ()
  {
    list($nodesChoices, $nodes) = dhcpSystem::getDhcpParentNodes();
    $macs   = $this->plugin->parent->getBaseObject()->macAddress;
    if (!is_array($macs)) {
      $macs = [$macs];
    }
    $ips    = $this->plugin->parent->getBaseObject()->ipHostNumber;
    if (!is_array($ips)) {
      $ips = [$ips];
    }
    $this->nodesCache = $nodes;
    $this->setChoices($nodesChoices, $macs, $ips);
  }

  function setChoices (array $parentNodes, array $macs, array $ips)
  {
    asort($parentNodes, SORT_NATURAL);
    $this->attribute->attributes[0]->setChoices(array_keys($parentNodes), array_values($parentNodes));
    if (is_object($this->plugin) && $this->plugin->is_template) {
      $this->attribute->attributes[1] = new StringAttribute(
        '', _('The Mac address to use in the DHCP host record'),
        $this->getLdapName().'_mac', TRUE,
        '%macAddress%'
      );
      $this->attribute->attributes[2] = new StringAttribute(
        '', _('The IP address to use in the DHCP host record'),
        $this->getLdapName().'_ip', TRUE,
        '%ipHostNumber%'
      );
      $this->attribute->setAttributes($this->attribute->attributes);
    } else {
      $this->attribute->attributes[1]->setChoices($macs);
      $this->attribute->attributes[2]->setChoices($ips);
    }
  }

  function loadRecords ()
  {
    global $config;

    $this->value = [];

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=dhcpHost)(cn='.$this->plugin->parent->getBaseObject()->cn.')(dhcpHWAddress=*))', ['dhcpStatements','dhcpHWAddress','dn']);
    while ($attrs = $ldap->fetch()) {
      $ip = '';
      foreach ($attrs['dhcpStatements'] as $statement) {
        if (preg_match('/fixed-address ([0-9\.]+)$/', $statement, $m)) {
          $ip = $m[1];
          break;
        }
      }
      $this->value[] = [
        preg_replace('/^[^,]+,/', '', $attrs['dn']),
        preg_replace('/ethernet (([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2}))/', '\\1', $attrs['dhcpHWAddress'][0]),
        $ip,
        $attrs['dn']
      ];
    }
    $this->initialValue = $this->getValue();
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap, $fullrewrite, $ipchanged, array $oldips, array $newips, $macchanged, array $oldmacs, array $newmacs)
  {
    global $config, $ui;

    if ($this->plugin->is_template) {
      return;
    }
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
          if (in_array($value[2], $removedips)) {
            unset($values[$key]);
          }
        }
      }
    }
    if ($macchanged) {
      if ((count($oldmacs) == 1) && (count($newmacs) == 1)) {
        foreach ($values as &$value) {
          if ($value[1] == reset($oldmacs)) {
            $value[1] = reset($newmacs);
          }
        }
        unset($value);
      } else {
        $removedmacs = array_diff($oldmacs, $newmacs);
        foreach ($values as $key => $value) {
          if (in_array($value[1], $removedmacs)) {
            unset($values[$key]);
          }
        }
      }
    }

    $valueHashes    = array_flip(array_map('join', $values));
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
    $errors = [];
    foreach ($values as $value) {
      $configDn = $value[0];
      if (preg_match('/([^,]+,'.preg_quote(get_ou('dhcpRDN')).'.*'.preg_quote($config->current['BASE']).')$/', $value[0], $m)) {
        $configDn = $m[1];
      } else {
        trigger_error('Failed to parse '.$value[0].' to find DHCP configuration DN');
      }
      /* Check locking */
      if ($locks = Lock::get($configDn)) {
        $errors[] = sprintf(_('"%s" is locked (by "%s" since %s), could not save modifications to this DHCP configuration'), $locks[0]->objectDn, $locks[0]->userDn, date('%Y-%m-%d, %H:%M:%S', $locks[0]->timestamp));
        continue;
      } else {
        /* Lock the current entry */
        Lock::add($configDn);
      }
      $dhcpTabs = objects::open($configDn, 'dhcpConfiguration');
      $dhcpTabs->getBaseObject()->attributesAccess['dhcpSections']->addHost($value[0], $cn, $value[1], $value[2]);
      $msgs = $dhcpTabs->save();
      Lock::deleteByObject($configDn);
      msg_dialog::displayChecks($msgs);
    }

    return $errors;
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      if (!is_array($this->value)) {
        return new SimplePluginCheckError(
          $this,
          SimplePluginCheckError::invalidValue(_('Value is not an array'))
        );
      }
      foreach ($this->value as $value) {
        $error = dhcpSystem::dhcpIsIpInSubnet($this->nodesCache[$value[0]], $value[2]);
        if ($error !== TRUE) {
          return new SimplePluginCheckError(
            $this,
            htmlescape($error)
          );
        }
      }
    }
  }

  function fillLdapValue (array &$attrs)
  {
    if ($this->plugin->is_template) {
      $attrs[$this->getLdapName()] = array_map(
        function ($v)
        {
          return implode(static::$templateValueSeparator, $v);
        },
        $this->value
      );
    }
  }
}

class dhcpSystem extends simplePlugin
{
  protected $zonesCache;
  protected $loaded = FALSE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('DHCP'),
      'plDescription'   => _('Edit the DHCP zones of a system'),
      'plIcon'          => 'geticon.php?context=applications&icon=dhcp&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=dhcp&size=16',
      'plObjectType'    => ['server','workstation','terminal','printer','component','phone','mobilePhone'],
      'plPriority'      => 4,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('DHCP zones'),
        'class' => ['fullwidth'],
        'attrs' => [
          new DhcpHostsAttribute(
            '', _('DHCP hosts declared for this system'),
            'dhcpHosts'
          )
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->ignore_account = FALSE;
  }

  function is_this_account ($attrs)
  {
    /* Will not work when call from constructor (or when $attrs is not us)
     * See DhcpHostsAttribute::setParent which sets is_account later
     * */
    return (count($this->dhcpHosts) > 0);
  }

  function setTemplate (bool $bool)
  {
    parent::setTemplate($bool);
    $this->attributesAccess['dhcpHosts']->setParent($this);
  }

  public function update (): bool
  {
    parent::update();
    $this->is_account = $this->is_this_account($this->attrs);
    return TRUE;
  }

  static function getDhcpParentNodes ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(|(objectClass=dhcpService)(objectClass=dhcpGroup)'.
                    '(objectClass=dhcpSubnet)(objectClass=dhcpSharedNetwork))', ['cn', 'dhcpNetMask']);

    $nodes    = [];
    $choices  = [];
    while ($attrs = $ldap->fetch()) {
      $subdn = preg_replace('/,'.preg_quote(get_ou('dhcpRDN')).'.+$/', '', $attrs['dn']);
      $parts = ldap_explode_dn($subdn, 1);
      if ($parts === FALSE) {
        /* Fallback */
        $choices[$attrs['dn']] = $subdn;
      } else {
        unset($parts['count']);
        $choices[$attrs['dn']] = implode('>', array_reverse($parts));
      }
      $nodes[$attrs['dn']] = $attrs;
    }
    return [$choices, $nodes];
  }

  /* Returns error text if the IP is not in the given subnet LDAP node, or TRUE */
  static function dhcpIsIpInSubnet ($subnetAttrs, $ip)
  {
    if (isset($subnetAttrs['dhcpNetMask'][0])) {
      if (!tests::is_in_network($subnetAttrs['cn'][0], normalize_netmask($subnetAttrs['dhcpNetMask'][0]), $ip)) {
        return sprintf(_('The ip %s is not in the network %s/%d'), $ip, $subnetAttrs['cn'][0], $subnetAttrs['dhcpNetMask'][0]);
      }
    }
    return TRUE;
  }

  protected function shouldSave (): bool
  {
    /* We need ldap_save to always run */
    return TRUE;
  }

  protected function ldap_save (): array
  {
    global $config;

    $errors = parent::ldap_save();
    if (!empty($errors)) {
      return $errors;
    }

    $ldap = $config->get_ldap_link();
    $baseObject = $this->parent->getBaseObject();
    return $this->attributesAccess['dhcpHosts']->postLdapSave(
      $ldap,
      $baseObject->attributeHaveChanged('cn'),
      $baseObject->attributeHaveChanged('ipHostNumber'),
      $baseObject->attributeInitialValue('ipHostNumber'),
      $baseObject->attributeValue('ipHostNumber'),
      $baseObject->attributeHaveChanged('macAddress'),
      $baseObject->attributeInitialValue('macAddress'),
      $baseObject->attributeValue('macAddress')
    );
  }

  protected function ldap_remove (): array
  {
    if ($this->is_template) {
      return parent::ldap_remove();
    } elseif (($this->dn != '') && ($this->dn != 'new')) {
      /* Remove all records */
      $this->attributesAccess['dhcpHosts']->setValue([]);
      return $this->ldap_save();
    } else {
      return [];
    }
  }

  function adapt_from_template (array $attrs, array $skip = [])
  {
    parent::adapt_from_template($attrs, $skip);
    $this->attributesAccess['dhcpHosts']->loadChoices();
    $this->attributesAccess['dhcpHosts']->loadAdditionalTemplatesValues();
    $this->is_account = $this->is_this_account($this->attrs);
  }
}
