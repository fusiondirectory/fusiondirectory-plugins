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
          new HiddenAttribute('inReverse')
          //~ new BooleanAttribute(
            //~ _('PTR record'), _('Should a PTR record be created in the reverse zone for this record'),
            //~ 'ptrRecord', FALSE, TRUE
          //~ )
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL, $value = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->attributesAccess['relativeSubdomainName']->setInLdap(FALSE);
    $this->attributesAccess['dnsRecord']->setInLdap(FALSE);
    $this->attributesAccess['dnsRecord']->setLinearRendering(FALSE);
    if (!empty($value)) {
      $this->relativeSubdomainName = array_shift($value);
      if (isset($value[2])) {
        $this->inReverse = $value[2];
        unset($value[2]);
      }
      $this->dnsRecord = $value;
    }
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
    $this->dialog     = new $this->dialogClass($simplePlugin->config, $simplePlugin->dn, $simplePlugin, $value);
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
  protected $dialogClass = 'DnsRecordDialog';

  protected function getAttributeArrayValue($key, $value)
  {
    return array(
      $value[0],
      DnsRecordAttribute::$types[$value[1]],
      $value[2],
    );
  }

  protected function loadAttrValue ($attrs)
  {
    global $config;

    $this->value = array();
    foreach (array_keys(DnsRecordAttribute::$types) as $type) {
      if (isset($attrs[$type]['count'])) {
        for ($i = 0; $i < $attrs[$type]['count']; $i++) {
          $this->value[] = array('', $type, $attrs[$type][$i], FALSE);
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
            $this->value[] = array($subattrs['relativeDomainName'][0], $type, $subattrs[$type][$j], FALSE);
          }
        }
      }
      /* Get reverse zone
      $ldap->ls('(&(objectClass=dNSZone)(relativeDomainName=@)(zoneName=*))', $attrs['dn'], array('zoneName'));
      $reverseDn = ;
      while ($attrs = ldap->fetch()) {
        $Zones[$zoneName]['ReverseZone'] = DNS::FlipIp(preg_replace('/\.in\-addr\.arpa\.?$/', '', $tmp['zoneName'][0]));
      }*/
    }
  }

  function fillLdapValue (&$attrs)
  {
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
      $ldap->rmdir($dn);
    }
    /* Then add our values */
    $nodes = array();
    foreach ($this->value as $line) {
      list ($domain, $type, $content) = $line;
      if (empty($domain)) {
        /* ignore root records */
        continue;
      }
      if (!isset($nodes[$domain])) {
        $nodes[$domain] = array(
          'objectClass'         => array('dNSZone'),
          'relativeDomainName'  => $domain,
          'zoneName'            => $zoneName,
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
  }

  function cnChanged($oldcn, $newcn) {
    $nb = 0;
    foreach ($this->value as &$row) {
      if ($row[0] == $oldcn) {
        $row[0] = $newcn;
        $nb++;
      }
    }
    return $nb;
  }

  function ipChanged($oldip, $newip) {
    $nb = 0;
    foreach ($this->value as &$row) {
      if ($row[2] == $oldip) {
        $row[2] = $newip;
        $nb++;
      }
    }
    return $nb;
  }

  function ipRemoved($oldips) {
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

class dnsZone extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('dNSZone');

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS zone'),
      'plDescription' => _('DNS zone'),
      'plObjectType'  => array('dnsZone' => array(
        'name'        => _('DNS zone'),
        'filter'      => '(&(objectClass=dNSZone)(relativeDomainName=@))',
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
          new HostNameAttribute (
            _('Zone name'), _('Zone name'),
            'zoneName', TRUE
          ),
          new HiddenAttribute (
            //~ _('Relative domain name'), _('Relative domain name'),
            'relativeDomainName', TRUE,
            '@'
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
              new IntAttribute(
                _('Serial number'), _('Version number of the original copy of the zone'),
                'soaRecord_serial', TRUE,
                0, FALSE
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
