<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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

class serviceSympa extends simpleService
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Sympa Server'),
      'plDescription' => _('Sympa Server').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=applications&icon=sympa&size=16',
      'plObjectClass' => ['sympaServer'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    return  [
      'main' => [
        'name'  => _('Sympa server'),
        'attrs' => [
          new StringAttribute(
            _('URL'), _('URL to access the sympa server'),
            'sympaServerURL', TRUE
          ),
          new StringAttribute(
            _('User'), _('User to access sympa server SOAP API.'),
            'sympaServerUser', FALSE
          ),
          new PasswordAttribute(
            _('Password'), _('Password to access sympa server SOAP API.'),
            'sympaServerPassword', FALSE
          )
        ]
      ],
    ];
  }
}
