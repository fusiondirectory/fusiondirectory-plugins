<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2019 FusionDirectory

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

class dsaManagement extends management
{
  public static function plInfo ()
  {
    return array(
      'plShortName'   => _('DSA'),
      'plTitle'       => _('DSA management'),
      'plDescription' => _('Manage simple security objects'),
      'plIcon'        => 'geticon.php?context=applications&icon=dsa&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 55,
      'plCategory'    => array('dsa' => array('description'  => _('DSA'),
                                              'objectClass'  => array('simpleSecurityObject'))),
      'plManages'     => array('simpleSecurityObject'),
    );
  }
}
