<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2015-2020  FusionDirectory

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
  public static function plInfo (): array
  {
    $plProvidedAcls = parent::generatePlProvidedAcls(static::getAttributesInfo());

    $plProvidedAcls['dnsRecord'] = _('DNS Record domain and type');

    foreach (DnsRecordAttribute::$types as $id => $name) {
      $plProvidedAcls['dnsRecord_'.$id] = sprintf(_('%s record'), $name);
    }

    return [
      'plShortName'     => _('DNS record'),
      'plDescription'   => _('DNS record'),
      'plCategory'      => ['dnsZone'],
      'plProvidedAcls'  => $plProvidedAcls
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Record'),
        'class' => ['fullwidth'],
        'attrs' => [
          new StringAttribute(
            _('Subdomain'), _('Relative subdomain name'),
            'relativeSubdomainName', FALSE,
            '', 'dnsRecord'
          ),
          new DnsRecordAttribute(
            _('Record'), _('DNS Record'),
            'dnsRecord', TRUE
          ),
        ]
      ],
    ];
  }

  /*
   * $value can be an array containing:
   * relativeSubdomainName, recordType, recordValue, reverseZone
   * */
  function __construct ($parent, $attribute, $value = NULL)
  {
    parent::__construct($parent->dn, $parent);

    $this->attributesAccess['relativeSubdomainName']->setInLdap(FALSE);
    $this->attributesAccess['dnsRecord']->setInLdap(FALSE);
    $this->attributesAccess['dnsRecord']->setLinearRendering(FALSE);

    $types = DnsRecordAttribute::$types;
    foreach ($types as $id => $name) {
      if (!preg_match('/w/', $this->aclGetPermissions('dnsRecord_'.$id, NULL, $this->acl_skip_write()))) {
        unset($types[$id]);
      }
    }
    $this->attributesAccess['dnsRecord']->attributes[0]->setChoices(array_keys($types), array_values($types));

    if (!empty($value)) {
      $this->relativeSubdomainName  = array_shift($value);
      if ($value[0] === 'tXTRecord') {
        if (preg_match('/^v=spf1 (.*)$/', $value[1], $m)) {
          /* Convert TXT to SPF */
          $value = [
            'sPFRecord',
            $m[1],
            $value[2],
          ];
        } elseif (preg_match('/^v=DKIM1;(.*)$/', $value[1], $m)) {
          /* Convert TXT to DKIM */
          $value = [
            'dKIMRecord',
            $m[1],
            $value[2],
          ];
        } elseif (preg_match('/^v=DMARC1;(.*)$/', $value[1], $m)) {
          /* Convert TXT to DMARC */
          $value = [
            'dMARCRecord',
            $m[1],
            $value[2],
          ];
        }
      }
      $this->dnsRecord              = $value;
    } elseif (!($parent instanceof dnsZone)) {
      /* Read default values from system: cn and IPs */
      $this->relativeSubdomainName = preg_replace('/\$$/', '', $parent->parent->getBaseObject()->cn);
      $ips = $parent->parent->getBaseObject()->ipHostNumber;
      $this->attributesAccess['dnsRecord']->setParentIPs(
        array_filter($ips, 'tests::is_ipv4'),
        array_filter($ips, 'tests::is_ipv6'),
        $this->relativeSubdomainName.'.'.$attribute->getZoneName()
      );
    }
    $this->attributesAccess['dnsRecord']->attributes[2]->setChoices($attribute->getReverseZones());
  }

  /*!
   * \brief Get record value for DnsRecordsAttribute instance
   *
   * \return array containing:
   * relativeSubdomainName, recordType, recordValue, reverseZone
   */
  public function getRecordValue (): array
  {
    $relativeSubdomainName = $this->relativeSubdomainName;
    if ($relativeSubdomainName == '@') {
      $relativeSubdomainName = '';
    }
    $record = $this->attributesAccess['dnsRecord']->getValue();
    if ($record[0] == 'sPFRecord') {
      /* Convert SPF to TXT */
      $record = [
        'tXTRecord',
        'v=spf1 '.$record[1],
        $record[2],
      ];
    } elseif ($record[0] == 'dKIMRecord') {
      /* Convert DKIM to TXT */
      $record = [
        'tXTRecord',
        'v=DKIM1; '.$record[1],
        $record[2],
      ];
    } elseif ($record[0] == 'dMARCRecord') {
      /* Convert DMARC to TXT */
      $record = [
        'tXTRecord',
        'v=DMARC1; '.$record[1],
        $record[2],
      ];
    }
    return array_merge([$relativeSubdomainName], $record);
  }

  function save (): array
  {
    return [];
  }

  public function render (): string
  {
    $smarty = get_smarty();
    $attr   = $this->attributesAccess['dnsRecord'];
    foreach ($this->attributesAccess as $attr) {
      $smarty->assign($attr->getAcl().'ACL', $this->aclGetPermissions($attr->getAcl(), NULL, $this->acl_skip_write()));
    }
    return parent::render();
  }
}

class DnsRecordDialog extends GenericDialog
{
  protected $dialogClass = 'DnsRecordPlugin';

  protected $initialDialogValue = NULL;

  protected $plugin;

  function __construct ($simplePlugin, $attribute, $value = [])
  {
    $this->attribute  = $attribute;
    $this->plugin     = $simplePlugin;
    $this->dialog     = new $this->dialogClass($simplePlugin, $attribute, $value);
    if (!empty($value)) {
      $this->initialDialogValue = $value;
    }
  }

  public function render (): string
  {
    return $this->dialog->render()."\n".
    '<p class="plugbottom">'."\n".
    '  <input type="submit" name="add_finish" value="'.msgPool::addButton().'"/>&nbsp;'."\n".
    '  <input type="submit" formnovalidate="formnovalidate" name="add_cancel" value="'.msgPool::cancelButton().'"/>'."\n".
    '</p>';
  }

  protected function handleFinish ()
  {
    $this->dialog->update();

    /* Check for errors */
    if (!$this->plugin->is_template) {
      $checks = $this->dialog->check();
      if (count($checks)) {
        msg_dialog::displayChecks($checks);
        return;
      }
    }
    /* Add new/edited record */
    $this->attribute->addValue('', $this->dialog->getRecordValue());
    unset($this->dialog);
  }

  protected function handleCancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addValue('', $this->initialDialogValue);
    }
    unset($this->dialog);
  }
}

class DnsRecordsAttribute extends DialogOrderedArrayAttribute
{
  protected $height       = 400;
  protected $dialogClass  = 'DnsRecordDialog';
  protected $reverseZones = [];
  protected $zoneName;
  protected $zoneDn;
  protected $initialReverseZones;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setHeaders([
      _('Subdomain'),
      _('Type'),
      _('Content'),
      _('Reverse'),
      ''
    ]);
  }

  function getZoneName ()
  {
    if ($this->plugin instanceof dnsZone) {
      return $this->plugin->zoneName;
    } else {
      return $this->zoneName;
    }
  }

  function getZoneDn ()
  {
    if ($this->plugin instanceof dnsZone) {
      return $this->plugin->dn;
    } else {
      return $this->zoneDn;
    }
  }

  function getReverseZones ()
  {
    if ($this->plugin instanceof dnsZone) {
      return $this->plugin->reverseZones;
    } else {
      return $this->reverseZones;
    }
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    return [
      $value[0],
      DnsRecordAttribute::$types[$value[1]],
      $value[2],
      $value[3]
    ];
  }

  protected function loadReverseZones ($attrs)
  {
    global $config;

    /* Get reverse zones */
    $ldap = $config->get_ldap_link();
    $ldap->cd($attrs['dn']);
    $ldap->search('(&(objectClass=dNSZone)(relativeDomainName=@)(zoneName=*))', ['zoneName','nSRecord','sOARecord'], 'one');
    $reverseZones = [];
    while ($subattrs = $ldap->fetch()) {
      $reverseZones[$subattrs['dn']] = $subattrs;
    }
    $zoneNames = array_values(array_map(
      function ($subattrs)
      {
        return $subattrs['zoneName'][0];
      },
      $reverseZones
    ));
    if ($this->plugin instanceof dnsZone) {
      $this->plugin->reverseZones = $zoneNames;
      $this->plugin->attributesAccess['reverseZones']->setInitialValue($this->plugin->reverseZones);
    } else {
      $this->reverseZones = $zoneNames;
    }

    return $reverseZones;
  }

  protected function loadAttrValue (array $attrs)
  {
    global $config;

    $this->value = [];
    $aRecords = [
      'aRecord'     => [],
      'aAAARecord'  => []
    ];
    foreach (DnsRecordAttribute::getLdapAttributesList() as $type) {
      if (isset($attrs[$type]['count'])) {
        for ($i = 0; $i < $attrs[$type]['count']; $i++) {
          $this->value[] = ['', $type, $attrs[$type][$i], ''];
          if (in_array($type, DnsRecordAttribute::$reverseTypes)) {
            end($this->value);
            $ip = $attrs[$type][$i];
            if ($type == 'aAAARecord') {
              $ip = expandIPv6($ip);
            }
            $aRecords[$type]['@'.$ip] = key($this->value);
          }
        }
      }
    }
    if (isset($attrs['dn'])) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($attrs['dn']);
      $ldap->search('(&(objectClass=dNSZone)(zoneName='.$attrs['zoneName'][0].'))', array_merge(['relativeDomainName'], DnsRecordAttribute::getLdapAttributesList()), 'one');
      while ($subattrs = $ldap->fetch()) {
        for ($i = 0; $i < $subattrs['count']; $i++) {
          $type = $subattrs[$i];
          if ($type == 'relativeDomainName') {
            continue;
          }
          for ($j = 0; $j < $subattrs[$type]['count']; $j++) {
            $this->value[] = [$subattrs['relativeDomainName'][0], $type, $subattrs[$type][$j], ''];
            if (in_array($type, DnsRecordAttribute::$reverseTypes)) {
              end($this->value);
              $ip = $subattrs[$type][$j];
              if ($type == 'aAAARecord') {
                $ip = expandIPv6($ip);
              }
              $aRecords[$type][$subattrs['relativeDomainName'][0].$ip] = key($this->value);
            }
          }
        }
      }
      $reverseZones = $this->loadReverseZones($attrs);
      /* Search reverse zones for our IPs */
      foreach ($reverseZones as $reverseZoneDn => $reverseZoneAttrs) {
        $reverseZoneName = $reverseZoneAttrs['zoneName'][0];
        $ipv6 = preg_match('/ip6/', $reverseZoneName);
        $baseIp = preg_replace('/\.(in-addr|ip6)\.arpa\.?$/i', '', $reverseZoneName);
        if (preg_match('/^([[:digit:]]+[-\/][[:digit:]]+).([\.[:digit:]]+)$/', $baseIp, $m)) {
          /* Reverse zone using rfc2317 with - or / */
          $baseIp = $m[2];
        }
        if ($ipv6) {
          $baseIp = implode(':', array_map('implode', array_chunk(array_reverse(explode('.', $baseIp)), 4)));
        } else {
          $baseIp = implode('.', array_reverse(explode('.', $baseIp)));
        }
        $ldap->cd($reverseZoneDn);
        $ldap->search('(objectClass=dNSZone)', ['relativeDomainName', 'pTRRecord'], 'one');
        while ($subattrs = $ldap->fetch()) {
          unset($subattrs['pTRRecord']['count']);
          foreach ($subattrs['pTRRecord'] as $ptrRecord) {
            if (preg_match('/^'.preg_quote($attrs['zoneName'][0], '/').'\.?$/', $ptrRecord)) {
              $name = '@';
            } else {
              $name = preg_replace('/\.'.preg_quote($attrs['zoneName'][0], '/').'\.?$/', '', $ptrRecord);
            }
            if ($ipv6) {
              $ip = $baseIp.':'.implode(':', array_map('implode', array_chunk(array_reverse(explode('.', $subattrs['relativeDomainName'][0])), 4)));
            } else {
              $ip = $baseIp.'.'.implode('.', array_reverse(explode('.', $subattrs['relativeDomainName'][0])));
            }
            if ($ipv6 && isset($aRecords['aAAARecord'][$name.$ip])) {
              $this->value[$aRecords['aAAARecord'][$name.$ip]][3] = $reverseZoneName;
            } elseif (!$ipv6 && isset($aRecords['aRecord'][$name.$ip])) {
              $this->value[$aRecords['aRecord'][$name.$ip]][3] = $reverseZoneName;
            }
          }
        }
      }
      $this->initialReverseZones = $reverseZones;
    } else {
      $this->initialReverseZones = [];
    }
    sort($this->value);
  }

  function fillLdapValue (array &$attrs)
  {
    foreach (DnsRecordAttribute::getLdapAttributesList() as $type) {
      if (!isset($attrs[$type])) {
        $attrs[$type] = [];
      }
    }
    foreach ($this->value as $line) {
      list ($domain, $type, $content) = $line;
      // Only save root records here
      if (empty($domain)) {
        $attrs[$type][] = $content;
      }
    }
  }

  private function valueToNodes ($value)
  {
    /* Compute values into $nodes and $ptrs */
    $zoneDn     = $this->getZoneDn();
    $zoneName   = $this->getZoneName();
    $nodes      = [];
    $ptrs       = [];
    $nsRecords  = [];
    foreach ($value as $line) {
      list ($domain, $type, $content, $reverse) = $line;
      if (!empty($reverse)) {
        if (!DnsRecordAttribute::matchReverseZone($type, $content, $reverse)) {
          $error = new SimplePluginError(
            $this,
            htmlescape(sprintf(
              _('The IP %s does not match the selected reverse %s, it has been ignored'),
              $content,
              $reverse
            ))
          );
          $error->display();
        } else {
          if (!isset($ptrs[$reverse])) {
            $ptrs[$reverse] = [];
          }
          if (!isset($ptrs[$reverse][$content])) {
            $ptrs[$reverse][$content] = [];
          }
          $ptrs[$reverse][$content][] = $domain;
        }
      }
      if (empty($domain)) {
        if ($type == 'nSRecord') {
          $nsRecords[] = $content;
        }
        /* ignore root records */
        continue;
      }
      $dn = 'relativeDomainName='.$domain.','.$zoneDn;
      if (!isset($nodes[$dn])) {
        $nodes[$dn] = [
          'objectClass'         => ['dNSZone'],
          'relativeDomainName'  => $domain,
          'zoneName'            => $zoneName,
          'dNSClass'            => 'IN',
        ];
      }
      if (!isset($nodes[$dn][$type])) {
        $nodes[$dn][$type] = [];
      }
      $nodes[$dn][$type][] = $content;
    }
    return [$nodes, $ptrs, $nsRecords];
  }

  protected function reverseZoneNeedUpdate (array $new, array $old, array $ptrs, array $initialPtrs, $reverseZone)
  {
    // NS Record changes
    if (!empty($old['nSRecord'])) {
      unset($old['nSRecord']['count']);
      if (array_differs($new['nSRecord'], $old['nSRecord'])) {
        return TRUE;
      }
    } elseif (!empty($new['nSRecord'])) {
      return TRUE;
    }
    // PTR changes
    if (!isset($initialPtrs[$reverseZone])) {
      if (isset($ptrs[$reverseZone])) {
        return TRUE;
      }
    } elseif (!isset($ptrs[$reverseZone])) {
      return TRUE;
    } elseif (array_differs_recursive($ptrs[$reverseZone], $initialPtrs[$reverseZone])) {
      return TRUE;
    }
    return FALSE;
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    $zoneDn   = $this->getZoneDn();
    $zoneName = $this->getZoneName();
    if (!$this->plugin instanceof dnsZone) {
      die('Should only be called on dnsZone instances');
    }
    $this->reverseZones = $this->plugin->reverseZones;
    // SOA changes (ignoring serial)
    $initialSoa = $this->plugin->attributesAccess['sOARecord']->readValues($this->plugin->attributesAccess['sOARecord']->getInitialValue());
    $newSoa     = $this->plugin->attributesAccess['sOARecord']->readValues($this->plugin->attributesAccess['sOARecord']->getValue());
    unset($initialSoa[2]);
    unset($newSoa[2]);
    $soaChanged = array_differs($initialSoa, $newSoa);
    /* Compute values into $nodes and $ptrs */
    list ($nodes, $ptrs, $nsRecords) = $this->valueToNodes($this->value);
    /* List all old nodes */
    list ($initialNodes, $initialPtrs, ) = $this->valueToNodes($this->getInitialValue());
    /* Update reverse zones top nodes */
    $oldReverseZones = $this->initialReverseZones;
    foreach ($this->reverseZones as $reverseZone) {
      /* Write this reverse under the zone dn */
      $reverseDn = 'zoneName='.$reverseZone.','.$zoneDn;
      /* Copy NS records and SOA from zone */
      $node = [
        'objectClass'         => ['dNSZone'],
        'zoneName'            => $reverseZone,
        'relativeDomainName'  => '@',
        'dNSClass'            => 'IN',
        'sOARecord'           => $this->plugin->sOARecord,
        'nSRecord'            => $nsRecords
      ];
      $ldap->cd($reverseDn);
      if (isset($oldReverseZones[$reverseDn])) {
        if (!$soaChanged && !$this->reverseZoneNeedUpdate($node, $oldReverseZones[$reverseDn], $ptrs, $initialPtrs, $reverseZone)) {
          unset($oldReverseZones[$reverseDn]);
          continue;
        }
        $ldap->modify($node);
        if (!$ldap->success()) {
          $error = new SimplePluginLdapError($this, $reverseDn, LDAP_MOD, $ldap->get_error(), $ldap->get_errno());
          $error->display();
        }
        unset($oldReverseZones[$reverseDn]);
      } else {
        if (empty($node['nSRecord'])) {
          unset($node['nSRecord']);
        }
        $ldap->add($node);
        if (!$ldap->success()) {
          $error = new SimplePluginLdapError($this, $reverseDn, LDAP_ADD, $ldap->get_error(), $ldap->get_errno());
          $error->display();
        }
      }
    }
    /* Delete old reverse not present in new list */
    foreach (array_keys($oldReverseZones) as $dn) {
      $ldap->rmdir_recursive($dn);
      if (!$ldap->success()) {
        $error = new SimplePluginLdapError($this, $dn, LDAP_DEL, $ldap->get_error(), $ldap->get_errno());
        $error->display();
      }
    }
    /* Delete old records not present in new list */
    foreach (array_keys($initialNodes) as $dn) {
      if (!isset($nodes[$dn])) {
        $ldap->rmdir_recursive($dn);
        if (!$ldap->success()) {
          $error = new SimplePluginLdapError($this, $dn, LDAP_DEL, $ldap->get_error(), $ldap->get_errno());
          $error->display();
        }
      }
    }
    /* Then add our values */
    foreach ($nodes as $dn => $node) {
      $ldap->cd($dn);
      if (isset($initialNodes[$dn])) {
        if (!array_differs_recursive($initialNodes[$dn], $node)) {
          continue;
        }
        foreach (DnsRecordAttribute::getLdapAttributesList() as $type) {
          if (!isset($node[$type])) {
            $node[$type] = [];
          }
        }
        $ldap->modify($node);
        $operation = LDAP_MOD;
      } else {
        $ldap->add($node);
        $operation = LDAP_ADD;
      }
      if (!$ldap->success()) {
        $error = new SimplePluginLdapError($this, $dn, $operation, $ldap->get_error(), $ldap->get_errno());
        $error->display();
      }
    }

    foreach ($this->reverseZones as $reverseZone) {
      $reverseDn  = 'zoneName='.$reverseZone.','.$zoneDn;
      $ipv6       = preg_match('/ip6/', $reverseZone);
      /* Write each needed PTR */
      if (isset($ptrs[$reverseZone])) {
        foreach ($ptrs[$reverseZone] as $ip => $names) {
          if (isset($initialPtrs[$reverseZone][$ip]) && !array_differs_recursive($initialPtrs[$reverseZone][$ip], $names)) {
            unset($initialPtrs[$reverseZone][$ip]);
            continue;
          }
          list($mask, $reversedIp) = DnsRecordAttribute::getReverseZoneInfo($ipv6, $ip, $reverseZone);
          $relativeDomainName = preg_replace('/\.'.preg_quote($mask).'$/', '', $reversedIp);
          $dn = 'relativeDomainName='.$relativeDomainName.','.$reverseDn;
          $ldap->cd($dn);
          $node = [
            'objectClass'         => ['dNSZone'],
            'zoneName'            => $reverseZone,
            'dNSClass'            => 'IN',
            'pTRRecord'           => array_map(
              function ($name) use ($zoneName)
              {
                return (empty($name) ? '' : $name.'.').$zoneName;
              },
              $names
            ),
            'relativeDomainName'  => $relativeDomainName,
          ];
          if (isset($initialPtrs[$reverseZone][$ip])) {
            $ldap->modify($node);
            if (!$ldap->success()) {
              $error = new SimplePluginLdapError($this, $dn, LDAP_MOD, $ldap->get_error(), $ldap->get_errno());
              $error->display();
            }
            unset($initialPtrs[$reverseZone][$ip]);
          } else {
            $ldap->add($node);
            if (!$ldap->success()) {
              $error = new SimplePluginLdapError($this, $dn, LDAP_ADD, $ldap->get_error(), $ldap->get_errno());
              $error->display();
            }
          }
        }
      }
      if (isset($initialPtrs[$reverseZone])) {
        foreach (array_keys($initialPtrs[$reverseZone]) as $ip) {
          list($mask, $reversedIp) = DnsRecordAttribute::getReverseZoneInfo($ipv6, $ip, $reverseZone);
          $relativeDomainName = preg_replace('/\.'.preg_quote($mask).'$/', '', $reversedIp);
          $dn = 'relativeDomainName='.$relativeDomainName.','.$reverseDn;
          $ldap->rmdir_recursive($dn);
          if (!$ldap->success()) {
            $error = new SimplePluginLdapError($this, $dn, LDAP_DEL, $ldap->get_error(), $ldap->get_errno());
            $error->display();
          }
        }
      }
    }
  }

  function cnChanged ($oldcn, $newcn)
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

  function ipChanged ($oldip, $newip)
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

  function cnRemoved ($oldcn)
  {
    $nb = 0;
    foreach ($this->value as $key => $row) {
      if ($row[0] == $oldcn) {
        unset($this->value[$key]);
        $nb++;
      }
    }
    return $nb;
  }

  function ipRemoved ($oldips)
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

  protected function canWriteRecord ($value)
  {
    $ui     = get_userinfo();
    $rights = $ui->get_permissions($this->getZoneDn(), 'dnsZone/DnsRecordPlugin', 'dnsRecord_'.$value[1], $this->plugin->readOnly());
    return preg_match('/w/', $rights);
  }

  protected function genRowIcons ($key, $value)
  {
    list ($img, $nbicons) = parent::genRowIcons($key, $value);
    if ($this->canWriteRecord($value)) {
      return [$img, $nbicons];
    } else {
      return ['', 0];
    }
  }

  protected function handleEdit ($key)
  {
    if ($this->canWriteRecord($this->value[$key])) {
      return parent::handleEdit($key);
    }
  }

  function delPostValue ($key)
  {
    if ($this->canWriteRecord($this->value[$key])) {
      return parent::delPostValue($key);
    }
  }
}

class FQDNAttribute extends StringAttribute
{
  function validate ()
  {
    if (dnsZone::finalDot()) {
      $regexp = '/^(([a-z0-9\-]{1,63})\.)+$/';
      $error  = _('"%s" must contain a domain name in lowercase and end with a final dot.<br/><br/>Example: example.com.');
    } else {
      $regexp = '/^(([a-z0-9\-]{1,63})\.)*[a-z0-9\-]{1,63}$/';
      $error  = _('"%s" must contain a domain name in lowercase.<br/><br/>Example: example.com');
    }
    if (!preg_match($regexp, $this->value)) {
      return sprintf($error, $this->getLabel());
    }
  }
}

class dnsZone extends simplePlugin
{
  public static function finalDot ()
  {
    global $config;
    return ($config->get_cfg_value('DNSFinalDot', 'TRUE') == 'TRUE');
  }

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('DNS zone'),
      'plDescription' => _('DNS zone'),
      'plObjectClass' => ['dNSZone'],
      'plFilter'      => '(&(objectClass=dNSZone)(relativeDomainName=@)(!(|(zoneName=*.arpa)(zoneName=*.arpa.))))',
      'plObjectType'  => ['dnsZone' => [
        'name'        => _('DNS zone'),
        'ou'          => get_ou('dnsRDN'),
        'icon'        => 'geticon.php?context=applications&icon=dns&size=16',
        'mainAttr'    => 'zoneName'
      ]],
      'plSearchAttrs' => ['relativeDomainName', 'sOARecord'],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Zone'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('dnsRDN')),
          new FQDNAttribute(
            _('Zone name'), _('Zone name'),
            'zoneName', TRUE
          ),
          new HiddenAttribute(
            // Relative domain name
            'relativeDomainName', TRUE,
            '@'
          ),
          new SetAttribute(
            new StringAttribute(
              _('Reverse zones'), sprintf(_('Reverse zones for this zone in the form xx.xx.in-addr.arpa%1$s or x.x.ip6.arpa%1$s'), (dnsZone::finalDot() ? '.' : '')),
              'reverseZones', FALSE,
              '', '',
              '/^.*\.(in-addr|ip6)\.arpa'.(dnsZone::finalDot() ? '\.' : '').'$/i', '11.168.192.in-addr.arpa'.(dnsZone::finalDot() ? '.' : '')
            )
          ),
          new HiddenAttribute(
            'dNSClass', TRUE,
            'IN'
          ),
        ]
      ],
      'soa' => [
        'name'  => _('SOA record'),
        'attrs' => [
          new CompositeAttribute(
            _('SOA Record'),
            'sOARecord',
            [
              new FQDNAttribute(
                _('Primary DNS server'), _('Domain name of the name server that was the original or primary source of data for this zone'),
                'soaRecord_primary', TRUE
              ),
              new FQDNAttribute(
                _('Mail address'), _('Domain name which specifies the mailbox of the person responsible for this zone'),
                'soaRecord_mail', TRUE
              ),
              new DisplayLDAPAttribute(
                _('Serial number'), _('Version number of the original copy of the zone'),
                'soaRecord_serial', FALSE,
                date('Ymd').'01'
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
            ],
            '/^(\S*) (\S*) (\S*) (\S*) (\S*) (\S*) (\S*)$/',
            '%s %s %s %s %s %s %s'
          ),
        ]
      ],
      'records' => [
        'name'  => _('Records'),
        'class' => ['fullwidth'],
        'attrs' => [
          new DnsRecordsAttribute(
            '', _('The DNS records for this zone'),
            'dnsRecords', FALSE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['reverseZones']->setInLdap(FALSE);
  }

  function prepareSavedAttributes ()
  {
    parent::prepareSavedAttributes();
    foreach (DnsRecordAttribute::getLdapAttributesList() as $type) {
      if (isset($this->attrs[$type])) {
        $this->saved_attributes[$type] = $this->attrs[$type];
      }
    }
  }

  function prepare_save (): array
  {
    /* Update serial (Also thanks to this save is triggered if something changed - see simplePlugin::shouldSave) */
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
    return parent::prepare_save();
  }

  function ldap_save (): array
  {
    global $config;
    $errors = parent::ldap_save();
    if (
      $this->attributesAccess['zoneName']->hasChanged() ||
      $this->attributesAccess['reverseZones']->hasChanged() ||
      $this->attributesAccess['sOARecord']->hasChanged() ||
      $this->attributesAccess['dnsRecords']->hasChanged()
      ) {
      $this->attributesAccess['dnsRecords']->postLdapSave($config->get_ldap_link());
    }
    return $errors;
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

  function cnRemoved ($oldcn)
  {
    $nb = $this->attributesAccess['dnsRecords']->cnRemoved($oldcn);
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
