<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2005 Guillaume Delecourt
  Copuright (C) 2005 Benoit Mortier
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

class phpscheduleitAccount extends simplePlugin
{
  /* Definitions */
  var $displayHeader = TRUE;
  var $plHeadline= "PHPscheduleit";
  var $plDescription= "This does something";
  var $objectclasses= array("phpscheduleitAccount");

  static function plInfo()
  {
    return (array(
          "plShortName"     => _("PHP Schedule it"),
          "plDescription"   => _("PHP Schedule it settings")."&nbsp;("._("Connectivity addon").")",
          "plSelfModify"    => TRUE,
          "plDepends"       => array("user"),
          "plPriority"      => 29,                                 // Position in tabs
          "plSection"       => array("personal" => _("My account")),
          "plCategory"      => array("users"),
          "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
          ));
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
    // Attributes are grouped by section
    'section1' => array(
        'name'  => _('PHPscheduleit account'),
        'attrs' => array(
          new StringAttribute(
            _('Login'),              // Label of the attribute
            _('Phpscheduleit name account'),              // Description
            'phpscheduleitAccountLogin',                 // LDAP name
            TRUE,                            // Mandatory
            ''                            // default value
          )
        )
      )
    );
  }
}

?>
