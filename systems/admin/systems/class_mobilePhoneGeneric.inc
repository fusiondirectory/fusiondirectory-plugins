<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016 FusionDirectory

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

class mobilePhoneGeneric extends systemSimplePluginGlue
{
  var $mainTab = TRUE;

  var $netConfigDNS;

  var $objectclasses = array("top", "fdMobilePhone");

  var $inheritance = array("gosaGroupOfNames" => "member");

  var $preInitAttributes = array('cn', 'networkSettings');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Mobile phone'),
      'plDescription' => _('Mobile phone information'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array(
        'mobilePhone' => array(
          'name'        => _('Mobile phone'),
          'description' => _('Mobile phone'),
          'filter'      => 'objectClass=fdMobilePhone',
          'icon'        => 'geticon.php?context=devices&icon=phone&size=16',
          'ou'          => get_ou('mobilePhoneRDN'),
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
            _('Name'), _('The name of the mobile phone'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('A short description of the mobile phone'),
            'description', FALSE
          ),
          new BaseSelectorAttribute(get_ou('mobilePhoneRDN')),
        )
      ),
      'phone' => array(
        'name'  => _('Phone'),
        'attrs' => array(
          new StringAttribute (
            _('Serial Number'), _('The serial number of the mobile phone'),
            'serialNumber', FALSE
          ),
          new StringAttribute (
            _('IMEI Number'), _('The IMEI number of the mobile phone'),
            'fdMobileIMEI', FALSE, '', '',
            '/[[:digit:]]+/'
          ),
          new StringAttribute (
            _('OS'), _('The Operating System installed on this phone'),
            'fdMobileOS', FALSE
          ),
        )
      ),
      'simcard' => array(
        'name'  => _('SimCard'),
        'attrs' => array(
          new StringAttribute (
            _('Telephone Number'), _('The telephone number of the mobile phone'),
            'telephoneNumber', FALSE
          ),
          new StringAttribute (
            _('PUK Number'), _('The PUK number of the simcard in this mobile phone'),
            'fdMobilePUK', FALSE, '', '',
            '/[[:digit:]]+/'
          ),
        )
      ),
      'network' => array(
        'name'      => _('Network settings'),
        'attrs'     => array(new NetworkSettingsAttribute('cn', array('ipHost', 'ieee802Device'))),
        'template'  => get_template_path('network_section.tpl', TRUE, dirname(__FILE__)),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
    $this->netConfigDNS->IPisMust   = FALSE;
    $this->netConfigDNS->MACisMust  = FALSE;
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
    if (!$this->initially_was_account) {
      return;
    }

    if ($this->acl_is_removeable()) {
      $this->netConfigDNS->remove_from_parent();

      parent::remove_from_parent();

      /* Delete references to object groups */
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
      while ($ldap->fetch()) {
        $og = new ogroup($this->config, $ldap->getDN());
        unset($og->member[$this->dn]);
        $og->save();
      }
    }
  }

  protected function ldap_save($cleanup = TRUE)
  {
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
