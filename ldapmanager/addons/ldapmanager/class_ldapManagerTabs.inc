<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class ldapManagerTabs extends simpleTabs_noSpecial
{
  static function plInfo ()
  {
    return array(
      'plShortName'   => _('LDAP import/export'),
      'plDescription' => _('Allows the import or export of the ldap tree'),
      'plIcon'        => 'geticon.php?context=applications&icon=ldapmanager&size=48',
      'plSection'     => 'conf',
      'plPriority'    => 10,
      'plCategory'    => array('ldapmanager' => array('objectClass' => 'none', 'description' => _('Ldap manager'))),
    );
  }

  function save ()
  {
  }

  static function mainInc ()
  {
    simplePlugin::mainInc(get_called_class(), 'new', TRUE, FALSE, 'ldapmanager');
  }
}
