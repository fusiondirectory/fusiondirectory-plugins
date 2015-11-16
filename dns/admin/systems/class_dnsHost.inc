<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015  FusionDirectory

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
  protected $zoneDn;

  function __construct ($label, $description, $ldapName, $zoneDn, $defaultValue = array(), $acl = "")
  {
    parent::__construct ($label, $description, $ldapName, FALSE, $defaultValue, $acl);
    $this->zoneDn = $zoneDn;
    $this->loadRecords();
  }

  protected function loadAttrValue ($attrs)
  {
  }

  function loadRecords ()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cat($this->zoneDn);
    $attrs = $ldap->fetch();
    parent::loadAttrValue($attrs);
  }

  protected function getAttributeArrayValue($key, $value)
  {
    $values = parent::getAttributeArrayValue($key, $value);
    $ips    = $this->plugin->parent->getBaseObject()->ipHostNumber;
    $cn     = $this->plugin->parent->getBaseObject()->cn;
    if (in_array($values[2], $ips) || ($values[0] == $cn)) {
      return $values;
    } else {
      return array();
    }
  }

  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  /* FIXME - We should lock the zone object */
  function postLdapSave ($ldap, $zoneDn = NULL, $zoneName = NULL)
  {
    $attrs = array();
    parent::fillLdapValue($attrs);
    if (!empty($attrs)) {
      $ldap->cd($this->zoneDn);
      $ldap->modify($attrs);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $this->zoneDn, LDAP_ADD, get_class()), LDAP_ERROR);
      }
    }
    $ldap->cat($this->zoneDn, array('zoneName'));
    $attrs = $ldap->fetch();
    parent::postLdapSave($ldap, $this->zoneDn, $attrs['zoneName'][0]);
  }
}

class dnsHost extends simplePlugin
{
  var $objectclasses = array('fdDNSHost');
  var $displayHeader = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('DNS'),
      'plDescription'   => _('Edit the DNS zones of a system'),
      'plIcon'          => 'geticon.php?context=applications&icon=dns&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=dns&size=16',
      'plObjectType'    => array('server', 'workstation'),
      'plPriority'      => 5,

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
              '', _('DNS zones for this host'),
              'fdDNSZoneDn', TRUE
            )
          )
        )
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
    $zones = objects::ls('dnsZone');
    $this->attributesAccess['fdDNSZoneDn']->attribute->setChoices(array_keys($zones), array_values($zones));
    $this->attributesAccess['fdDNSZoneDn']->setSubmitForm('zoneChanged');
    $this->zoneChanged();
  }

  function zoneChanged()
  {
    $dnsZones   = $this->fdDNSZoneDn;
    $this->attributesInfo['records']['attrs'] = array();
    foreach($dnsZones as $dn) {
      $name = $this->attributesAccess['fdDNSZoneDn']->attribute->displayValue($dn);
      $attrId = 'zoneRecords'.$name;
      $this->attributesInfo['records']['attrs'][$attrId] = new DnsRecordsFilteredAttribute(
        '', sprintf(_('The DNS records for zone "%s"'), $name),
        $attrId, $dn
      );
      $this->attributesAccess[$attrId] =& $this->attributesInfo['records']['attrs'][$attrId];
      $this->attributesAccess[$attrId]->setParent($this);
      unset($this->$attrId);
    }
    $this->updateAttributesValues();
  }

  function save()
  {
    parent::save();
    foreach ($this->attributesInfo['records']['attrs'] as &$attr) {
      $attr->postLdapSave($this->config->get_ldap_link());
    }
    $baseObject = $this->parent->getBaseObject();
    $message    = '';
    if ($baseObject->attributesAccess['cn']->hasChanged()) {
      $oldcn      = $baseObject->attributesAccess['cn']->getInitialValue();
      $newcn      = $baseObject->attributesAccess['cn']->getValue();
      $dnsZones   = $this->fdDNSZoneDn;
      foreach($dnsZones as $dn) {
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
    if ($baseObject->attributesAccess['ipHostNumber']->hasChanged()) {
      $oldips = $baseObject->attributesAccess['ipHostNumber']->getInitialValue();
      $newips = $baseObject->attributesAccess['ipHostNumber']->getValue();
      if ((count($oldips) == 1) && (count($newips) == 1)) {
        $dnsZones   = $this->fdDNSZoneDn;
        foreach($dnsZones as $dn) {
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
        foreach($dnsZones as $dn) {
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
