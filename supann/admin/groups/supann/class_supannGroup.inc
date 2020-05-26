<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2019 FusionDirectory

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

class supannGroup extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('SupAnn'),
      'plDescription'   => _('SupAnn group'),
      'plSmallIcon'     => 'geticon.php?context=applications&icon=supann&size=16',
      'plObjectClass'   => ['supannGroupe'],
      'plObjectType'    => ['group', 'ogroup-user', 'role'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Information'),
        'attrs' => [
          new GeneralizedTimeDateAttribute(
            _('End date'), _('Date at which the validity of this group ends'),
            'supannGroupeDateFin', FALSE,
            ''
          ),
          new UsersGroupsRolesAttribute(
            _('Readers'), _('Users or groups allowed to read the member list of this group'),
            'supannGroupeLecteurDN'
          ),
          new UsersGroupsRolesAttribute(
            _('Admins'), _('Users or groups allowed to edit this group'),
            'supannGroupeAdminDN'
          ),
          new SetAttribute(
            new StringAttribute(
              _('Ref ids'), _('supannRefId - IDs/links for this user on other systems'),
              'supannRefId', FALSE,
              '', '',
               // Validation regexp: it must have a prefix
              '/^{[^}]+}.+$/'
            )
          ),
        ]
      ]
    ];
  }
}
