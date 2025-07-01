<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
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

class systemManagement extends management
{
  public static $skipTemplates = FALSE;

  public $siActive = FALSE;

  public static $columns = [
    ['ObjectTypeColumn',     []],
    ['LinkColumn',           ['attributes' => 'nameAttr',     'label' => 'Name']],
    ['IpColumn',             ['attributes' => 'ipHostNumber', 'label' => 'IP']],
    ['Column',               ['attributes' => 'macAddress',   'label' => 'MAC']],
    ['LinkColumn',           ['attributes' => 'description',  'label' => 'Description']],
    ['SystemServicesColumn', ['label' => 'Services']],
    ['SystemReleaseColumn',  ['label' => 'Release']],
    ['ActionsColumn',        ['label' => 'Actions']],
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Systems'),
      'plTitle'       => _('Systems Management'),
      'plDescription' => _('Manage servers, computers, printers, phones and other devices'),
      'plIcon'        => 'geticon.php?context=devices&icon=computer&size=48',
      'plSection'     => ['systems' => ['name' => _('Systems'), 'priority' => 3]],
      'plPriority'    => 0,
      'plManages'     => ['server','workstation','terminal','printer','component','phone','mobilePhone'],
    ];
  }
}
