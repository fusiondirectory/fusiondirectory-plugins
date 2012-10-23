<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class goTerminalServer extends simpleService{

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goTerminalServer");

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Terminal service"),
      "plDescription" => _("Terminal service")." ("._("Services").")",
      "plSelfModify"  => FALSE,
      "plSection"     => array("administration"),
      "plCategory"    => array("server"),
      "plProvidedAcls"=> parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
      return array (
      'section1' => array (
        'name'  => '<img src="images/rightarrow.png" /> '._('Terminal service'),
        'attrs' => array (
          new BooleanAttribute (
            _('Temporary disable login'),
            _(' '),
            'goXdmcpIsEnabled',
            FALSE,
            FALSE,
            '',
            1,
            0
          ),
          new SetAttribute (
            new SelectAttribute (
              _('Supported session types'),
              _(' '),
              'gotoSessionType',
              FALSE,
              array("LDM","XDMCP", "SHELL", "RDP", "TELNET"),
              '',
              array("LDM","XDMCP", "SHELL", "RDP", "TELNET")
            )
          )
        )
      )
    );
  }
}

?>
