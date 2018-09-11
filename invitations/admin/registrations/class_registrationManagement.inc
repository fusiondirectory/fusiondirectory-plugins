<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018 FusionDirectory

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

class registrationManagement extends management
{
  public static $columns = array(
    array('ObjectTypeColumn', array()),
    array('LinkColumn',       array('attributes' => 'nameAttr',                 'label' => 'Email')),
    array('LinkColumn',       array('attributes' => 'fdRegistrationState',      'label' => 'Status')),
    array('LinkColumn',       array('attributes' => 'fdRegistrationLastChange', 'label' => 'Last change')),
    array('ActionsColumn',    array('label' => 'Actions')),
  );

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Registrations'),
      'plTitle'       => _('Registrations management'),
      'plDescription' => _('Manage registrations'),
      'plIcon'        => '',
      'plSection'     => 'accounts',
      'plManages'     => array('registration'),

      'plProvidedAcls' => array()
    );
  }
}
