<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2010-2012 Antoine Gallavardin
  Copyright (C) 2013-2025 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class supannAccountExternal extends simplePlugin
{
  protected $displayHeader = TRUE;

  protected $entitesCache = [];

  static function plInfo (): array
  {
    global $config;
    if ($config->get_cfg_value('SupannShowEmployee', 'TRUE') !== 'TRUE') {
      return [];
    }

    return [
      'plShortName'   => _('SupAnn External'),
      'plDescription' => _('SupAnn External Account Management'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 14,
      'plObjectClass' => ['fdSupannExternal'],
      'plObjectType'  => ['user'],
      'plDepends' => ['supannAccount'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'personnal' => [
        'name'  => _('External profile'),
        'attrs' => [
          new GeneralizedTimeDateAttribute(
            _('End contract'), _('date of the end of the contract'),
            'supannExtDateFin', FALSE,
            ''
          ),
        ]
      ]
    ];
  }


}
