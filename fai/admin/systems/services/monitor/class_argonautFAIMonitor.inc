<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015  FusionDirectory

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

class argonautFAIMonitor extends simpleService
{
  var $objectclasses  = array("argonautFAIMonitorConfig");

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Argonaut FAI monitor'),
      'plDescription'   => _('Argonaut FAI monitor settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=fai&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array (
    'config' => array (
        'name'  => _('Argonaut FAI monitor settings'),
        'attrs' => array (
          new IntAttribute (
            _('Port'), _('The port argonaut-fai-monitor should listen on. Default is 4711.'),
            'argonautFAIMonitorPort', FALSE,
            0, FALSE, 4711
          ),
          new IntAttribute (
            _('Timeout'), _('The timeout for bad clients. 0 to disable.'),
            'argonautFAIMonitorTimeout', FALSE,
            0, FALSE, 0
          ),
          new StringAttribute (
            _('CA certificate'), _('Path to the CA certificate file on Argonaut FAI monitor server'),
            'argonautFAIMonitorCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
          new StringAttribute (
            _('Log directory'), _('Directory in which argonaut-fai-monitor will store its log'),
            'argonautFAIMonitorLogDir', TRUE,
            '/var/log/argonaut'
          ),
        )
      )
    );
  }
}

?>
