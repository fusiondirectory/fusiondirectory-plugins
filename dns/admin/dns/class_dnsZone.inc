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

class DnsRecordPlugin extends simplePlugin
{
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Record'),
        'attrs' => array(
          new StringAttribute (
            _('Subdomain'), _('Relative subdomain name'),
            'relativeSubdomainName', FALSE
          ),
          new DnsRecordAttribute (
            _('Record'), _('DNS Record'),
            'dnsRecord', TRUE
          ),
        )
      ),
    );
  }

  /*
   * $value can be an array containing:
   * relativeSubdomainName, recordType, recordValue, reverseZone
   * */
  function __construct ($parent, $attribute, $value = NULL)
  {
    global $config;
    parent::__construct($config, $parent->dn, $parent);

    $this->attributesAccess['relativeSubdomainName']->setInLdap(FALSE);
    $this->attributesAccess['dnsRecord']->setInLdap(FALSE);
    $this->attributesAccess['dnsRecord']->setLinearRendering(FALSE);
    if (!empty($value)) {
      $this->relativeSubdomainName  = array_shift($value);
      $this->dnsRecord              = $value;
    } elseif (!($parent instanceof dnsZone)) {
      /* Read default values from system: cn and IPs */
      $this->relativeSubdomainName = $parent->parent->getBaseObject()->cn;
      $ips = $parent->parent->getBaseObject()->ipHostNumber;
      $this->attributesAccess['dnsRecord']->setParentIPs(
        array_filter($ips, 'tests::is_ipv4'),
        array_filter($ips, 'tests::is_ipv6')
      );
    }
    $this->attributesAccess['dnsRecord']->attributes[2]->setChoices($attribute->getReverseZones());
  }

  function save()
  {
  }
}

class DnsRecordDialog extends GenericDialog
{
  protected $dialogClass = 'DnsRecordPlugin';

  protected $initialDialogValue = NULL;

  protected $plugin;

  function __construct($simplePlugin, &$attribute, $value = array())
  {
    $this->attribute  = $attribute;
    $this->plugin     = $simplePlugin;
    $this->dialog     = new $this->dialogClass($simplePlugin, $attribute, $value);
    if (!empty($value)) {
      $this->initialDialogValue = $value;
    }
  }

  function dialog_execute ()
  {
    $this->dialog->save_object();
    return $this->dialog->execute()."\n".
    '<p class="plugbottom">'."\n".
    '  <input type="submit" name="add_finish" value="'.msgPool::addButton().'"/>&nbsp;'."\n".
    '  <input type="submit" name="add_cancel" value="'.msgPool::cancelButton().'"/>'."\n".
    '</p>';
  }

  function handle_finish ()
  {
    $this->dialog->save_object();

    /* Check for errors */
    if (!$this->plugin->is_template) {
      $checks = $this->dialog->check();
      if (count($checks)) {
        msg_dialog::displayChecks($checks);
        return $this->dialog_execute();
      }
    }
    /* add new/edited zone */
    $ret = array_merge(array($this->dialog->relativeSubdomainName), $this->dialog->dnsRecord);
    $this->attribute->addValue($ret);
    return FALSE;
  }

  function handle_cancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addValue($this->initialDialogValue);
    }
    return FALSE;
  }
}

class DnsRecordsAttribute extends DialogOrderedArrayAttribute
{
  protected $dialogClass  = 'DnsRecordDialog';
  protected $reverseZones = array();
  protected $sOARecord    = '';

  function getReverseZones()
  {
    if ($this->plugin instanceof dnsZone) {
      return $this->plugin->reverseZones;
    } else {
      return $this->reverseZones;
    }
  }

  protected function getAttributeArrayValue($key, $value)
  {
    return array(
      $value[0],
      DnsRecordAttribute::$types[$value[1]],
      $value[2],
      $value[3]
    );
  }

  protected function loadAttrValue ($attrs)
  {
    global $config;

    $this->value = array();
    $aRecords = array(
      'aRecord'     => array(),
      'aAAARecord'  => array()
    );
    foreach (array_keys(DnsRecordAttribute::$types) as $type) {
      if (isset($attrs[$type]['count'])) {
        for ($i = 0; $i < $attrs[$type]['count']; $i++) {
          $this->value[] = array('', $type, $attrs[$type][$i], '');
        }
        if (in_array($type, DnsRecordAttribute::$reverseTypes)) {
          end($this->value);
          $aRecords[$type]['@'] = key($this->value);
        }
      }
    }
    if (isset($attrs['dn'])) {
      $ldap = $config->get_ldap_link();
      $ldap->ls('objectClass=dNSZone', $attrs['dn'], array_merge(array('relativeDomainName'), array_keys(DnsRecordAttribute::$types)));
      while ($subattrs = $ldap->fetch()) {
        for ($i = 0; $i < $subattrs['count']; $i++) {
          $type = $subattrs[$i];
          if ($type == 'relativeDomainName') {
            continue;
          }
          for ($j = 0; $j < $subattrs[$type]['count']; $j++) {
            $this->value[] = array($subattrs['relativeDomainName'][0], $type, $subattrs[$type][$j], '');
          }
          if (in_array($type, DnsRecordAttribute::$reverseTypes)) {
            end($this->value);
            $aRecords[$type][$subattrs['relativeDomainName'][0]] = key($this->value);
          }
        }
      }
      /* Get reverse zone */
      $ldap->ls('(&(objectClass=dNSZone)(relativeDomainName=@)(zoneName=*))', $attrs['dn'], array('zoneName'));
      $reverseZones = array();
      while ($subattrs = $ldap->fetch()) {
        $reverseZones[$subattrs['dn']] = $subattrs['zoneName'][0];
      }
      if ($this->plugin instanceof dnsZone) {
        $this->plugin->reverseZones = array_values($reverseZones);
        $this->plugin->attributesAccess['reverseZones']->setInitialValue($this->plugin->reverseZones);
      } else {
        $this->reverseZones = array_values($reverseZones);
        $this->sOARecord    = $attrs['sOARecord'][0];
      }
      foreach ($reverseZones as $reverseZoneDn => $reverseZoneName) {
        $ipv6 = preg_match('/ip6/', $reverseZoneName);
        $ldap->ls('(objectClass=dNSZone)', $reverseZoneDn, array('relativeDomainName', 'pTRRecord'));
        while ($subattrs = $ldap->fetch()) {
          if ($subattrs['pTRRecord'][0] == $attrs['zoneName'][0]) {
            $name = '@';
          } else {
            $name = preg_replace('/\.'.preg_quote($attrs['zoneName'][0]).'$/', '', $subattrs['pTRRecord'][0]);
          }
          if ($ipv6 && isset($aRecords['aAAARecord'][$name])) {
            $this->value[$aRecords['aAAARecord'][$name]][3] = $reverseZoneName;
          } elseif (!$ipv6 && isset($aRecords['aRecord'][$name])) {
            $this->value[$aRecords['aRecord'][$name]][3] = $reverseZoneName;
          }
        }
      }
    }
  }

  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
    foreach ($this->value as $line) {
      list ($domain, $type, $content) = $line;
      // Only save root records here
      if (empty($domain)) {
        if (!isset($attrs[$type])) {
          $attrs[$type] = array();
        }
        $attrs[$type][] = $content;
      }
    }
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap, $zoneDn = NULL, $zoneName = NULL)
  {
    if ($zoneDn === NULL) {
      $zoneDn = $this->plugin->dn;
    }
    if ($zoneName === NULL) {
      $zoneName = $this->plugin->zoneName;
    }
    /* First delete all old nodes */
    $ldap->ls('objectClass=dNSZone', $zoneDn, array('dn'));
    $delete = array();
    while ($attrs = $ldap->fetch()) {
      $delete[] = $attrs['dn'];
    }
    foreach ($delete as $dn) {
      $ldap->rmdir_recursive($dn);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
      }
    }
    /* Then add our values */
    $nodes  = array();
    $ptrs   = array();
    $nsRecords = array();
    foreach ($this->value as $line) {
      list ($domain, $type, $content, $reverse) = $line;
      if (!empty($reverse)) {
        if (!isset($ptrs[$reverse])) {
          $ptrs[$reverse] = array();
        }
        $ptrs[$reverse][$content] = $domain;
      }
      if (empty($domain)) {
        if ($type == 'nSRecord') {
          $nsRecords[] = $content;
        }
        /* ignore root records */
        continue;
      }
      if (!isset($nodes[$domain])) {
        $nodes[$domain] = array(
          'objectClass'         => array('dNSZone'),
          'relativeDomainName'  => $domain,
          'zoneName'            => $zoneName,
          'dNSClass'            => 'IN',
        );
      }
      if (!isset($nodes[$domain][$type])) {
        $nodes[$domain][$type] = array();
      }
      $nodes[$domain][$type][] = $content;
    }
    foreach ($nodes as $node) {
      $dn = 'relativeDomainName='.$node['relativeDomainName'].','.$zoneDn;
      $ldap->cd($dn);
      $ldap->add($node);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
      }
    }

    if ($this->plugin instanceof dnsZone) {
      $this->reverseZones = $this->plugin->reverseZones;
      $this->sOARecord    = $this->plugin->sOARecord;
    }
    foreach ($this->reverseZones as $reverseZone) {
      /* Write this reverse under the zone dn */
      $reverseDn = 'zoneName='.$reverseZone.','.$zoneDn;
      /* Copy NS records and SOA from zone */
      $node = array(
        'objectClass'         => array('dNSZone'),
        'zoneName'            => $reverseZone,
        'relativeDomainName'  => '@',
        'dNSClass'            => 'IN',
        'sOARecord'           => $this->sOARecord,
      );
      if (!empty($nsRecords)) {
        $node['nSRecord'] = $nsRecords;
      }
      $ldap->cd($reverseDn);
      $ldap->add($node);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $reverseDn, LDAP_ADD, get_class()), LDAP_ERROR);
      }
      /* Write each needed PTR */
      $mask = preg_replace('/\.(in-addr|ip6)\.arpa\.?$/i', '', $reverseZone);
      if (isset($ptrs[$reverseZone])) {
        $ipv6 = preg_match('/ip6/', $reverseZone);
        foreach ($ptrs[$reverseZone] as $ip => $name) {
          if ($ipv6) {
            $relativeDomainName = preg_replace('/\.'.preg_quote($mask).'$/', '', implode('.', array_reverse(preg_split('/:|/', expandIPv6($ip), NULL, PREG_SPLIT_NO_EMPTY))));
          } else {
            $relativeDomainName = preg_replace('/\.'.preg_quote($mask).'$/', '', implode('.', array_reverse(explode('.', $ip))));
          }
          $dn = 'relativeDomainName='.$relativeDomainName.','.$reverseDn;
          $ldap->cd($dn);
          $ldap->add(
            array(
              'objectClass'         => array('dNSZone'),
              'zoneName'            => $reverseZone,
              'dNSClass'            => 'IN',
              'pTRRecord'           => (empty($name) ? '' : $name.'.').$zoneName,
              'relativeDomainName'  => $relativeDomainName,
            )
          );
        }
      }
    }
  }

  function cnChanged($oldcn, $newcn)
  {
    $nb = 0;
    foreach ($this->value as &$row) {
      if ($row[0] == $oldcn) {
        $row[0] = $newcn;
        $nb++;
      }
    }
    unset($row);
    return $nb;
  }

  function ipChanged($oldip, $newip)
  {
    $nb = 0;
    foreach ($this->value as &$row) {
      if ($row[2] == $oldip) {
        $row[2] = $newip;
        $nb++;
      }
    }
    unset($row);
    return $nb;
  }

  function ipRemoved($oldips)
  {
    $nb = 0;
    foreach ($this->value as $key => $row) {
      if (in_array($row[2], $oldips)) {
        unset($this->value[$key]);
        $nb++;
      }
    }
    return $nb;
  }
}

class ZoneNameAttribute extends StringAttribute
{
  function validate ()
  {
    if (dnsZone::finalDot()) {
      $regexp = '/^(([a-z0-9\-]{2,63})\.)+$/';
      $error  = _('"%s" must contain a domain name and end with a final dot.<br/><br/>Example: example.com.');
    } else {
      $regexp = '/^(([a-z0-9\-]{2,63})\.)*[a-z]{2,63}$/';
      $error  = _('"%s" must contain a domain name.<br/><br/>Example: example.com');
    }
    if (!preg_match($regexp, $this->value)) {
      return sprintf($error, $this->getLabel());
    }
  }
}

class dnsZone extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('dNSZone');

  public static function finalDot()
  {
    global $config;
    return ($config->get_cfg_value('DNSFinalDot', 'TRUE') == 'TRUE');
  }

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS zone'),
      'plDescription' => _('DNS zone'),
      'plObjectType'  => array('dnsZone' => array(
        'name'        => _('DNS zone'),
        'filter'      => '(&(objectClass=dNSZone)(relativeDomainName=@)(!(|(zoneName=*.arpa)(zoneName=*.arpa.))))',
        'ou'          => 'ou=dns,',
        'icon'        => 'geticon.php?context=applications&icon=dns&size=16',
        'mainAttr'    => 'zoneName'
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Zone'),
        'attrs' => array(
          new BaseSelectorAttribute('ou=dns,'),
          new ZoneNameAttribute (
            _('Zone name'), _('Zone name'),
            'zoneName', TRUE
          ),
          new HiddenAttribute (
            //~ _('Relative domain name'), _('Relative domain name'),
            'relativeDomainName', TRUE,
            '@'
          ),
          new SetAttribute (
            new StringAttribute (
              _('Reverse zones'), sprintf(_('Reverse zones for this zone in the form xx.xx.in-addr.arpa%1$s or x.x.ip6.arpa%1$s'), (dnsZone::finalDot() ? '.' : '')),
              'reverseZones', FALSE,
              '', '',
              '/^.*\.(in-addr|ip6)\.arpa'.(dnsZone::finalDot() ? '\.' : '').'$/i', '11.168.192.in-addr.arpa'.(dnsZone::finalDot() ? '.' : '')
            )
          ),
          new HiddenAttribute (
            'dNSClass', TRUE,
            'IN'
          ),
        )
      ),
      'soa' => array(
        'name'  => _('SOA record'),
        'attrs' => array(
          new CompositeAttribute (
            _('SOA Record'),
            'sOARecord',
            array(
              new StringAttribute(
                _('Primary DNS server'), _('Domain name of the name server that was the original or primary source of data for this zone'),
                'soaRecord_primary', TRUE
              ),
              new StringAttribute(
                _('Mail address'), _('Domain name which specifies the mailbox of the person responsible for this zone'),
                'soaRecord_mail', TRUE
              ),
              new DisplayLDAPAttribute(
                _('Serial number'), _('Version number of the original copy of the zone'),
                'soaRecord_serial', TRUE,
                0, FALSE, date('Ymd').'01'
              ),
              new TimeAttribute(
                _('Refresh'), _('Time interval before the zone should be refreshed'),
                'soaRecord_refresh', TRUE,
                0, FALSE, 3600
              ),
              new TimeAttribute(
                _('Retry'), _('Time interval that should elapse before a failed refresh should be retried'),
                'soaRecord_retry', TRUE,
                0, FALSE, 1800
              ),
              new TimeAttribute(
                _('Expire'), _('Time value that specifies the upper limit on the time interval that can elapse before the zone is no longer authoritative'),
                'soaRecord_expire', TRUE,
                0, FALSE, 720000
              ),
              new TimeAttribute(
                _('TTL'), _('Minimum TTL field that should be exported with any RR from this zone'),
                'soaRecord_ttl', TRUE,
                0, FALSE, 6400
              ),
            ),
            '/^(\S*) (\S*) (\S*) (\S*) (\S*) (\S*) (\S*)$/',
            '%s %s %s %s %s %s %s'
          ),
        )
      ),
      'records' => array(
        'name'  => _('Records'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new DnsRecordsAttribute(
            '', _('The DNS records for this zone'),
            'dnsRecords', FALSE
          )
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesAccess['reverseZones']->setInLdap(FALSE);
    $this->updateAttributesValues();
  }

  function prepare_save ()
  {
    if (
      $this->attributesAccess['zoneName']->hasChanged() ||
      $this->attributesAccess['reverseZones']->hasChanged() ||
      $this->attributesAccess['sOARecord']->hasChanged() ||
      $this->attributesAccess['dnsRecords']->hasChanged()
      ) {
      $serial = $this->attributesAccess['sOARecord']->attributes[2]->getValue();
      if (preg_match('/^'.date('Ymd').'/', $serial)) {
        $serial++;
      } else {
        $serial = date('Ymd').'01';
      }
      $this->attributesAccess['sOARecord']->attributes[2]->setValue($serial);
    }
    parent::prepare_save();
  }

  function ldap_save ($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);
    $this->attributesAccess['dnsRecords']->postLdapSave($this->config->get_ldap_link());
  }

  function cnChanged ($oldcn, $newcn)
  {
    $nb = $this->attributesAccess['dnsRecords']->cnChanged($oldcn, $newcn);
    if ($nb > 0) {
      $this->save();
    }
    return $nb;
  }

  function ipChanged ($oldip, $newip)
  {
    $nb = $this->attributesAccess['dnsRecords']->ipChanged($oldip, $newip);
    if ($nb > 0) {
      $this->save();
    }
    return $nb;
  }

  function ipRemoved ($oldips)
  {
    $nb = $this->attributesAccess['dnsRecords']->ipRemoved($oldips);
    if ($nb > 0) {
      $this->save();
    }
    return $nb;
  }
}
