<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014 FusionDirectory

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

class systemServers extends simplePlugin
{
  var $objectclasses = array('fdSystemServers');

  var $inheritance = array('gosaGroupOfNames' => 'member');

  static function getAttributesInfo ()
  {
    return array(
      'servers' => array(
        'name'  => _('Servers'),
        'icon'  => 'geticon.php?context=devices&icon=server&size=16',
        'attrs' => array(
          new SelectAttribute (
            _('Syslog server'), _('The syslog server the system should be using'),
            'fdSyslogServer', FALSE
          ),
          new SetAttribute (
            new SelectAttribute (
              _('NTP servers'), _('The NTP servers for this system'),
              'fdNtpServer', FALSE
            )
          ),
        )
      ),
    );
  }

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Servers'),
      'plDescription'   => _('Servers this system is using'),
      'plPriority'      => 3,
      'plObjectType'    => array('server', 'workstation', 'terminal', 'winstation', 'ogroup-dynamic'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->init_server_section();
  }

  protected function init_server_section()
  {
    /* Create available ntp options */
    $tmp = array();
    foreach ($this->config->data['SERVERS']['NTP'] as $key => $server) {
      if ($server == 'default') {
        continue;
      }
      $tmp[] = $server;
    }
    $this->attributesAccess['fdNtpServer']->attribute->setChoices($tmp);

    /* Create an array of all Syslog servers */
    $tmp        = $this->config->data['SERVERS']['SYSLOG'];
    $tmp_output = $tmp;
    if (($key = array_search('default', $tmp)) !== FALSE) {
      $tmp[$key] = '';
    }
    $this->attributesAccess['fdSyslogServer']->setChoices($tmp, $tmp_output);
  }
}

?>
