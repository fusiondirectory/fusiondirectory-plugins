<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2018  FusionDirectory

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

class fiInventoryAgent extends simplePlugin
{
  var $objectclasses  = array('fusionInventoryAgent');
  var $displayHeader  = TRUE;

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Configuration'),
        'attrs' => array(
          new StringAttribute(
            _('Server'), _('Server which FI will send inventory infos'),
            'fiAgentServer', FALSE
          ),
          new IntAttribute(
            _('Delay time'), _('Maximum initial delay before first target, in seconds'),
            'fiAgentDelaytime', FALSE,
            0, FALSE
          ),
          new IntAttribute(
            _('Wait'), _('Maximum delay between each targets, in seconds'),
            'fiAgentWait', FALSE,
            0, FALSE
          ),
        ),
      ),
      'http' => array(
        'name'  => _('Web Interface'),
        'attrs' => array(
          new BooleanAttribute(
            _('Disable web interface'), _('Do not use web interface'),
            'fiAgentNoHttpd', FALSE
          ),
          new IPv4Attribute(
            _('IP'), _('Network interface to listen to'),
            'fiAgentHttpdIp', FALSE
          ),
          new IntAttribute(
            _('Port'), _('Maximum delay between each targets, in seconds'),
            'fiAgentHttpdPort', FALSE,
            0, FALSE
          ),
          new IPv4Attribute(
            _('Trust'), _('IPs allowed to launch inventory task through web interface'),
            'fiAgentHttpdTrust', FALSE
          ),
        )
      )
    );
  }

  /* Return plugin information */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('FusionInventory'),
      'plDescription' => _('FusionInventory agent configuration'),
      'plPriority'    => 90,
      'plObjectType'  => array('workstation', 'server'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }
}
?>
