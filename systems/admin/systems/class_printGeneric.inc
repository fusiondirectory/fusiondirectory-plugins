<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

class printGeneric extends ipHostPlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Printer'),
      'plDescription' => _('Printer information'),
      'plPriority'    => 10,
      'plObjectClass' => ['fdPrinter', 'ieee802Device'],
      'plFilter'      => '(objectClass=fdPrinter)',
      'plObjectType'  => ['printer' =>
        [
          'name'        => _('Printer'),
          'description' => _('Printer'),
          'icon'        => 'geticon.php?context=devices&icon=printer&size=16',
          'ou'          => get_ou('printerRDN'),
        ]
      ],
      'plForeignKeys'  => [
        'fdPrinterUsers'      => 'user',
        'fdPrinterAdminUsers' => 'user',
      ],
      'plSearchAttrs' => ['description', 'ipHostNumber', 'macAddress'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('printerRDN')),
          new HostNameAttribute(
            _('Name'), _('The name of the printer'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('A short description of the printer'),
            'description', FALSE
          ),
        ]
      ],
      'details' => [
        'name'  => _('Details'),
        'attrs' => [
          new StringAttribute(
            _('Printer location'), _('The location of the printer'),
            'l', FALSE
          ),
          new StringAttribute(
            _('Printer URL'), _('The URL of the printer'),
            'labeledURI', TRUE
          ),
        ]
      ],
      'users' => [
        'name'  => _('Users which are allowed to use this printer'),
        'attrs' => [
          new UsersAttribute(
            '', _('Users which are allowed to use this printer'),
            'fdPrinterUsers', FALSE
          )
        ]
      ],
      'admins' => [
        'name'  => _('Users which are allowed to administrate this printer'),
        'attrs' => [
          new UsersAttribute(
            '', _('Users which are allowed to administrate this printer'),
            'fdPrinterAdminUsers', FALSE
          )
        ]
      ],
      'network' => [
        'name'      => _('Network settings'),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
        'attrs'     => [
          new SetAttribute(
            new IPAttribute(
              _('IP address'), _('IP address this system uses'),
              'ipHostNumber', TRUE
            )
          ),
          new SetAttribute(
            new MacAddressAttribute(
              _('Mac address'), _('Mac address of this system'),
              'macAddress', FALSE
            )
          ),
        ],
      ],
      'windows' => [
        'name'      => _('Windows paths'),
        'icon'      => 'geticon.php?context=devices&icon=computer-windows&size=16',
        'attrs'     => [
          new StringAttribute(
            _('Inf file'), _('Path to windows inf file for this printer'),
            'fdPrinterWindowsInfFile', FALSE
          ),
          new StringAttribute(
            _('Driver directory'), _('Path to directory that contains windows drivers for this printer'),
            'fdPrinterWindowsDriverDir', FALSE
          ),
          new StringAttribute(
            _('Driver name'), _('Windows name of the printer driver'),
            'fdPrinterWindowsDriverName', FALSE
          ),
        ],
      ],
    ];
  }
}
