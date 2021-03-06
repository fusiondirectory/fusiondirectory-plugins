<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2014-2019 FusionDirectory

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

class ejbcaCertificate extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('EJBCA certificate'),
      'plDescription' => _('EJBCA certificate'),
      'plObjectType'  => ['ejbcaCertificate' => [
        'name'        => _('EJBCA certificate'),
        'filter'      => 'userCertificate;binary=*',
        'ou'          => get_ou('ejbcaRDN'),
        'icon'        => 'geticon.php?context=applications&icon=ejbca&size=16'
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Certificate'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('ejbcaRDN')),
          new HostNameAttribute(
            _('Name'), _('Common name'),
            'cn', TRUE
          ),
          new FileDownloadAttribute(
            _('Certificate'), _('Certificate content'),
            'userCertificate;binary', TRUE,
            '.der'
          ),
        ]
      ],
    ];
  }

  function renderAttributes (bool $readOnly = FALSE)
  {
    parent::renderAttributes(TRUE);
  }

  function save (): array
  {
    return [];
  }

  function remove (bool $fulldelete = FALSE): array
  {
    return [];
  }
}
