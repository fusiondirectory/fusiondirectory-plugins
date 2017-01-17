<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016 FusionDirectory

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

class phoneGeneric extends simplePlugin
{
  var $mainTab = TRUE;

  var $netConfigDNS;

  var $objectclasses = array('fdPhone', 'ipHost', 'ieee802Device');

  var $inheritance = array('gosaGroupOfNames' => 'member');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Phone'),
      'plDescription' => _('Phone information'),
      'plObjectType'  => array(
        'phone' => array(
          'name'        => _('Phone'),
          'description' => _('Phone hardware'),
          'filter'      => 'objectClass=fdPhone',
          'icon'        => 'geticon.php?context=devices&icon=telephone&size=16',
          'ou'          => get_ou('phoneRDN'),
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute (
            _('Name'), _('The name of the phone'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('A short description of the phone'),
            'description', FALSE
          ),
          new BaseSelectorAttribute(get_ou('phoneRDN')),
        )
      ),
      'phone' => array(
        'name'  => _('Phone'),
        'attrs' => array(
          new StringAttribute (
            _('Serial Number'), _('The serial number of the phone'),
            'serialNumber', FALSE
          ),
          new StringAttribute (
            _('Telephone Number'), _('The telephone number of the phone'),
            'telephoneNumber', FALSE
          ),
        )
      ),
      'network' => array(
        'name'      => _('Network settings'),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
        'attrs'     => array(
          new SetAttribute(
            new IPAttribute(
              _('IP address'), _('IP addresses this system uses (v4 or v6)'),
              'ipHostNumber', FALSE
            )
          ),
          new MacAddressAttribute(
            _('Mac address'), _('Mac address of this system'),
            'macAddress', FALSE
          ),
        ),
      ),
    );
  }
}
?>
