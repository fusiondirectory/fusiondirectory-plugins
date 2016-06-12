<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2012-2016  FusionDirectory

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

class componentGeneric extends systemSimplePluginGlue
{
  var $objectclasses = array("top", "device", "ipHost", "ieee802Device");

  var $mainTab = TRUE;

  var $netConfigDNS;

  var $preInitAttributes = array('cn', 'networkSettings');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Component'),
      'plDescription' => _('Component information'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array(
        'component' => array(
          'name'        => _('Network device'),
          'description' => _('Network device'),
          'filter'      => '(&(objectClass=ieee802Device)(objectClass=device)(objectClass=ipHost))',
          'icon'        => 'geticon.php?context=devices&icon=network-device&size=16',
          'ou'          => get_ou('componentRDN'),
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute (
            _('Name'), _('The name of the component'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('A short description of the component'),
            'description', FALSE
          ),
          new BaseSelectorAttribute(get_ou("componentRDN")),
        )
      ),
      'network' => array(
        'name'      => _('Network settings'),
        'attrs'     => array(new NetworkSettingsAttribute()),
        'template'  => get_template_path('network_section.tpl', TRUE, dirname(__FILE__)),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL)
  {
    parent::__construct($dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'cn='.ldap_escape_dn($this->cn).','.get_ou('componentRDN').$this->base;
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();
    $this->netConfigDNS->resetCopyInfos();
  }

  function set_acl_base($base)
  {
    plugin::set_acl_base($base);
    $this->netConfigDNS->set_acl_base($base);
  }

  function set_acl_category($cat)
  {
    plugin::set_acl_category($cat);
    $this->netConfigDNS->set_acl_category($cat);
  }

  function remove_from_parent()
  {
    global $config;
    if (!$this->initially_was_account) {
      return;
    }

    if ($this->acl_is_removeable()) {
      $this->netConfigDNS->remove_from_parent();

      parent::remove_from_parent();

      /* Delete references to object groups */
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $ldap->search("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
      while ($ldap->fetch()) {
        $og = new ogroup($ldap->getDN());
        unset($og->member[$this->dn]);
        $og->save();
      }
    }
  }

  protected function ldap_save($cleanup = TRUE)
  {
    /* If this is a new Object IP & Mac aren't set.
     * IP & Mac are msut attributes, so we set this values by here. */
    if ($this->orig_dn == 'new') {
      $this->attrs['ipHostNumber']  = $this->netConfigDNS->ipHostNumber;
      $this->attrs['macAddress']    = $this->netConfigDNS->macAddress;
    }

    parent::ldap_save($cleanup);
    $this->netConfigDNS->cn = $this->cn;
    $this->netConfigDNS->save();
  }

  function is_modal_dialog()
  {
    return (isset($this->netConfigDNS->dialog) && $this->netConfigDNS->dialog);
  }
}
?>
