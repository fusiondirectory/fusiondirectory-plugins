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

class ldif extends plugin
{
  /* Definitions */
  var $plHeadline= "LDAP manager";
  var $plDescription= "This plugin allows the management of the ldap tree";
  var $plIcon = "plugins/ldapmanager/images/ldif.png";

  /* attribute list for save action */
  var $attributes= array();
  var $objectclasses= array();
  var $ldif;

  function ldif (&$config)
  {
    /* Include config object */
    $this->ldif= new ldiftab($config, $config->data['TABS']['LDIFTAB'], "");
  }

  function execute()
  {
	/* Call parent execute */
	plugin::execute();

    /* Show main page */
    return ($this->ldif->execute());
  }

  /* Return plugin informations for acl handling
     #FIXME You can only read attributes within this ldif import plugin */
  static function plInfo()
  {
    return (array(
        "plShortName"   => _("Ldap manager"),
        "plDescription" => _("Ldap manager")."&nbsp;("._("import")."/"._("export").")",
        "plSelfModify"  => FALSE,
        "plDepends"     => array(),
        "plPriority"    => 1,
        "plSection"     => array("addon"),
        "plCategory"    => array("ldapmanager" => array("objectClass" => "none", "description" => _("Ldap manager"))),

        "plProvidedAcls" => array()
        ));
  }



}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
