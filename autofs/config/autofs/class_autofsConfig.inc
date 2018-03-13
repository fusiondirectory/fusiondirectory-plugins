<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016 FusionDirectory

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

class autofsConfig extends simplePlugin
{
  var $objectclasses  = array("fdAutofsPluginConf");

  static function plInfo()
  {
    return array(
      "plShortName"     => _("Autofs configuration"),
      "plDescription"   => _("FusionDirectory autofs plugin configuration"),
      "plCategory"      => array("configuration"),
      "plObjectType"    => array("smallConfig"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('AutoFS'),
        'attrs' => array(
          new StringAttribute (
            _('AutoFS RDN'), _('Branch in which automount info will be stored'),
            'fdAutofsRDN', TRUE,
            'ou=autofs'
          ),
        )
      ),
    );
  }
}
?>
