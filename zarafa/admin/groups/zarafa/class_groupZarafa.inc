<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2009-2010 Bas Tichelaar
  Copyright (C) 2013  FusionDirectory

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

class zarafagroup extends simplePlugin
{
  var $displayHeader = TRUE;
  var $objectclasses = array('zarafa-group');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'    => _('Zarafa group'),
      'plDescription'  => _('Zarafa group'),
      'plSelfModify'   => FALSE,
      'plPriority'     => 10,
      'plCategory'     => array('groups'),
      'plObjectType'   => array('group'),
      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }


  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['zarafaSecurityGroup']->setManagedAttributes(
      array(
        'erase' => array(
          FALSE => array(
            'zarafaHidden',
          )
        )
      )
    );
  }


   static function getAttributesInfo ()
  {
    return array(
      'section1' => array(
        'name'  => _('Zarafa Settings'),
        'attrs' => array(
          new BooleanAttribute(
            _('Make this group a Zarafa Securitygroup'), _('Make this group a Zarafa Securitygroup'),
            'zarafaSecurityGroup', FALSE,
            FALSE,
            '',
            1,
            0
          ),
          new BooleanAttribute(
            _('Hide from Zarafa addressbook'), _('Hide from Zarafa addressbook'),
            'zarafaHidden', FALSE,
            FALSE,
            '',
            1,
            0
          )
        )
      )
    );
  }
}
