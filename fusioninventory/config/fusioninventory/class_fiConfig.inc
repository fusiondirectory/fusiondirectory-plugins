<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2018 FusionDirectory

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

class fiConfig extends simplePlugin
{
  var $objectclasses  = array("fdInventoryPluginConf");

  static function plInfo()
  {
    return array(
      "plShortName"     => _("FusionInventory configuration"),
      "plDescription"   => _("FusionDirectory FusionInventory plugin configuration"),
      "plCategory"      => array("configuration"),
      "plObjectType"    => array("smallConfig"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('FusionInventory'),
        'attrs' => array(
          new StringAttribute (
            _('Inventory RDN'), _('Branch in which inventory objects will be stored'),
            'fdInventoryRDN', TRUE,
            'ou=inventory'
          ),
          new SelectAttribute (
            _('Inventory matching'), _('Criteria to link an inventory result to a system'),
            'fdInventoryMatching', TRUE,
            array('mac','ip','both','either'), 'mac',
            array(_('Mac address'), _('IP address'), _('Both'), _('IP or Mac address'))
          ),
        )
      ),
    );
  }
}
?>
