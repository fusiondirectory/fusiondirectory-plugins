<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class application extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Application'),
      'plDescription' => _('Application information'),
      'plObjectClass' => ['fdDesktopApplication'],
      'plObjectType'  => ['application' => [
        'name'    => _('Application'),
        'ou'      => get_ou('applicationsRDN'),
        'icon'    => 'geticon.php?context=mimetypes&icon=application-x-executable&size=16',
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
          new BaseSelectorAttribute(get_ou('applicationsRDN')),
          new HostNameAttribute(
            _('Application name'), _('The name of this application'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('A short description of this application'),
            'description', FALSE
          ),
          new StringAttribute(
            _('Execute'), _('The application to be executed'),
            'fdApplicationExecutePath', TRUE
          ),
          new StringAttribute(
            _('Display name'), _('The displayed name for this application'),
            'fdApplicationTitle', FALSE
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
        ]
      ],
      'options' => [
        'name'  => _('Options'),
        'attrs' => [
          new BooleanAttribute(
            _('Only executable for member'), _('Only executable by the memebers of the group'),
            'flag_group', FALSE, FALSE, '',
            'G', ''
          ),
          new BooleanAttribute(
            _('Replace user configuration on startup'), _('Replace the user configuration on startup'),
            'flag_overwrite', FALSE, FALSE, '',
            'O', ''
          ),
          new BooleanAttribute(
            _('Place icon on members desktop'), _('Put an icon on the user desktop'),
            'flag_desktop', FALSE, FALSE, '',
            'D', ''
          ),
          new BooleanAttribute(
            _('Place entry in members startmenu'), _('Put an entry in the startup menu'),
            'flag_menu', FALSE, FALSE, '',
            'M', ''
          ),
          new BooleanAttribute(
            _('Place entry in members launch bar'), _('Put an entry in the startup menu'),
            'flag_launchbar', FALSE, FALSE, '',
            'L', ''
          ),
          new FlagsAttribute(
            'fdApplicationFlags',
            ['flag_group','flag_overwrite','flag_desktop','flag_menu','flag_launchbar']
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique('one');

    $filename = './plugins/applications/images/default_icon.png';

    if (file_exists($filename)) {
      $fd = fopen($filename, 'rb');
      $this->attributesAccess['fdApplicationImage']->setDefaultValue(fread($fd, filesize($filename)));
      fclose($fd);
      if (empty($this->fdApplicationImage)) {
        $this->attributesAccess['fdApplicationImage']->resetToDefault();
      }
    }
  }
}
