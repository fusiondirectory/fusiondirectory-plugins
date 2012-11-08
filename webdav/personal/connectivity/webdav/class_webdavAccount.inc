<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class webdavAccount extends simplePlugin
{
  /* Definitions */
  var $displayHeader = TRUE;
  var $objectclasses= array("gosaWebdavAccount");

  static function plInfo()
  {
    return array(
      "plShortName"     => _("WebDAV"),
      "plDescription"   => _("WebDAV account")."&nbsp;("._("Connectivity addon").")",
      "plSelfModify"    => TRUE,
      "plCategory"      => array("users"),
      "plObjectType"    => array("connectivity"),
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function execute()
  {
    $smarty = get_smarty();
    parent::execute();
    if ($this->displayPlugin) {
      return $this->header.$smarty->fetch(get_template_path('webdavAccount.tpl', TRUE, dirname(__FILE__)));
    } else {
      return $this->header;
    }
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array();
  }

}
?>
