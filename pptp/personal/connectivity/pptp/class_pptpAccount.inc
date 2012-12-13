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

class pptpAccount extends simplePlugin
{
  /* Definitions */
  var $displayHeader  = TRUE;
  var $objectclasses  = array("pptpServerAccount");

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array();
  }

  static function plInfo()
  {
    return array(
      "plShortName"     => _("PPTP"),
      "plDescription"   => _("PPTP account")."&nbsp;("._("Connectivity addon").")",
      "plSelfModify"    => TRUE,
      "plPriority"      => 28,                                 // Position in tabs
      "plCategory"      => array("users"),
      "plObjectType"    => array("connectivity"),
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->templatePath = get_template_path('pptp.tpl', TRUE, dirname(__FILE__));
  }
}

?>
