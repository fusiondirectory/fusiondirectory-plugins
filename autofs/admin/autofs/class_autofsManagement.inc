<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2019  FusionDirectory

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

class autofsManagement extends management
{
  /* Default columns */
  public static $columns = array(
    array('ObjectTypeColumn',     array()),
    array('LinkColumn',           array('attributes' => 'nameAttr',             'label' => 'Name')),
    array('LinkColumn',           array('attributes' => 'nisMapName',           'label' => 'Mount point')),
    array('LinkColumn',           array('attributes' => 'nisMapEntry',          'label' => 'Automount entry')),
    array('ActionsColumn',        array('label' => 'Actions')),
  );

  static function plInfo ()
  {
    return array(
      'plShortName'   => _('Auto fs'),
      'plTitle'       => _('Auto fs management'),
      'plDescription' => _('Manage auto fs mount points and directories'),
      'plIcon'        => 'geticon.php?context=applications&icon=autofs&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 5,
      'plManages'     => array('nisMap', 'nisObject'),
    );
  }
}
