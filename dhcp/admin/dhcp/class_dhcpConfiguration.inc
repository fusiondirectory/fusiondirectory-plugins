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

class dhcpConfiguration extends simplePlugin
{
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
      'plObjectClass' => array('dhcpService'),
      'plObjectType'  => array('dhcpConfiguration' => array(
        'name'        => _('DHCP configuration'),
        'ou'          => get_ou('dhcpRDN'),
        'icon'        => 'geticon.php?context=applications&icon=dhcp&size=16'
      )),
      'plForeignKeys'   => array(
        'dhcpSections' => array(
          array('serverGeneric', 'dn', '(|(dhcpPrimaryDN=%oldvalue%)(dhcpSecondaryDN=%oldvalue%))'),
        ),
      ),
      'plSearchAttrs' => array('description'),

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
