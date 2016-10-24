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

class dhcpHost extends dhcpPlugin
{
  public $objectclasses = array('dhcpHost');

  static function getAttributesInfo ()
  {
    $attributesInfo = parent::getAttributesInfo();
    $attributesInfo['host'] = array(
      'name'  => _('Host'),
      'attrs' => array(
        new CompositeAttribute(
          _('The client hardware address'), 'dhcpHWAddress',
          array(
            new SelectAttribute (
              '', _('The hardware address type'),
              'dhcpHWAddress_type', TRUE,
              array(  'ethernet',   'fddi',   'token-ring'), 'ethernet',
              array(_('Ethernet'),_('FDDI'),_('Token Ring'))
            ),
            new StringAttribute (
              '', _('The client hardware address'),
              'dhcpHWAddress_address', FALSE
            ),
          ),
          '/^([^ ]+) ([^ ]+)$/',
          '%s %s',
          '',
          _('Hardware address')
        )
      )
    );
    $attributesInfo['host']['attrs'][0]->setLinearRendering(TRUE);
    $attributesInfo['main']['attrs'][2]->setDefaultValue(
      array(
        'host-name cn',
        'fixed-address ip',
      )
    );
    return $attributesInfo;
  }
}
