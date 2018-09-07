<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2018  FusionDirectory

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

class dhcpSubnet extends dhcpPlugin
{
  public $objectclasses = array('dhcpSubnet');

  public static function plInfo()
  {
    return array(
      'plShortName'     => _('DHCP subnet'),
      'plDescription'   => _('DHCP subnet'),
      'plCategory'      => array('dhcpConfiguration'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    $attributesInfo = parent::getAttributesInfo();
    $attributesInfo['subnet'] = array(
      'name'  => _('Subnet'),
      'attrs' => array(
        new IntAttribute (
          _('Mask length'), _('The subnet mask length for the subnet. The mask can be easily computed from this length.'),
          'dhcpNetMask', TRUE,
          0, 255, 24
        ),
        new SetAttribute (
          new StringAttribute (
            _('Range'), _('The starting & ending IP Addresses in the range (inclusive), separated by a space; if the range only contains one address, then just the address can be specified. Each range is defined as a separate value.'),
            'dhcpRange', FALSE,
            '', '',
            '/^[0-9\.:]+(\s[0-9\.:]+)?$/'
          )
        ),
      )
    );
    $attributesInfo['main']['attrs'][0] = new IPAttribute(
      _('Network address'), _('Network address of this subnet'),
      'cn', TRUE
    );
    return $attributesInfo;
  }
}
