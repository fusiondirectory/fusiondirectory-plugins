<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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
  protected $dhcpObjectCache = array();

  protected function getAttributeArrayValue($key, $value)
  {
    $id = $this->getHtmlId();
    $subnetExpandImg = $this->renderInputField(
      'image', $id.'_expandDHCP_'.$key,
      array(
        'src'   => 'images/lists/expand.png',
        'title' => _('Expand DHCP subnet'),
        'alt'   => _('Expand'),
        'class' => 'center',
        'style' => 'padding:5px 6px 4px;'
      )
    );
    $subnetExpandedImg = $this->renderInputField(
      'image', $id.'_foldDHCP_'.$key,
      array(
        'src'   => 'images/down-arrow.png',
        'title' => _('Fold DHCP subnet'),
        'alt'   => _('Fold'),
        'class' => 'center',
        'style' => 'padding:6px 5px 6px 4px;'
      )
    );
    $dn = $value[0];
    if (empty($this->dhcpObjectCache[$dn])) {
      return array('not in cache',$dn,'');
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
        return array(
          array('html' => $link),
          '',
          '',
        );
      } else {
        if ($objtype == 'dhcpSubnet') {
          if ($dn === $this->subnet_expanded) {
            $link = '&nbsp;&nbsp;'.$subnetExpandedImg.$link;
          } else {
            $link = '&nbsp;&nbsp;'.$subnetExpandImg.$link;
          }
        }
        return array(
          array('html' => $link),
          array('html' => $dhcpObject->getOption('routers')),
          ''
        );
      }
    } else {
      return array(
        array('html' => $link),
        array('html' => $dhcpObject->getStatement('fixed-address')),
        array('html' => preg_replace('/^[^ ]+ /', '', isset($dhcpObject->dhcpHWAddress) ? $dhcpObject->dhcpHWAddress : '')),
      );
    }
  }

  protected function genRowIcons($key, $value)
  {
    $id = $this->getHtmlId();
    list ($img, $nbicons) = parent::genRowIcons($key, $value);

    if ($key == 0) {
      /* Remove delete button on first row */
      $img = preg_replace('/<input[^>]+>$/', '', $img);
    }

    $img = $this->renderInputField(
      'image', $id.'_insertDHCP_'.$key,
      array(
        'src'   => 'geticon.php?context=actions&amp;icon=document-new&amp;size=16',
        'title' => _('Insert new DHCP section'),
        'alt'   => _('Insert new DHCP section'),
        'class' => 'center',
      )
    ).$img;
    $nbicons++;

    return array ($img, $nbicons);
  }

  protected function handlePostValueActions($id, $postValue)
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

  protected function handleEdit($key)
  {
    $dn           = $this->value[$key][0];
    $editingValue = $this->dhcpObjectCache[$dn];
    $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->objectType($dn), $dn, $editingValue));
  }

  function delPostValue($key)
  {
    $dn = $this->postValue[$key][0];
    $this->dhcpObjectCache[$dn] = array();
    foreach ($this->postValue as $i => $value) {
      if (preg_match('/'.preg_quote($dn, '/').'$/', $value[0])) {
        unset($this->postValue[$i]);
        $this->dhcpObjectCache[$value[0]] = array();
      }
    }
    parent::delPostValue($key);
  }

  /* Should be addValue but we need more attributes */
  function editEnded($dn, $object, $olddn = FALSE)
  {
    $this->dhcpObjectCache[$dn] = $object;
    if (($olddn !== FALSE) && ($olddn !== $dn)) {
      /* If we renamed a section, we've to rename a couple of objects, too */
      foreach ($this->dhcpObjectCache as $key => $dsc) {
        if (preg_match("/,$olddn$/", $key)) {
          $new_dn = preg_replace("/,$olddn$/", ','.$dn, $key);
          $dsc['MODIFIED'] = TRUE;
          $this->dhcpObjectCache[$new_dn] = $dsc;
          $this->dhcpObjectCache[$key]    = array();
        }
      }
      $this->dhcpObjectCache[$olddn]    = array();
    }
    $this->reload(FALSE);
  }

  /* Used by dhcpHost */
  function addHost($parentDn, $cn, $mac, $ip)
  {
    $dn = 'cn='.$cn.','.$parentDn;
    $this->dhcpObjectCache[$dn] = array(
      'dn'              => $dn,
      'objectClass'     => array('dhcpHost'),
      'cn'              => array($cn),
      'dhcpHWAddress'   => array('ethernet '.$mac),
      'dhcpStatements'  => array(
        'fixed-address '.$ip
      ),
      'dhcpOption'      => array(),
      'dhcpComments'    => array(),
      'MODIFIED'        => TRUE,
    );
    $this->reload(FALSE);
  }

  /* Used by dhcpHost */
  function delHost($dn)
  {
    if (($dn != '') && ($dn != 'new')) {
      $this->dhcpObjectCache[$dn] = array();
    }
  }

  protected function loadAttrValue ($attrs)
  {
  }

  /* Not saving anything into base node */
  function fillLdapValue (&$attrs)
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
      return msgPool::required(_('Name'));
    }
  }

  function getGlobalCn ()
  {
    return $this->dhcpObjectCache[$this->value[0][0]]['cn'][0];
  }

  /* Subfonction of reload function */
  protected function reload_readItemFromLDAP(&$ldap, $attrs, &$final, $erase)
  {
    global $config;
    $sattrs = array();
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
      $ldap_subnet->search('(objectClass=dhcpHost)', array('*'), 'one');
      while ($host_attrs = $ldap_subnet->fetch()) {
        $this->reload_readItemFromLDAP($ldap_subnet, $host_attrs, $final, $erase);
      }
    }
  }

  /* Subfonction of reload function */
  protected function reload_refreshListFromCache(&$final)
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
      if ($type_id === NULL) {
        trigger_error('Unknown DHCP type for '.$dn);
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

  function reload($erase = TRUE)
  {
    global $config;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $erase, 'reload');
    /* Init LDAP and load list */
    $ldap = $config->get_ldap_link();

    $final  = array();
    $ldap->cat($this->plugin->dn, array('cn'), '(objectClass=dhcpService)');

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
      $ldap->search('(|(objectClass=dhcpService)(objectClass=dhcpClass)(objectClass=dhcpSubClass)(objectClass=dhcpGroup)(objectClass=dhcpPool)(objectClass=dhcpSubnet)(objectClass=dhcpSharedNetwork)(objectClass=dhcpTSigKey)(objectClass=dhcpDnsZone)(objectClass=dhcpFailOverPeer))', array());

      while ($attrs = $ldap->fetch()) {
        $this->reload_readItemFromLDAP($ldap, $attrs, $final, $erase);
      }
    }
    $this->reload_refreshListFromCache($final);

    /* Sort it... */
    natsort($final);
    $this->value = array();
    foreach ($final as $dn => $val) {
      $this->value[] = array($dn, preg_replace('/^[^!]+!(.*)$/', '\\1', $val));
    }

    if (empty($final)) {
      $this->add_global_node();
    }
  }

  function add_global_node()
  {
    $attrs = array(
      'dn'              => $this->plugin->dn,
      'cn'              => array($this->plugin->cn),
      'objectClass'     => array('top', 'dhcpService'),
      'dhcpStatements'  => array(
        'default-lease-time 600',
        'max-lease-time 7200',
        'authoritative',
        'ddns-update-style none'
      ),
      'MODIFIED' => TRUE,
    );

    $this->value[] = array($this->plugin->dn, _('Global options'));
    $this->dhcpObjectCache[$this->plugin->dn]  = $attrs;
  }

  function objectType($dn)
  {
    $types = array_keys($this->plugin->types);

    foreach ($this->dhcpObjectCache[$dn]['objectClass'] as $oc) {
      if (in_array($oc, $types)) {
        return $oc;
      }
    }

    /* That should not happen... */
    msg_dialog::display(_('Error'), _('The DHCP configuration set is unkown. Please contact your system administrator.'), ERROR_DIALOG);

    return '';
  }

  function getCache()
  {
    return $this->dhcpObjectCache;
  }

  function renderButtons ()
  {
    return '';
  }

  public function htmlIds()
  {
    $ids = parent::htmlIds();
    unset($ids[0]);
    return $ids;
  }

  function foreignKeyUpdate($oldvalue, $newvalue, $source)
  {
    if (($source['CLASS'] == 'serverGeneric') && ($source['FIELD'] == 'dn') && ($source['MODE'] == 'move')) {
      $globalNode = $this->value[0][0];
      if (isset($this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN']) && in_array_ics($oldvalue, $this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN'])) {
        if ($newvalue === NULL) {
          unset($this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN']);
        } else {
          $this->dhcpObjectCache[$globalNode]['dhcpPrimaryDN'] = array($newvalue);
        }
        $this->dhcpObjectCache[$globalNode]['MODIFIED'] = TRUE;
      }
      if (isset($this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN']) && in_array_ics($oldvalue, $this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN'])) {
        if ($newvalue === NULL) {
          unset($this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN']);
        } else {
          $this->dhcpObjectCache[$globalNode]['dhcpSecondaryDN'] = array($newvalue);
        }
        $this->dhcpObjectCache[$globalNode]['MODIFIED'] = TRUE;
      }
    } else {
      return parent::foreignKeyCheck($value, $source);
    }
  }

  function foreignKeyCheck($value, $source)
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

  function isIpInParentSubnet($dn, $ip)
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

class dhcpConfiguration extends simplePlugin
{
  public $mainTab = TRUE;

  public $objectclasses = array('dhcpService');

  public $types = array();

  static $sectionMap = array(
    'dhcpService'       => array(
      'dhcpSharedNetwork','dhcpSubnet','dhcpGroup',
      'dhcpHost','dhcpClass','dhcpTSigKey','dhcpDnsZone','dhcpFailOverPeer'
    ),
    'dhcpClass'         => array('dhcpSubClass'),
    'dhcpSubClass'      => array(),
    'dhcpHost'          => array(),
    'dhcpGroup'         => array('dhcpHost'),
    'dhcpPool'          => array(),
    'dhcpSubnet'        => array(
      'dhcpPool','dhcpGroup',
      'dhcpHost','dhcpClass','dhcpTSigKey','dhcpDnsZone','dhcpFailOverPeer'
    ),
    'dhcpSharedNetwork' => array('dhcpSubnet', 'dhcpPool','dhcpTSigKey','dhcpDnsZone','dhcpFailOverPeer'),
    'dhcpFailOverPeer'  => array(),
    'dhcpTSigKey'       => array(),
    'dhcpDnsZone'       => array()
  );

  static $quote_option = array('domain-name');

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('DHCP configuration'),
      'plDescription' => _('DHCP configuration'),
      'plObjectType'  => array('dhcpConfiguration' => array(
        'name'        => _('DHCP configuration'),
        'filter'      => '(objectClass=dhcpService)',
        'ou'          => get_ou('dhcpRDN'),
        'icon'        => 'geticon.php?context=applications&icon=dhcp&size=16'
      )),
      'plForeignKeys'   => array(
        'dhcpSections' => array(
          array('serverGeneric', 'dn', '(|(dhcpPrimaryDN=%oldvalue%)(dhcpSecondaryDN=%oldvalue%))'),
        ),
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('DHCP Objects'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('dhcpRDN')),
          new HiddenAttribute('cn'),
          new DhcpSectionsAttribute (
            '', _('The DHCP sections handled by this server'),
            'dhcpSections', FALSE
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    $this->types = array(
      'dhcpLog'           => _('Logging'),
      'dhcpService'       => _('Global options'),
      'dhcpClass'         => _('Class'),
      'dhcpSubClass'      => _('Subclass'),
      'dhcpHost'          => _('Host'),
      'dhcpGroup'         => _('Group'),
      'dhcpPool'          => _('Pool'),
      'dhcpSubnet'        => _('Subnet'),
      'dhcpFailOverPeer'  => _('Failover peer'),
      'dhcpSharedNetwork' => _('Shared network'),
      'dhcpTSigKey'       => _('DNS update key'),
      'dhcpDnsZone'       => _('DNS update zones')
    );

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['dhcpSections']->setHeight(400);
    $this->cn = $this->attributesAccess['dhcpSections']->getGlobalCn();
  }

  function save_object()
  {
    parent::save_object();
    $this->cn = $this->attributesAccess['dhcpSections']->getGlobalCn();
  }

  function save()
  {
    $this->cn = $this->attributesAccess['dhcpSections']->getGlobalCn();
    return parent::save();
  }

  protected function shouldSave()
  {
    return TRUE;
  }

  protected function ldap_save()
  {
    global $config;
    $this->ldap_error = 'Success';
    /* Save dhcp settings */
    $ldap   = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
    $cache  = $this->attributesAccess['dhcpSections']->getCache();
    $errors = array();
    $new    = ($this->orig_dn == 'new');
    foreach ($cache as $dn => $data) {
      if (($this->dn != $this->orig_dn) && !$new) {
        $dn = preg_replace('/'.preg_quote($this->orig_dn, '/').'$/i', $this->dn, $dn);
      }

      /* Remove entry? */
      if (count($data) == 0) {
        if (!$new) {
          /* Check if exists, then remove... */
          if ($ldap->cat($dn)) {
            $ldap->rmdir_recursive($dn);
            if (!$ldap->success()) {
              $this->ldap_error = $ldap->get_error();
              $errors[] = msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class());
            }
          }
        }
        continue;
      }

      /* Modify existing entry? */
      if (isset($data['MODIFIED']) || ($this->orig_dn != $this->dn)) {
        if ($ldap->cat($dn)) {
          $modify = TRUE;
        } else {
          $modify = FALSE;
        }

        /* Build new entry */
        $attrs = array();
        foreach ($data as $attribute => $values) {
          if ($attribute == 'MODIFIED' || $attribute == 'dn') {
            continue;
          }

          if (in_array($attribute, array('dhcpServerDN','dhcpFailOverPeerDN'))) {
            foreach ($values as $v_key => $value) {
              $values[$v_key] = preg_replace('/'.preg_quote($this->orig_dn, '/').'$/i', $this->dn, $value);
            }
          }

          if (count($values)) {
            if ($attribute == 'dhcpOption') {
              foreach ($values as $key => $value) {
                $option_name  = trim(preg_replace('/[^ ]*$/', '', $value));
                $option_value = trim(preg_replace('/^[^ ]*/', '', $value));
                if (in_array($option_name, static::$quote_option)) {
                  $values[$key] = $option_name.' "'.$option_value.'"';
                }
              }
            }
            if (is_array($values) && (count($values) == 1)) {
              $attrs[$attribute] = $values[0];
            } else {
              $attrs[$attribute] = $values;
            }
          } elseif ($modify) {
            $attrs[$attribute] = array();
          }
        }

        $ldap->cd($dn);
        if ($modify) {
          $ldap->modify($attrs);
          if (!$ldap->success()) {
            $this->ldap_error = $ldap->get_error();
            $errors[] = msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class());
          }
        } else {
          $ldap->add($attrs);
          if (!$ldap->success()) {
            $this->ldap_error = $ldap->get_error();
            $errors[] = msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class());
          }
        }
      }
    }

    return $errors;
  }
}
