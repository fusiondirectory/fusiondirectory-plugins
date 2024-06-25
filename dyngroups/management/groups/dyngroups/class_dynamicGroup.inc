<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory
  Copyright (C) 2020  Worteks

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

class dynamicGroup extends simplePlugin
{

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Dynamic group'),
      'plDescription' => _('Dynamic group information'),
      'plObjectClass' => ['groupOfURLs','extensibleObject'],
      'plObjectType'  => ['dyngroup' => [
        'name'        => _('Dynamic group'),
        'description' => _('Dynamic group'),
        'ou'          => get_ou('ogroupRDN'),
        'icon' => 'geticon.php?context=types&icon=dyngroup&size=16',
      ]],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    $memberAttribute = new SetAttribute(
      new StringAttribute(
        '', _('Group members'),
        'member', FALSE
      )
    );
    $memberAttribute->setDisabled(TRUE);

    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('ogroupRDN')),
          new StringAttribute(
            _('Name'), _('Name of this group'),
            'cn', TRUE,
            '', '', (strict_uid_mode() ? '/^[a-z0-9_-]+$/' : '/^[a-z0-9_.-]+$/i')
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of this group'),
            'description', FALSE
          )
        ]
      ],
      'members' => [
        'name'  => _('Group members'),
        'attrs' => [
          $memberAttribute
        ]
      ],
      'member URL' => [
        'name'  => _('member URL'),
        'attrs' => [
          new SetAttribute(
            new StringAttribute(
              _('member URL'), _('URL of member of this group'),
              'memberURL', FALSE,
              '', '',
              '/^ldap:\/\/\/[\w _=,-]+\?[\w _=,-]*\?\w+\?.+$/',
              'ldap:///BASE_DN??SCOPE?FILTER'
            )
          )
        ]
      ]
    ];
  }
}
