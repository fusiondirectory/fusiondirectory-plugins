<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class serviceDHCP extends simpleService
{
  var $objectclasses = array('dhcpServer');

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('DHCP sections'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new SelectAttribute (
            'dhcpServiceDN', _('The DN of dhcpService object which contain the configuration information'),
            'dhcpServiceDN'
          )
        )
      ),
    );
  }

  static function plInfo()
  {
    return array(
      'plShortName'   => _('DHCP service'),
      'plDescription' => _('DHCP service'),
      'plIcon'        => 'geticon.php?context=applications&icon=dhcp&size=16',

      'plProvidedAcls' => array(
        'start'         => _('Start service'),  // Remove this to hide the start button at all.
        'stop'          => _('Stop service'),   // Remove this to hide the stop button at all.
        'restart'       => _('Restart service') // Remove this to hide the restart button at all.
      )
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->dhcpServiceDN = 'cn=dhcp,'.$dn;
  }
}
?>
