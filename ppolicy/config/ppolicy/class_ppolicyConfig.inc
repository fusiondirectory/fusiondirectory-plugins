<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2014-2015 Dhatim
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

class ppolicyConfig extends multiPluginSection
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Ppolicy plugin configuration'),
      'plDescription'   => _('FusionDirectory ppolicy plugin configuration'),
      'plObjectClass'   => ['fdPpolicyPluginConf'],
      'plCategory'      => ['configuration'],
      'plObjectType'    => ['smallConfig'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'ppolicy' => [
        'name'  => _('Ppolicy'),
        'attrs' => [
          new StringAttribute(
            _('Ppolicy RDN'), _('Branch in which ppolicies will be stored'),
            'fdPpolicyRDN', TRUE,
            'ou=ppolicies'
          ),
          new StringAttribute(
            _('Default ppolicy dn'), _('What you put as default ppolicy in the overlay config'),
            'fdPpolicyDefaultDn', FALSE
          ),
          new HiddenAttribute('fdPpolicyDefaultCn'),
        ]
      ],
    ];
  }

  public static function fixAttributesOnLoad (multiPlugin $multiPlugin)
  {
    global $config;

    if (!empty($multiPlugin->fdPpolicyDefaultCn)) {
      if (empty($multiPlugin->fdPpolicyDefaultDn)) {
        $multiPlugin->fdPpolicyDefaultDn = 'cn='.$multiPlugin->fdPpolicyDefaultCn.','.get_ou('ppolicyRDN').$config->current['BASE'];
      }
      $multiPlugin->fdPpolicyDefaultCn = '';
    }
  }
}
