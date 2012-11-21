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

/*! \brief   connectivity plugin
  \author  Cajus Pollmeier <pollmeier@gonicus.de>
  \version 2.30
  \date    29.03.2005

  This class provides the functionality to read and write all attributes
  relevant for connectivity settings from/to the LDAP. It does syntax checking
  and displays the formulars required.
 */

class connectivity extends multiPlugin
{
  var $tabClass = 'CONNECTIVITYTABS';

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Connectivity"),
      "plDescription" => _("This plugin manage settings for plugins which don't have their own page"),
      "plIcon"        => "plugins/connectivity/images/plugin.png",
      "plSelfModify"  => TRUE,
      "plDepends"     => array("user"),
      "plPriority"    => 20,
      "plCategory"    => array("users"),
      "plObjectType"  => array("user"),

      "plProvidedAcls"  => array()
    );
  }
}
?>
