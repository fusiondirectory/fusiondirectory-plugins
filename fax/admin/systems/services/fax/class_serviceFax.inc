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

class goFaxServer extends simpleService {

  var $cli_summary       = "This plugin is used within the ServerService Plugin \nand indicates that this server supports fax informations.";
  var $cli_description   = "Some longer text\nfor help";
  var $cli_parameters    = array("one" => "one is nice", "two" => "two is even better");
  /* This plugin only writes its objectClass */

  var $objectclasses     =  array("goFaxServer");
  var $StatusFlag        = "goFaxServerStatus";
  var $goFaxServerStatus = "";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("FAX service"),
          "plDescription" => _("FAX service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 99,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),
          "plProvidedAcls"=> parent::generatePlProvidedAcls(self::getAttributesInfo())
          ));
  }

    /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
      return array (
      'section1' => array (
        'name'  => _('FAX database information'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('FAX DB user'),
            '',
            'goFaxAdmin',
            TRUE
          ),
          new PasswordAttribute (
            _('Password'),
            '',
            'goFaxPassword',
            TRUE
          )
        )
      )
    );
  }
}

?>
