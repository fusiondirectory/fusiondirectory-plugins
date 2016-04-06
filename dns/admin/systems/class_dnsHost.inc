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
    if (in_array($values[2], $ips) || ($values[0] == $cn) || preg_match('/'.preg_quote($cn.'.'.$this->getZoneName(), '/').'/', $value[2])) {
      return $values;
    } else {
      return array();
    }
  }

  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
    if ($this->plugin->is_template) {
      $attrs[$this->getLdapName()] = array_map(function ($v) {return implode('|', $v);}, $this->value);
    }
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  /* FIXME - We should lock the zone object and check if we don’t erase changes */
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
      'plObjectType'    => array('server','workstation','terminal','printer','component','winstation','phone','mobilePhone'),
      'plPriority'      => 5,
      'plForeignKeys'  => array(
        'fdDNSZoneDn' => 'dnsZone'
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
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

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->zonesCache = objects::ls('dnsZone');
    $this->attributesAccess['fdDNSZoneDn']->attribute->setChoices(array_keys($this->zonesCache), array_values($this->zonesCache));
    $this->attributesAccess['fdDNSZoneDn']->setSubmitForm('zoneChanged');
  }

  function zoneChanged()
  {
    global $config;
    $ldap     = $config->get_ldap_link();
    $dnsZones = $this->fdDNSZoneDn;
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
          $name = $this->zonesCache[$dn];
        } else {
          continue;
        }
        $attrId = 'zoneRecords'.$name;
        $this->attributesInfo['records']['attrs'][$attrId] = new DnsRecordsFilteredAttribute(
          $name, sprintf(_('The DNS records for zone "%s"'), $name),
          $attrId, $dn,
          array(), 'dnsRecords'
        );
        $this->attributesAccess[$attrId] =& $this->attributesInfo['records']['attrs'][$attrId];
        $this->attributesAccess[$attrId]->setParent($this);
        unset($this->$attrId);

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
          $attrId = 'soaRecord'.$name;
          $this->attributesInfo['soa']['attrs'][$attrId] = new DisplayAttribute(
            $name, sprintf(_('The primary server for zone "%s"'), $name),
            $attrId, FALSE,
            $fqdn
          );
          $this->attributesInfo['soa']['attrs'][$attrId]->setAllowHTML(TRUE);
          $this->attributesAccess[$attrId] =& $this->attributesInfo['soa']['attrs'][$attrId];
          $this->attributesAccess[$attrId]->setParent($this);
          unset($this->$attrId);
        }
      }
    }
    $this->updateAttributesValues();
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
    $smarty     = get_smarty();
    $ui         = get_userinfo();
    $skip_write = $this->read_only;
    $acl        = $ui->get_permissions($this->acl_base, 'dnsZone/'.get_class($this), 'dnsRecords', $this->read_only);
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
              msg_dialog::display(_('Could not get run ldap2zone'), msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
              );
            } else {
              $target = array($this->parent->getBaseObject()->macAddress);
              $res    = $s_daemon->append_call('Ldap2Zone.start', $target, array('args' => array($zone)));
              if ($s_daemon->is_error()) {
                msg_dialog::display(_('Could not get run ldap2zone'), msgPool::siError($s_daemon->get_error()), ERROR_DIALOG);
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
    parent::save();
    foreach ($this->attributesInfo['records']['attrs'] as &$attr) {
      if ($attr->hasChanged()) {
        $attr->postLdapSave($this->config->get_ldap_link());
      }
    }
    unset($attr);
    /* Update records if IP or CN changed */
    if ($this->initially_was_account) {
      $baseObject = $this->parent->getBaseObject();
      $message    = '';
      $oldcn      = preg_replace('/\$$/', '', $baseObject->attributeInitialValue('cn'));
      if ($baseObject->attributeHaveChanged('cn') && !empty($oldcn)) {
        $newcn      = preg_replace('/\$$/', '', $baseObject->attributeValue('cn'));
        $dnsZones   = $this->fdDNSZoneDn;
        foreach ($dnsZones as $dn) {
          $tabObject = objects::open($dn, 'dnsZone');
          $nbRecords = $tabObject->getBaseObject()->cnChanged($oldcn, $newcn);
          if ($nbRecords > 0) {
            $message .= sprintf(
              _('%d records were updated from %s to %s in zone %s')."<br/>\n",
              $nbRecords,
              $oldcn,
              $newcn,
              $tabObject->getBaseObject()->zoneName
            );
          }
        }
      }

      if ($baseObject->attributeHaveChanged('ipHostNumber')) {
        $oldips = $baseObject->attributeInitialValue('ipHostNumber');
        $newips = $baseObject->attributeValue('ipHostNumber');
        if ((count($oldips) == 1) && (count($newips) == 1)) {
          $dnsZones   = $this->fdDNSZoneDn;
          foreach ($dnsZones as $dn) {
            $tabObject = objects::open($dn, 'dnsZone');
            $nbRecords = $tabObject->getBaseObject()->ipChanged(reset($oldips), reset($newips));
            if ($nbRecords > 0) {
              $message .= sprintf(
                _('%d records were updated from %s to %s in zone %s')."<br/>\n",
                $nbRecords,
                reset($oldips),
                reset($newips),
                $tabObject->getBaseObject()->zoneName
              );
            }
          }
        } else {
          $dnsZones   = $this->fdDNSZoneDn;
          foreach ($dnsZones as $dn) {
            $tabObject = objects::open($dn, 'dnsZone');
            $nbRecords = $tabObject->getBaseObject()->ipRemoved(array_diff($oldips, $newips));
            if ($nbRecords > 0) {
              $message .= sprintf(
                _('%d records were removed in zone %s')."<br/>\n",
                $nbRecords,
                $tabObject->getBaseObject()->zoneName
              );
            }
          }
        }
      }
      if (!empty($message)) {
        msg_dialog::display(_('DNS update'), $message, INFO_DIALOG);
      }
    }
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
}
