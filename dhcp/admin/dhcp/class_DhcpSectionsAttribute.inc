<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class DhcpSectionsAttribute extends DialogOrderedArrayAttribute
{
  protected $dialogClass = 'DhcpSectionCreationDialog';

  protected $subnet_expanded = FALSE;
  protected $dhcpObjectCache = [];

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setHeaders([
      _('Object'),
      _('IP'),
      _('Mac'),
      '',
    ]);
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    $id = $this->getHtmlId();
    $subnetExpandImg = $this->renderInputField(
      'image', $id.'_expandDHCP_'.$key,
      [
        'src'   => 'images/lists/expand.png',
        'title' => _('Expand DHCP subnet'),
        'alt'   => _('Expand'),
        'class' => 'center',
        'style' => 'padding:5px 6px 4px;'
      ],
      FALSE
    );
    $subnetExpandedImg = $this->renderInputField(
      'image', $id.'_foldDHCP_'.$key,
      [
        'src'   => 'images/down-arrow.png',
        'title' => _('Fold DHCP subnet'),
        'alt'   => _('Fold'),
        'class' => 'center',
        'style' => 'padding:6px 5px 6px 4px;'
      ],
      FALSE
    );
    $dn = $value[0];
    if (empty($this->dhcpObjectCache[$dn])) {
      return ['not in cache',$dn,''];
    }

    $link = $value[1];

    $objtype = $this->objectType($dn);
    $dhcpObject = new $objtype($this->plugin, $dn, $this->dhcpObjectCache[$dn]);
    if (count(dhcpConfiguration::$sectionMap[$objtype])) {
      if ($objtype == 'dhcpService') {
        if ($dn === $this->subnet_expanded) {
          $link = $subnetExpandedImg.$link;
        } else {
          $link = $subnetExpandImg.$link;
        }
        return [
          ['html' => $link],
          '',
          '',
        ];
      } else {
        if ($objtype == 'dhcpSubnet') {
          if ($dn === $this->subnet_expanded) {
            $link = '&nbsp;&nbsp;'.$subnetExpandedImg.$link;
          } else {
            $link = '&nbsp;&nbsp;'.$subnetExpandImg.$link;
          }
        }
        $ip = $dhcpObject->getOption('routers');
        return [
          ['html' => $link],
          (($ip === NULL) ? '' : $ip),
          ''
        ];
      }
    } else {
      $ip = $dhcpObject->getStatement('fixed-address');
      return [
        ['html' => $link],
        (($ip === NULL) ? '' : $ip),
        preg_replace('/^[^ ]+ /', '', (isset($dhcpObject->dhcpHWAddress) ? $dhcpObject->dhcpHWAddress : '')),
      ];
    }
  }

  protected function genRowIcons ($key, $value)
  {
    $id = $this->getHtmlId();
    list ($img, $nbicons) = parent::genRowIcons($key, $value);

    if ($key == 0) {
      /* Remove delete button on first row */
      $img = preg_replace('/<input[^>]+>$/', '', $img);
    }

    $img = $this->renderInputField(
      'image', $id.'_insertDHCP_'.$key,
      [
        'src'   => 'geticon.php?context=actions&icon=document-new&size=16',
        'title' => _('Insert new DHCP section'),
        'alt'   => _('Insert new DHCP section'),
        'class' => 'center',
      ],
      FALSE
    ).$img;
    $nbicons++;

    return  [$img, $nbicons];
  }

  protected function handlePostValueActions ($id, $postValue)
  {
    if (parent::handlePostValueActions($id, $postValue)) {
      return TRUE;
    }
    if (preg_match('/^'.$id.'_expandDHCP_/', $postValue)) {
      $key = preg_replace('/^'.$id.'_expandDHCP_/', '', $postValue);
      $key = preg_replace('/_[xy]$/', '', $key);

      $this->subnet_expanded = $this->value[$key][0];
      $this->reload(FALSE);
      $this->postValue = $this->value;

      return TRUE;
    }
    if (preg_match('/^'.$id.'_foldDHCP_/', $postValue)) {
      $this->subnet_expanded = FALSE;
      $this->reload(FALSE);
      $this->postValue = $this->value;

      return TRUE;
    }
    if (preg_match('/^'.$id.'_insertDHCP_/', $postValue)) {
      $key = preg_replace('/^'.$id.'_insertDHCP_/', '', $postValue);
      $key = preg_replace('/_[xy]$/', '', $key);

      $dn = $this->value[$key][0];
      $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->objectType($dn), $dn));
      return TRUE;
    }
    return FALSE;
  }

  protected function handleEdit ($key)
  {
    $dn           = $this->value[$key][0];
    $editingValue = $this->dhcpObjectCache[$dn];
    $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->objectType($dn), $dn, $editingValue));
  }

  function delPostValue ($key)
  {
    $dn = $this->postValue[$key][0];
    $this->dhcpObjectCache[$dn] = [];
    foreach ($this->postValue as $i => $value) {
      if (preg_match('/'.preg_quote($dn, '/').'$/', $value[0])) {
        unset($this->postValue[$i]);
        $this->dhcpObjectCache[$value[0]] = [];
      }
    }
    parent::delPostValue($key);
  }

  /* Should be addValue but we need more attributes */
  function editEnded ($dn, $object, $olddn = FALSE)
  {
    $this->dhcpObjectCache[$dn] = $object;
    if (($olddn !== FALSE) && ($olddn !== $dn)) {
      /* If we renamed a section, we've to rename a couple of objects, too */
      foreach ($this->dhcpObjectCache as $key => $dsc) {
        if (preg_match("/,$olddn$/", $key)) {
          $new_dn = preg_replace("/,$olddn$/", ','.$dn, $key);
          $dsc['MODIFIED'] = TRUE;
          $this->dhcpObjectCache[$new_dn] = $dsc;
          $this->dhcpObjectCache[$key]    = [];
        }
      }
      $this->dhcpObjectCache[$olddn]    = [];
    }
    $this->reload(FALSE);
  }

  /* Used by dhcpHost */
  function addHost ($parentDn, $cn, $mac, $ip)
  {
    $dn = 'cn='.$cn.','.$parentDn;
    $this->dhcpObjectCache[$dn] = [
      'dn'              => $dn,
      'objectClass'     => ['dhcpHost'],
      'cn'              => [$cn],
      'dhcpHWAddress'   => ['ethernet '.$mac],
      'dhcpStatements'  => [
        'fixed-address '.$ip
      ],
      'dhcpOption'      => [],
      'dhcpComments'    => [],
      'MODIFIED'        => TRUE,
    ];
    $this->reload(FALSE);
  }

  /* Used by dhcpHost */
  function delHost ($dn)
  {
    if (($dn != '') && ($dn != 'new')) {
      $this->dhcpObjectCache[$dn] = [];
    }
  }

  protected function loadAttrValue (array $attrs)
  {
  }

  /* Not saving anything into base node */
  function fillLdapValue (array &$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->reload();
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    if (empty($this->value) || empty($this->dhcpObjectCache[$this->value[0][0]]['cn']) || ($this->dhcpObjectCache[$this->value[0][0]]['cn'][0] == '')) {
      return new SimplePluginCheckError(
        $this,
        msgPool::required(_('Name'))
      );
    }
  }

  function getGlobalCn ()
  {
    return $this->dhcpObjectCache[$this->value[0][0]]['cn'][0];
  }

  /* Subfonction of reload function */
  protected function reload_readItemFromLDAP (&$ldap, $attrs, &$final, $erase)
  {
    global $config;
    $sattrs = [];
    $dn     = $attrs['dn'];

    if (isset($this->dhcpObjectCache[$dn]) && !$erase) {
      $sattrs = $this->dhcpObjectCache[$dn];
    } else {
      for ($i = 0; $i < $attrs['count']; $i++) {
        $sattrs[$attrs[$i]] = $attrs[$attrs[$i]];
        unset($sattrs[$attrs[$i]]['count']);
      }
      $sattrs['dn'] = $dn;
      foreach ($sattrs as $name => $values) {
        if ($name == 'dhcpOption') {
          foreach ($values as $key => $value) {
            $value_name   = trim(preg_replace('/[^ ]*$/', '', $value));
            $value_value  = trim(preg_replace('/^[^ ]*/', '', $value));
            if (in_array($value_name, dhcpConfiguration::$quote_option)) {
              $value_value = preg_replace('/^"/', '', $value_value);
              $value_value = preg_replace('/"$/', '', $value_value);
              $sattrs[$name][$key] = $value_name.' '.$value_value;
            }
          }
        }
      }
      $this->dhcpObjectCache[$dn] = $sattrs;
    }

    if ($dn === $this->subnet_expanded) {
      $ldap_subnet = $config->get_ldap_link();
      $ldap_subnet->cd($dn);
      $ldap_subnet->search('(objectClass=dhcpHost)', ['*'], 'one');
      while ($host_attrs = $ldap_subnet->fetch()) {
        $this->reload_readItemFromLDAP($ldap_subnet, $host_attrs, $final, $erase);
      }
    }
  }

  /* Subfonction of reload function */
  protected function reload_refreshListFromCache (&$final)
  {
    $firstdn = NULL;
    foreach ($this->dhcpObjectCache as $dn => $sattrs) {
      if (empty($sattrs)) {
        continue;
      }
      if ($firstdn === NULL) {
        $firstdn = $dn;
      }
      $type_id = NULL;
      foreach ($this->plugin->types as $key => $val) {
        if (in_array($key, $sattrs['objectClass'])) {
          $type     = $val;
          $type_id  = $key;
          break;
        }
      }
      if (!isset($type)) {
        trigger_error('Unknown DHCP type for '.$dn);
        continue;
      }

      if (($type_id == 'dhcpHost') && (preg_replace('/^[^,]+,/', '', $dn) != $this->subnet_expanded)) {
        continue;
      }

      $tmp = preg_replace('/'.preg_quote($firstdn, '/').'/', '', $dn);
      $indent = substr_count($tmp, ',');
      if ($type_id == 'dhcpSubnet') {
        $indent--;
      }
      if ($indent > 0) {
        if ($type_id == 'dhcpSubnet') {
          $spaces = "&nbsp;&nbsp;&nbsp;";
        } else {
          $spaces = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
        }
      } else {
        $spaces = '';
      }
      for ($i = 0; $i < $indent; $i++) {
        $spaces .= "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
      }

      /* Prepare for sorting... */
      $sortpart = explode(',', $dn);
      $sortpart = array_reverse($sortpart);
      $tmp = implode(',', $sortpart);
      $final[$dn] = $tmp.'!'.$spaces.$type.' "'.$sattrs['cn'][0].'"';
    }
  }

  function reload ($erase = TRUE)
  {
    global $config;
    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $erase, 'reload');
    /* Init LDAP and load list */
    $ldap = $config->get_ldap_link();

    $final  = [];
    $ldap->cat($this->plugin->dn, ['cn'], '(objectClass=dhcpService)');

    if ($value = $ldap->fetch()) {
      /* Set header */
      $sortpart = explode(',', $value['dn']);
      $sortpart = array_reverse($sortpart);
      $tmp = implode(',', $sortpart);

      if ($erase || !isset($this->dhcpObjectCache[$value['dn']]) || !empty($this->dhcpObjectCache[$value['dn']])) {
        $final[$value['dn']] = $tmp.'!'._('Global options');
      }

      /* Read all sub entries to place here */
      $ldap->cd($value['dn']);
      $ldap->search('(|(objectClass=dhcpService)(objectClass=dhcpClass)(objectClass=dhcpSubClass)(objectClass=dhcpGroup)(objectClass=dhcpPool)(objectClass=dhcpSubnet)(objectClass=dhcpSharedNetwork)(objectClass=dhcpTSigKey)(objectClass=dhcpDnsZone)(objectClass=dhcpFailOverPeer))', []);

      while ($attrs = $ldap->fetch()) {
        $this->reload_readItemFromLDAP($ldap, $attrs, $final, $erase);
      }
    }
    $this->reload_refreshListFromCache($final);

    /* Sort it... */
    natsort($final);
    $this->value = [];
    foreach ($final as $dn => $val) {
      $this->value[] = [$dn, preg_replace('/^[^!]+!(.*)$/', '\\1', $val)];
    }

    if (empty($final)) {
      $this->add_global_node();
    }
  }

  function add_global_node ()
  {
    $attrs = [
      'dn'              => $this->plugin->dn,
      'cn'              => [$this->plugin->cn],
      'objectClass'     => ['top', 'dhcpService'],
      'dhcpStatements'  => [
        'default-lease-time 600',
        'max-lease-time 7200',
        'authoritative',
        'ddns-update-style none'
      ],
      'MODIFIED' => TRUE,
    ];

    $this->value[] = [$this->plugin->dn, _('Global options')];
    $this->dhcpObjectCache[$this->plugin->dn]  = $attrs;
  }

  function objectType ($dn)
  {
    $types = array_keys($this->plugin->types);

    foreach ($this->dhcpObjectCache[$dn]['objectClass'] as $oc) {
      if (in_array($oc, $types)) {
        return $oc;
      }
    }

    /* That should not happen... */
    $error = new SimplePluginError($this, htmlescape(_('The DHCP configuration set is unkown. Please contact your system administrator.')));
    $error->display();

    return '';
  }

  function getCache ()
  {
    return $this->dhcpObjectCache;
  }

  function renderButtons ()
  {
    return '';
  }

  public function htmlIds (): array
  {
    $ids = parent::htmlIds();
    unset($ids[0]);
    return $ids;
  }

  function foreignKeyUpdate ($oldvalue, $newvalue, array $source)
  {
    if (($source['CLASS'] == 'serverGeneric') && ($source['FIELD'] == 'dn') && ($source['MODE'] == 'move')) {
      $globalNode = $this->value[0][0];
      if (isset($this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN']) && in_array_ics($oldvalue, $this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN'])) {
        if ($newvalue === NULL) {
          unset($this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN']);
        } else {
          $this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN'] = [$newvalue];
        }
        $this->dhcpObjectCache[$globalNode]['MODIFIED'] = TRUE;
      }
      if (isset($this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN']) && in_array_ics($oldvalue, $this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN'])) {
        if ($newvalue === NULL) {
          unset($this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN']);
        } else {
          $this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN'] = [$newvalue];
        }
        $this->dhcpObjectCache[$globalNode]['MODIFIED'] = TRUE;
      }
    } else {
      return parent::foreignKeyUpdate($oldvalue, $newvalue, $source);
    }
  }

  function foreignKeyCheck ($value, array $source): bool
  {
    if (($source['CLASS'] == 'serverGeneric') && ($source['FIELD'] == 'dn')) {
      $globalNode = $this->value[0][0];
      if (isset($this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN']) && in_array_ics($value, $this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN'])) {
        return TRUE;
      }
      if (isset($this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN']) && in_array_ics($value, $this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN'])) {
        return TRUE;
      }
      return FALSE;
    } else {
      return parent::foreignKeyCheck($value, $source);
    }
  }

  function isIpInParentSubnet ($dn, $ip)
  {
    $parts = explode(',', $dn);
    do {
      array_shift($parts);
      $parentdn = implode(',', $parts);
      if (isset($this->dhcpObjectCache[$parentdn]) && ($this->objectType($parentdn) == 'dhcpSubnet')) {
        return dhcpSystem::dhcpIsIpInSubnet($this->dhcpObjectCache[$parentdn], $ip);
      }
    } while (!empty($parts));
    return TRUE;
  }
}
