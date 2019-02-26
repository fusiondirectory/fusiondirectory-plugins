<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2012-2016 FusionDirectory

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

class simpleSecurityObject extends simplePlugin
{
  public static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Simple security object'),
      'plDescription' => _('Simple security object'),
      'plObjectClass' => array('organizationalRole', 'simpleSecurityObject'),
      'plFilter'      => '(objectClass=simpleSecurityObject)',
      'plObjectType'  => array('simpleSecurityObject' => array(
        'name'        => _('Simple security object'),
        'aclCategory' => 'dsa',
        'ou'          => get_ou('dsaRDN'),
        'icon'        => 'geticon.php?context=applications&icon=dsa&size=16',
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('Simple security object'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('dsaRDN')),
          new HostNameAttribute(
            _('Entry name'), _('Account name'),
            'cn', TRUE
          ),
          new TextAreaAttribute(
            _('Description'), _('Description of this simple security object'),
            'description', FALSE
          ),
        )
      ),
      'password' => array(
        'name'  => _('Change password'),
        'attrs' => array(
          new UserPasswordAttribute(
            _('Password'), _('Password'),
            'userPassword', FALSE
          )
        )
      )
    );
  }
}
