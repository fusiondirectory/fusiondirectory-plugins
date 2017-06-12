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

/* DnsRecords edition showing only records for our host */
class DnsRecordsFilteredAttribute extends DnsRecordsAttribute
{
  function __construct ($label, $description, $ldapName, $zoneDn, $defaultValue = array(), $acl = "")
  {
    parent::__construct ($label, $description, $ldapName, FALSE, $defaultValue, $acl);
    $this->zoneDn = $zoneDn;
  }

  protected function loadAttrValue ($attrs)
  {
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      if (!$this->plugin->is_template) {
        $this->loadRecords();
      } elseif (isset($this->plugin->attrs[$this->getLdapName()]['count'])) {
        $this->value = array();
        $this->loadAdditionalTemplatesValues();
      }
    }
  }

  function loadAdditionalTemplatesValues ()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cat($this->getZoneDn());
    $attrs = $ldap->fetch();
    $this->zoneName = $attrs['zoneName'][0];

    for ($i = 0; $i < $this->plugin->attrs[$this->getLdapName()]['count']; $i++) {
      $this->value[] = explode('|', $this->plugin->attrs[$this->getLdapName()][$i]);
    }
  }

  function loadRecords ()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cat($this->getZoneDn());
    $attrs = $ldap->fetch();
    $this->zoneName = $attrs['zoneName'][0];
    parent::loadAttrValue($attrs);
    $this->initialValue = $this->getValue();
  }

  protected function getAttributeArrayValue($key, $value)
  {
    $values = parent::getAttributeArrayValue($key, $value);
    if ($this->plugin->is_template) {
      return $values;
    }
    $ips    = $this->plugin->parent->getBaseObject()->ipHostNumber;
    $cn     = preg_replace('/\$$/', '', $this->plugin->parent->getBaseObject()->cn);
    if (in_array($values[2], $ips) || ($values[0] == $cn) || preg_match('/'.preg_quote($cn.'.'.$this->getZoneName(), '/i').'/', $value[2])) {
      return $values;
    } else {
      return array();
    }
  }

  function fillLdapValue (&$attrs)
  {
    if ($this->plugin->is_template) {
      $attrs[$this->getLdapName()] = array_map(function ($v) {return implode('|', $v);}, $this->value);
    }
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    if ($this->plugin->is_template) {
      return;
    }
    $zoneTabs = objects::open($this->getZoneDn(), 'dnsZone');
    $zoneTabs->getBaseObject()->attributesAccess['dnsRecords']->setValue($this->getValue());
    $zoneTabs->save();
  }
}

class dnsHost extends simplePlugin
{
  var $objectclasses = array('fdDNSHost');
  var $displayHeader = TRUE;

  protected $zonesCache;
  protected $loaded = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('DNS'),
      'plDescription'   => _('Edit the DNS zones of a system'),
      'plIcon'          => 'geticon.php?context=applications&icon=dns&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=dns&size=16',
      'plObjectType'    => array('server','workstation','terminal','printer','component','phone','mobilePhone'),
      'plPriority'      => 5,
      'plForeignKeys'  => array(
        'fdDNSZoneDn' => 'dnsZone'
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'zones' => array(
        'name'  => _('DNS zones'),
        'attrs' => array(
          new SetAttribute(
            new SelectAttribute(
              '', _('DNS zones for this host'),
              'fdDNSZoneDn', TRUE
            )
          )
        )
      ),
      'soa' => array(
        'name'  => _('SOA records'),
        'attrs' => array()
      ),
      'records' => array(
        'name'  => _('DNS Records'),
        'class' => array('fullwidth'),
        'attrs' => array()
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);
    /* List DNS zones we can read */
    $this->zonesCache = objects::ls('dnsZone', NULL, NULL, '', TRUE);
    /* Check if some values are missing */
    $dnsZones = $this->fdDNSZoneDn;
    foreach ($dnsZones as $dn) {
      if (!isset($this->zonesCache[$dn])) {
        try {
          $zone = objects::ls('dnsZone', NULL, $dn);
          $this->zonesCache = array_merge($zone, $this->zonesCache);
        } catch (FusionDirectoryException $e) {
          continue;
        }
      }
    }
    $counts = array_count_values($this->zonesCache);
    foreach ($this->zonesCache as $dn => &$name) {
      if ($counts[$name] > 1) {
        $name .= ' ('.preg_replace(array('/^[^,]+,/', '/,'.preg_quote($config->current['BASE']).'$/'), '', $dn).')';
      }
    }
    unset($name);
    $this->attributesAccess['fdDNSZoneDn']->attribute->setChoices(array_keys($this->zonesCache), array_values($this->zonesCache));
    $this->attributesAccess['fdDNSZoneDn']->setSubmitForm('zoneChanged');
  }

  function zoneChanged()
  {
    global $config;
    $ldap     = $config->get_ldap_link();
    $dnsZones = $this->fdDNSZoneDn;
    /* Removes references to old attributes */
    if (isset($this->attributesInfo['soa'])) {
      foreach (array_keys($this->attributesInfo['records']['attrs']) as $oldattr) {
        $this->removeAttribute('records', $oldattr);
      }
      foreach (array_keys($this->attributesInfo['soa']['attrs']) as $oldattr) {
        $this->removeAttribute('soa', $oldattr);
      }
    }
    /* Remove sections or fill them */
    if (empty($dnsZones)) {
      unset($this->attributesInfo['soa']);
      unset($this->attributesInfo['records']);
    } else {
      if (is_object($this->parent)) {
        $cn = $this->parent->getBaseObject()->cn;
      } elseif (isset($this->attrs['cn'][0])) {
        /* When called by __construct, parent is not set yet */
        $cn = $this->attrs['cn'][0];
      } else {
        $cn = '';
      }
      $cn = preg_replace('/\$$/', '', $cn);
      $this->attributesInfo['soa'] = array(
        'name'  => _('Primary servers'),
        'attrs' => array()
      );
      $this->attributesInfo['records'] = array(
        'name'  => _('DNS Records'),
        'class' => array('fullwidth'),
        'attrs' => array()
      );
      foreach ($dnsZones as $dn) {
        if (isset($this->zonesCache[$dn])) {
          $name   = $this->zonesCache[$dn];
          $zoneid = preg_replace('/[\/\-,.#:; =]/', '_', $dn);
        } else {
          continue;
        }
        $attrId = 'zoneRecords'.$zoneid;
        $this->addAttribute(
          'records',
          new DnsRecordsFilteredAttribute(
            $name, sprintf(_('The DNS records for zone "%s"'), $name),
            $attrId, $dn,
            array(), 'dnsRecords'
          )
        );

        /* Fetch SOA record */
        $ldap->cat($dn);
        if ($attrs = $ldap->fetch()) {
          list ($fqdn) = explode(' ', $attrs['sOARecord'][0]);
          if ($fqdn == $cn.'.'.$attrs['zoneName'][0]) {
            $fqdn = '<strong>'.$fqdn.'</strong>';
            if (class_available('supportDaemon')) {
              $id = 'ldap2zone_refresh_'.base64_encode($attrs['zoneName'][0]);
              $fqdn .= ' <input type="submit" name="'.$id.'" id="'.$id.'" value="{t}Refresh zone file{/t}"/>';
            }
          }
          $attrId = 'soaRecord'.$zoneid;
          $this->addAttribute(
            'soa',
            new DisplayAttribute(
              $name, sprintf(_('The primary server for zone "%s"'), $name),
              $attrId, FALSE,
              $fqdn
            )
          );
          $this->attributesInfo['soa']['attrs'][$attrId]->setAllowHTML(TRUE);
        }
      }
    }
  }

  function load()
  {
    if (!$this->loaded) {
      $this->zoneChanged();
      $this->loaded = TRUE;
    }
  }

  function execute()
  {
    $this->load();
    $smarty = get_smarty();
    $ui     = get_userinfo();
    $acl    = $ui->get_permissions($this->acl_base, 'dnsZone/dnsZone', 'dnsRecords', $this->read_only);
    $smarty->assign('dnsRecordsACL', $acl);
    return parent::execute();
  }

  function save_object ()
  {
    $this->load();
    parent::save_object();
    if (isset($_POST[get_class($this).'_posted'])) {
      foreach ($_POST as $name => $value) {
        if (preg_match('/^ldap2zone_refresh_(.+)$/', $name, $m)) {
          $zone = base64_decode($m[1]);
          if ($this->parent->by_object['argonautClient']->is_account) {
            $s_daemon = new supportDaemon();
            if ($s_daemon->is_error()) {
              msg_dialog::display(_('Could not run ldap2zone'), msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
              );
            } else {
              $target = array($this->parent->getBaseObject()->macAddress);
              $s_daemon->append_call('Ldap2Zone.start', $target, array('args' => array($zone)));
              if ($s_daemon->is_error()) {
                msg_dialog::display(_('Could not run ldap2zone'), msgPool::siError($s_daemon->get_error()), ERROR_DIALOG);
              } else {
                msg_dialog::display(_('Ldap2zone'), sprintf(_('Ldap2Zone called for zone "%s"'), $zone), INFO_DIALOG);
              }
            }
          } else {
            msg_dialog::display(_('Error'), _('Argonaut client needs to be activated to use ldap2zone remotely'), ERROR_DIALOG);
          }
        }
      }
    }
  }

  function save()
  {
    global $config;
    $errors = parent::save();
    foreach ($this->attributesInfo['records']['attrs'] as &$attr) {
      if ($attr->hasChanged()) {
        $attr->postLdapSave($config->get_ldap_link());
      }
    }
    unset($attr);
    /* Update records if IP or CN changed */
    if ($this->initially_was_account) {
      $baseObject = $this->parent->getBaseObject();
      $messages   = array();
      $oldcn      = preg_replace('/\$$/', '', $baseObject->attributeInitialValue('cn'));
      $oldips     = $baseObject->attributeInitialValue('ipHostNumber');

      $oldzones = $this->attributeInitialValue('fdDNSZoneDn');
      $newzones = $this->attributeValue('fdDNSZoneDn');
      if ($this->attributeHaveChanged('fdDNSZoneDn') && !empty($oldzones)) {
        $messages = array_merge(
          $messages,
          static::removeIpsFromZones(array_diff($oldzones, $newzones), $oldips)
        );
        $messages = array_merge(
          $messages,
          static::removeCnFromZones(array_diff($oldzones, $newzones), $oldcn)
        );
      }

      if ($baseObject->attributeHaveChanged('cn') && !empty($oldcn)) {
        $newcn      = preg_replace('/\$$/', '', $baseObject->attributeValue('cn'));
        $dnsZones   = $this->fdDNSZoneDn;
        foreach ($dnsZones as $dn) {
          $tabObject = objects::open($dn, 'dnsZone');
          $nbRecords = $tabObject->getBaseObject()->cnChanged($oldcn, $newcn);
          if ($nbRecords > 0) {
            $messages[] = sprintf(
              _('%d records were updated from %s to %s in zone %s'),
              $nbRecords,
              $oldcn,
              $newcn,
              $tabObject->getBaseObject()->zoneName
            );
          }
        }
      }

      if ($baseObject->attributeHaveChanged('ipHostNumber')) {
        $newips = $baseObject->attributeValue('ipHostNumber');
        if ((count($oldips) == 1) && (count($newips) == 1)) {
          $dnsZones   = $this->fdDNSZoneDn;
          foreach ($dnsZones as $dn) {
            $tabObject = objects::open($dn, 'dnsZone');
            $nbRecords = $tabObject->getBaseObject()->ipChanged(reset($oldips), reset($newips));
            if ($nbRecords > 0) {
              $messages[] = sprintf(
                _('%d records were updated from %s to %s in zone %s'),
                $nbRecords,
                reset($oldips),
                reset($newips),
                $tabObject->getBaseObject()->zoneName
              );
            }
          }
        } elseif (!empty($oldips)) {
          static::removeIpsFromZone($this->fdDNSZoneDn, array_diff($oldips, $newips));
        }
      }
      if (!empty($messages)) {
        msg_dialog::display(_('DNS update'), implode("<br/>\n", $messages)."<br/>\n", INFO_DIALOG);
      }
    }

    return $errors;
  }

  protected function post_remove ()
  {
    /* Remove records of our IP or CN */
    if ($this->initially_was_account) {
      $baseObject = $this->parent->getBaseObject();
      $messages   = array();
      $oldcn      = preg_replace('/\$$/', '', $baseObject->attributeInitialValue('cn'));
      $oldips     = $baseObject->attributeInitialValue('ipHostNumber');

      $oldzones = $this->attributeInitialValue('fdDNSZoneDn');
      if (!empty($oldzones)) {
        $messages = array_merge(
          $messages,
          static::removeIpsFromZones($oldzones, $oldips)
        );
        $messages = array_merge(
          $messages,
          static::removeCnFromZones($oldzones, $oldcn)
        );
      }
    }
    if (!empty($messages)) {
      msg_dialog::display(_('DNS update'), implode("<br/>\n", $messages)."<br/>\n", INFO_DIALOG);
    }

    return parent::post_remove();
  }

  function adapt_from_template($attrs, $skip = array())
  {
    parent::adapt_from_template($attrs, $skip);
    $this->zoneChanged();
    if (isset($this->attributesInfo['records'])) {
      foreach ($this->attributesInfo['records']['attrs'] as &$attr) {
        $attr->loadAdditionalTemplatesValues();
      }
      unset($attr);
    }
  }

  /* Record example: array('host', 'aRecord', '192.168.1.1', '')
   * Fields are relativeDomainName, record type, record content, reverse zone (for a/aaaa) */
  function addRecord($zoneDn, array $record)
  {
    if (isset($this->zonesCache[$zoneDn])) {
      $zoneid = preg_replace('/[\/\-,.#:; =]/', '_', $zoneDn);
    } else {
      return FALSE;
    }
    $attrId = 'zoneRecords'.$zoneid;
    $this->attributesAccess[$attrId]->addValue($record);
  }

  static function removeIpsFromZones(array $zoneDns, array $ips)
  {
    $messages = array();
    foreach ($zoneDns as $zoneDn) {
      $tabObject = objects::open($zoneDn, 'dnsZone');
      $nbRecords = $tabObject->getBaseObject()->ipRemoved($ips);
      if ($nbRecords > 0) {
        $messages[] = sprintf(
          _('%d records were removed in zone %s'),
          $nbRecords,
          $tabObject->getBaseObject()->zoneName
        );
      }
    }
    return $messages;
  }

  static function removeCnFromZones(array $zoneDns, $cn)
  {
    $messages = array();
    foreach ($zoneDns as $zoneDn) {
      $tabObject = objects::open($zoneDn, 'dnsZone');
      $nbRecords = $tabObject->getBaseObject()->cnRemoved($cn);
      if ($nbRecords > 0) {
        $messages[] = sprintf(
          _('%d records were removed in zone %s'),
          $nbRecords,
          $tabObject->getBaseObject()->zoneName
        );
      }
    }
    return $messages;
  }
}
