<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

class dnsAcl extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('DNS acl'),
      'plDescription' => _('DNS acl'),
      'plObjectClass' => ['fdDNSAcl'],
      'plObjectType'  => ['dnsAcl' => [
        'name'        => _('DNS acl'),
        'ou'          => get_ou('dnsRDN'),
        'icon'        => 'geticon.php?context=categories&icon=acl&size=16'
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Acl'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('dnsRDN')),
          new HostNameAttribute(
            _('ACL name'), _('Name of this acl'),
            'cn', TRUE
          ),
          new SetAttribute(
            new StringAttribute(
              _('Address match list'), _('The ip address match list for this acl'),
              'fdDNSAclMatchList', FALSE
            )
          )
        ]
      ]
    ];
  }
}
