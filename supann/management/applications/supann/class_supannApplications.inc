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

class supannApplications extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('SupAnn'),
      'plDescription'   => _('SupAnn Applications'),
      'plSmallIcon'     => 'geticon.php?context=applications&icon=supann&size=16',
      'plObjectClass'   => ['supannApplication'],
      'plObjectType'    => ['webApplication'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Information'),
        'attrs' => [
          new mailAttribute(
            _('Contact'), _('contact email address, internal or external to the institution'),
            'supannMailContact', FALSE,
            ''
          ),
          new mailAttribute(
            _('Mailbox'), _('email address of the mailbox assigned to the application'),
            'mail', FALSE,
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
      ],
      'administration' => [
        'name'  => _('Administrative information'),
        'attrs' => [
          new UserAttribute(
            _('owner'), _('DN of the administrative manager of the application'),
            'owner', FALSE,
          ),
          new UserAttribute(
            _('Manager'), _('DN of the application technical manager'),
            'manager', FALSE,
          ),
        ]
      ]
    ];
  }
}
