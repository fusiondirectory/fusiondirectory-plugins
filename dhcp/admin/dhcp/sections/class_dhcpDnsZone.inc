<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

class dhcpDnsZone extends dhcpPlugin
{
  public static function plInfo()
  {
    return array(
      'plShortName'     => _('DHCP DNS update zone'),
      'plDescription'   => _('DHCP DNS update zone'),
      'plCategory'      => array('dhcpConfiguration'),
      'plObjectClass'   => array('dhcpDnsZone'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('DHCP'),
        'attrs' => array(
          new HostNameAttribute (
            _('Name'), _('Name of this DHCP configuration'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('DNS Zone Server'), _('Master server of the DNS Zone'),
            'dhcpDnsZoneServer', TRUE
          ),
          new SelectAttribute (
            _('Tsig key'), _('The TSig key DN'),
            'dhcpKeyDN', FALSE
          ),
          new TextAreaAttribute (
            _('Comments'), _('Comments about this DHCP object'),
            'dhcpComments', FALSE
          ),
        )
      )
    );
  }

  function __construct ($plugin, $dn, $attrs = NULL)
  {
    global $config;
    parent::__construct($plugin, $dn, $attrs);
    $keys = array();
    $ldap = $config->get_ldap_link();
    $ldap->search('(objectClass=dhcpTSigKey)', array('dn','cn'));
    while ($attrs = $ldap->fetch()) {
      $keys[$attrs['dn']] = $attrs['cn'][0];
    }
    $this->attributesAccess['dhcpKeyDN']->setChoices(array_keys($keys), array_values($keys));
  }
}
