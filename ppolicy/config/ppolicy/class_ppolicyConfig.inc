<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2015 Dhatim
  Copyright (C) 2014-2016 FusionDirectory

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

class ppolicyConfig extends simplePlugin
{
  var $objectclasses  = array("fdPpolicyPluginConf");

  static function plInfo()
  {
    return array(
      "plShortName"     => _("Ppolicy plugin configuration"),
      "plDescription"   => _("FusionDirectory ppolicy plugin configuration"),
      "plCategory"      => array("configuration"),
      "plObjectType"    => array("smallConfig"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Ppolicy plugin'),
        'attrs' => array(
          new StringAttribute (
            _('Ppolicy RDN'), _('Branch in which ppolicies will be stored'),
            'fdPpolicyRDN', TRUE,
            'ou=ppolicies'
          ),
          new StringAttribute (
            _('Default ppolicy cn'), _('What you put as default ppolicy in the overlay config'),
            'fdPpolicyDefaultCn', FALSE,
            'default'
          ),
        )
      ),
    );
  }
}
?>
