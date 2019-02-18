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

class serviceTerminal extends simpleService
{
  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Terminal service'),
      'plDescription' => _('Terminal service').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=devices&icon=terminal&size=16',
      'plObjectClass' => array('goTerminalServer'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
      return array (
      'section1' => array (
        'name'  => _('Terminal service'),
        'attrs' => array (
          new BooleanAttribute(
            _('Temporary disable login'),
            _('Disables the login on this terminal server'),
            'goXdmcpIsEnabled', FALSE,
            FALSE,
            '',
            1,
            0
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Supported session types'),
              _('The session types supported by this terminal server'),
              'gotoSessionType', FALSE,
              array('XDMCP', 'LDM', 'SHELL', 'TELNET', 'RDP'),
              '',
              array(_('XDMCP'), _('LDM'), _('Shell'), _('Telnet'), _('Windows RDP'))
            )
          )
        )
      )
    );
  }
}

?>
