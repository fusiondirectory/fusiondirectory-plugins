<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016 FusionDirectory

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

class personalConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return array(
      'plShortName'     => _('Personal configuration'),
      'plDescription'   => _('FusionDirectory personal plugin configuration'),
      'plObjectClass'   => array('fdPersonalPluginConf'),
      'plCategory'      => array('configuration'),
      'plObjectType'    => array('smallConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'  => _('Personal'),
        'attrs' => array(
          new BooleanAttribute(
            _('Allow use of private email for password recovery'), _('Allow users to use their private email address for password recovery'),
            'fdPrivateEmailPasswordRecovery', FALSE
          ),
        )
      ),
    );
  }
}
