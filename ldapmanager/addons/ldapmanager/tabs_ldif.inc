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

class ldiftab extends tabs
{
  function ldiftab($config, $data, $dn, $cat = '', $copied_object = NULL)
  {
    parent::tabs($config, $data, $dn, '', $copied_object);
    $this->set_acl_base($config->current['BASE']);
  }

  function save()
  {
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('LDAP import/export'),
      'plDescription' => _('Allows the import or export of the ldap tree'),
      'plIcon'        => 'plugins/ldapmanager/images/icon.png',
      'plSection'     => 'addons',
      'plPriority'    => 20,
      'plCategory'    => array('ldapmanager' => array('objectClass' => 'none', 'description' => _('Ldap manager'))),

      'plProvidedAcls' => array()
    );
  }
}
?>
