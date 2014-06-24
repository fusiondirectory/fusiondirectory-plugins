<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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

class nisMap extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses  = array('nisMap', 'top');

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Mount point'),
      'plDescription'   => _('Autofs mount point'),
      'plObjectType'    => array('nisMap' => array(
        'name'        => _('Mount point'),
        'filter'      => 'objectClass=nisMap',
        'ou'          => get_ou('autofsRDN'),
        'icon'        => 'plugins/autofs/images/iconMiniNisMap.png',
        'mainAttr'    => 'nisMapName',
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Mount point'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('autofsRDN')),
          new HostNameAttribute (
            _('Name'), _('Name of the mount point'),
            'nisMapName', TRUE
          )
        )
      )
    );
  }

  function compute_dn()
  {
    return "nisMapName=".$this->nisMapName.",".get_ou("autofsRDN").$this->base;
  }
}


?>
