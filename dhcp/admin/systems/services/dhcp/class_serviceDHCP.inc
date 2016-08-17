<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

    //~ $link = "<a href='?plug=".$_GET['plug']."&act=edit&id=".$key."'>".$value[1]."</a>";
    $link = $value[1];

    $objtype = $this->objectType($dn);
    $dhcpObject = new $objtype($this, $this->dhcpObjectCache[$dn]);
    if (count(serviceDHCP::$sectionMap[$objtype])) {
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
          array('html' => $dhcpObject->options->get('routers')),
          ''
        );
      }
    } else {
      return array(
        array('html' => $link),
        array('html' => $dhcpObject->statements->get('fixed-address')),
        array('html' => preg_replace('/^[^ ]+ /', '', isset($dhcpObject->dhcpHWAddress) ? $dhcpObject->dhcpHWAddress : '')),
      );
    }
  }

  protected function genRowIcons($key, $value)
  {
    $id = $this->getHtmlId();
    list ($img, $width) = parent::genRowIcons($key, $value);
    $img = $this->renderInputField(
      'image', $id.'_insertDHCP_'.$key,
      array(
        'src'   => 'geticon.php?context=actions&amp;icon=document-new&amp;size=16',
        'title' => _('Insert new DHCP section'),
        'alt'   => _('Insert new DHCP section'),
        'class' => 'center',
      )
    ).'&nbsp;'.$img;
    $width += 15;

    return array ($img, $width);
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
    }
    $this->reload(FALSE);
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

  /* Subfonction of reload function */
  function reload_readItemFromLDAP(&$ldap, $attrs, &$final, $erase)
  {
    global $config;
    $sattrs = array();
    $dn     = $ldap->getDN();

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
            if (in_array($value_name, serviceDHCP::$quote_option)) {
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
      $ldap_subnet->ls('(objectClass=dhcpHost)');
      while ($host_attrs = $ldap_subnet->fetch()) {
        $this->reload_readItemFromLDAP($ldap_subnet, $host_attrs, $final, $erase);
      }
    }
  }

  /* Subfonction of reload function */
  function reload_refreshListFromCache(&$final)
  {
    foreach ($this->dhcpObjectCache as $dn => $sattrs) {
      if (empty($sattrs)) {
        continue;
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

      $tmp = preg_replace("/".$this->plugin->dhcpServiceDN."/", "", $dn);
      $indent = substr_count($tmp, ",");
      if ($type_id == "dhcpSubnet") {
        $indent--;
      }
      if ($indent > 0) {
        $spaces = "&nbsp;&nbsp;";
      } else {
        $spaces = "";
      }
      for ($i = 0; $i < $indent; $i++) {
        $spaces .= "&nbsp;&nbsp;&nbsp;&nbsp;";
      }

      /* Prepare for sorting... */
      $sortpart = explode(",", $dn);
      $sortpart = array_reverse($sortpart);
      $tmp = implode(",", $sortpart);
      $final[$dn] = $tmp."!".$spaces.$type." '".$sattrs['cn'][0]."'";
    }
  }

  function reload($erase = TRUE)
  {
    global $config;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $erase, "reload");
    /* Init LDAP and load list */
    $ldap = $config->get_ldap_link();
    $ui   = get_userinfo();
    $me   = $this->plugin->dn;
    if (empty($me)) {
      trigger_error('reload launched with empty dn');
    }

    $filter = "(&(objectClass=dhcpService)(|(dhcpPrimaryDN=$me)(dhcpSecondaryDN=$me)(dhcpServerDN=$me)(dhcpFailOverPeerDN=$me)))";

    $list   = get_list($filter, array("server"), $config->current['BASE'], array("cn"), GL_SIZELIMIT | GL_SUBSEARCH);
    $final  = array();

    foreach ($list as $value) {
      /* Set header */
      $sortpart = explode(",", $value['dn']);
      $sortpart = array_reverse($sortpart);
      $tmp = implode(",", $sortpart);

      $final[$value['dn']] = $tmp."!"._("Global options");

      /* Read all sub entries to place here */
      $ldap->cd($value['dn']);
      $ldap->search("(|(objectClass=dhcpService)(objectClass=dhcpClass)(objectClass=dhcpSubClass)(objectClass=dhcpGroup)(objectClass=dhcpPool)(objectClass=dhcpSubnet)(objectClass=dhcpSharedNetwork)(objectClass=dhcpTSigKey)(objectClass=dhcpDnsZone)(objectClass=dhcpFailOverPeer))", array());

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
      'dn'              => 'cn=dhcp,'.$this->plugin->dn,
      'cn'              => array('dhcp'),
      'objectClass'     => array('top', 'dhcpService'),
      'dhcpPrimaryDN'   => array($this->plugin->dn),
      'dhcpStatements'  => array(
        'default-lease-time 600',
        'max-lease-time 1200',
        'authoritative',
        'ddns-update-style none'
      ),
      'MODIFIED' => TRUE,
    );

    $this->value[] = array('cn=dhcp,'.$this->plugin->dn, _('Global options'));
    $this->dhcpObjectCache['cn=dhcp,'.$this->plugin->dn]  = $attrs;
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
}

class serviceDHCP extends simpleService
{
  var $objectclasses = array('dhcpServer');

  var $types = array();

  static $sectionMap = array(
    'dhcpService'       => array(
      'dhcpSharedNetwork','dhcpSubnet','dhcpGroup',
      'dhcpHost','dhcpClass','dhcpTSigKey','dhcpDnsZone'
    ),
    'dhcpClass'         => array('dhcpSubClass'),
    'dhcpSubClass'      => array(),
    'dhcpHost'          => array(),
    'dhcpGroup'         => array('dhcpHost'),
    'dhcpPool'          => array(),
    'dhcpSubnet'        => array(
      'dhcpPool','dhcpGroup',
      'dhcpHost','dhcpClass','dhcpTSigKey','dhcpDnsZone'
    ),
    'dhcpSharedNetwork' => array('dhcpSubnet', 'dhcpPool','dhcpTsigKey','dhcpDnsZone'),
    'dhcpTSigKey'       => array(),
    'dhcpDnsZone'       => array()
  );

  static $quote_option = array('domain-name');

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('DHCP sections'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new HiddenAttribute (
            'dhcpServiceDN', _('The DN of dhcpService object which contain the configuration information'),
            'dhcpServiceDN'
          ),
          new DhcpSectionsAttribute (
            '', _('The DHCP sections handled by this server'),
            'dhcpSections', FALSE
          )
        )
      ),
    );
  }

  static function plInfo()
  {
    return array(
      'plShortName'   => _('DHCP service'),
      'plDescription' => _('DHCP service'),
      'plIcon'        => 'geticon.php?context=applications&icon=dhcp&size=16',

      'plProvidedAcls' => array(
        'start'         => _('Start service'),  // Remove this to hide the start button at all.
        'stop'          => _('Stop service'),   // Remove this to hide the stop button at all.
        'restart'       => _('Restart service') // Remove this to hide the restart button at all.
      )
    );
  }

  function __construct ($dn = NULL, $parent = NULL)
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

    parent::__construct($dn, $parent);

    $this->dhcpServiceDN = 'cn=dhcp,'.$dn;
    $this->attributesAccess['dhcpSections']->setHeight(400);
  }

  function getCache()
  {
    return $this->attributesAccess['dhcpSections']->getCache();
  }

  protected function ldap_remove ()
  {
    global $config;
    /* Remove subtrees */
    $ldap   = $config->get_ldap_link();
    $cache  = $this->getCache();
    foreach ($cache as $dn => $content) {
      if ($this->attributesAccess['dhcpSections']->objectType($dn) == 'dhcpService') {
        $ldap->rmdir_recursive($dn);
        if (!$ldap->success()) {
          msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_DEL, get_class()), LDAP_ERROR);
        }
      }
    }

    return parent::ldap_remove();
  }

  protected function ldap_save($cleanup = TRUE)
  {
    global $config;
    /* Save dhcp settings */
    $ldap   = $config->get_ldap_link();
    $cache  = $this->getCache();
    foreach ($cache as $dn => $data) {
      if ($this->dn != $this->orig_dn) {
        $dn = preg_replace('/'.preg_quote($this->orig_dn, '/').'$/i', $this->dn, $dn);
      }

      /* Remove entry? */
      if (count($data) == 0) {
        /* Check if exists, then remove... */
        if ($ldap->cat($dn)) {
          $ldap->rmdir_recursive($dn);
          if (!$ldap->success()) {
            msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class()), LDAP_ERROR);
          }
        }
        continue;
      }

      /* Update dhcp option 'server-name' to actual server name */
      if ($this->dn != $this->orig_dn) {
        foreach (array('dhcpHost','dhcpSubnet','dhcpGroup','dhcpSharedNetwork') as $object) {
          if (in_array($object, $data['objectClass']) && isset($data['dhcpOption'])) {
            foreach ($data['dhcpOption'] as $key => $option) {
              if (preg_match('/^server-name /', $option)) {
                $data['dhcpOption'][$key] = 'server-name '.$this->parent->parent->getBaseObject()->cn;
                $data['MODIFIED'] = TRUE;
                break;
              }
            }
          }
        }
      }

      /* Modify existing entry? */
      if (isset($data['MODIFIED']) || $this->orig_dn != $this->dn) {
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

          if (in_array($attribute, array('dhcpPrimaryDN','dhcpSecondaryDN','dhcpServerDN','dhcpFailOverPeerDN'))) {
            foreach ($values as $v_key => $value) {
              $values[$v_key] = preg_replace('/'.preg_quote($this->orig_dn, '/').'$/i', $this->dn, $value);
            }
          }

          if (count($values)) {
            if ($attribute == 'dhcpOption') {
              foreach ($values as $key => $value) {
                $option_name  = trim(preg_replace('/[^ ]*$/', '', $value));
                $option_value = trim(preg_replace('/^[^ ]*/', '', $value));
                if (in_array($option_name, self::$quote_option)) {
                  $values[$key] = $option_name.' "'.$option_value.'"';
                }
              }
            }
            if (count($values) == 1) {
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
            msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class()), LDAP_ERROR);
          }
        } else {
          $ldap->add($attrs);
          if (!$ldap->success()) {
            msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
          }
        }
      }
    }

    if ($this->dn != $this->orig_dn) {
      $this->dhcpServiceDN = preg_replace('/'.preg_quote($this->orig_dn, '/').'$/i', $this->dn, $this->dhcpServiceDN);
    }

    /* Replace 'new' dn */
    if (preg_match('/new$/', $this->dhcpServiceDN)) {
      $this->dhcpServiceDN = preg_replace('/new$/', $this->dn, $this->dhcpServiceDN);
    }

    parent::ldap_save($cleanup);
  }
}
?>
