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

class dhcpPool extends dhcpPlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'     => _('DHCP pool'),
      'plDescription'   => _('DHCP pool'),
      'plCategory'      => ['dhcpConfiguration'],
      'plObjectClass'   => ['dhcpPool'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    $attributesInfo = parent::getAttributesInfo();
    $attributesInfo['pool'] = [
      'name'  => _('Pool'),
      'attrs' => [
        new SetAttribute(
          new StringAttribute(
            _('Range'), _('The starting & ending IP Addresses in the range (inclusive), separated by a space; if the range only contains one address, then just the address can be specified. Each range is defined as a separate value.'),
            'dhcpRange', TRUE,
            '', '',
            '/^[0-9\.:]+(\s[0-9\.:]+)?$/'
          )
        ),
        new SetAttribute(
          new StringAttribute(
            _('Permit list'), _('This attribute contains the permit lists associated with a pool. Each permit list is defined as a separate value.'),
            'dhcpPermitList', FALSE
          )
        ),
      ]
    ];
    return $attributesInfo;
  }
}
