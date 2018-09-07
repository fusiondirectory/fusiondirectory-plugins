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

class dhcpTSigKey extends dhcpPlugin
{
  public $objectclasses = array('dhcpTSigKey');

  public static function plInfo()
  {
    return array(
      'plShortName'     => _('DHCP DNS update key'),
      'plDescription'   => _('DHCP DNS update key'),
      'plCategory'      => array('dhcpConfiguration'),
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
          new SelectAttribute (
            _('Algorithm'), _('Algorithm to generate TSIG Key'),
            'dhcpKeyAlgorithm', TRUE,
            array('HMAC-MD5','RSAMD5','RSASHA1','DSA','DH')
          ),
          new StringAttribute (
            _('Secret'), _('Secret to generate TSIG Key'),
            'dhcpKeySecret', TRUE
          ),
          new TextAreaAttribute (
            _('Comments'), _('Comments about this DHCP object'),
            'dhcpComments', FALSE
          ),
        )
      )
    );
  }
}
