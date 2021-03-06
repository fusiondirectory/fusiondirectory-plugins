<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016 FusionDirectory

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

class applicationsPluginConfig extends multiPluginSection
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Applications'),
      'plDescription'   => _('Applications plugin configuration'),
      'plObjectClass'   => ['fdApplicationsPluginConf'],
      'plCategory'      => ['configuration'],
      'plObjectType'    => ['smallConfig'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'applications' => [
        'name'  => _('Applications'),
        'attrs' => [
          new StringAttribute(
            _('Application RDN'), _('Branch in which applications will be stored'),
            'fdApplicationsRDN', TRUE,
            'ou=apps'
          ),
          new StringAttribute(
            _('Web applications RDN'), _('Branch in which web applications will be stored'),
            'fdWebappsRDN', TRUE,
            'ou=apps'
          ),
          new SelectAttribute(
            _('Show web applications in menu'), _('Whether to show web applications in FD main menu'),
            'fdWebappsMenu', TRUE,
            ['none', 'allowed', 'all'], 'none',
            [_('None'), _('Only allowed'), _('All')]
          ),
        ]
      ]
    ];
  }
}
