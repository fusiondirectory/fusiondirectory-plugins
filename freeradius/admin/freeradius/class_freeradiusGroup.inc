<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2009 GONICUS GmbH
  Copyright (C) 2012 WILLIAME Jeremy
  Copyright (C) 2011-2020  FusionDirectory

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

class freeradiusGroup extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Freeradius'),
      'plDescription'   => _('FreeRadius information'),
      'plIcon'          => 'geticon.php?context=applications&icon=freeradius&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=freeradius&size=16',
      'plObjectClass'   => ['radiusprofile'],
      'plObjectType'    => ['group'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Support 802.1x'),
        'attrs' => [
          new StringAttribute(
            _('Tunnel medium type'), _('Name of the tunnel medium type'),
            'radiusTunnelMediumType', TRUE,
            'IEEE-802'
          ),
            new StringAttribute(
            _('Tunnel type'), _('Name of the tunnel type'),
            'radiusTunnelType', TRUE,
            'VLAN'
          ),
          new StringAttribute(
            _('VLAN id'), _('VLAN identifier'),
            'radiusTunnelPrivateGroupId', FALSE
          ),
        ]
      ],
    ];
  }
}
