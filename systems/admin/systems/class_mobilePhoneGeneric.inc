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

class mobilePhoneGeneric extends ipHostPlugin
{
  var $inheritance = ['gosaGroupOfNames' => 'member'];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Mobile phone'),
      'plDescription' => _('Mobile phone information'),
      'plObjectClass' => ['fdMobilePhone', 'ieee802Device'],
      'plFilter'      => '(objectClass=fdMobilePhone)',
      'plObjectType'  => [
        'mobilePhone' => [
          'name'        => _('Mobile phone'),
          'description' => _('Mobile phone'),
          'icon'        => 'geticon.php?context=devices&icon=phone&size=16',
          'ou'          => get_ou('mobilePhoneRDN'),
        ]
      ],
      'plSearchAttrs' => ['description', 'ipHostNumber', 'macAddress'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('mobilePhoneRDN')),
          new HostNameAttribute(
            _('Name'), _('The name of the mobile phone'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('A short description of the mobile phone'),
            'description', FALSE
          ),
          new HiddenArrayAttribute('ipHostNumber', FALSE),
          new HiddenArrayAttribute('macAddress', FALSE),
        ]
      ],
      'phone' => [
        'name'  => _('Phone'),
        'attrs' => [
          new StringAttribute(
            _('Serial Number'), _('The serial number of the mobile phone'),
            'serialNumber', FALSE
          ),
          new StringAttribute(
            _('IMEI Number'), _('The IMEI number of the mobile phone'),
            'fdMobileIMEI', FALSE, '', '',
            '/[[:digit:]]+/'
          ),
          new StringAttribute(
            _('OS'), _('The Operating System installed on this phone'),
            'fdMobileOS', FALSE
          ),
        ]
      ],
      'simcard' => [
        'name'  => _('SimCard'),
        'attrs' => [
          new StringAttribute(
            _('Telephone Number'), _('The telephone number of the mobile phone'),
            'telephoneNumber', FALSE
          ),
          new StringAttribute(
            _('PUK Number'), _('The PUK number of the simcard in this mobile phone'),
            'fdMobilePUK', FALSE, '', '',
            '/[[:digit:]]+/'
          ),
        ]
      ],
    ];
  }
}
