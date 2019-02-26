<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2014-2016 FusionDirectory

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

class webApplication extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Web application'),
      'plDescription' => _('Web applicationts names, icons and links'),
      'plObjectClass' => ['fdWebApplication'],
      'plObjectType'  => ['webApplication' => [
        'name'        => _('Web application'),
        'ou'          => get_ou('webappsRDN'),
        'icon'        => 'geticon.php?context=categories&icon=applications-internet&size=16'
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Application'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('webappsRDN')),
          new HostNameAttribute(
            _('Name'), _('Name or id for application'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('A short description of this application'),
            'description', FALSE
          ),
          new URLAttribute(
            _('URL'), _('Link to this application'),
            'labeledURI', TRUE
          ),
          new StringAttribute(
            _('Display Name'), _('Displayed name for links to this application'),
            'fdApplicationTitle', TRUE
          ),
        ]
      ],
      'icon' => [
        'name'  => _('Icon'),
        'attrs' => [
          new ImageAttribute(
            '', _('The icon for this application'),
            'fdApplicationImage', FALSE,
            48, 48, 'png'
          ),
          new StringAttribute(
            _('Icon location'), _('Usual path to this application icon'),
            'fdApplicationImageLocation', FALSE
          ),
        ]
      ],
    ];
  }
}
