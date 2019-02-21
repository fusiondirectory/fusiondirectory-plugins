<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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

class userRegistration extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Registration'),
      'plDescription' => _('User registration status'),
      'plSmallIcon'   => 'geticon.php?context=status&icon=pending&size=16',
      'plObjectClass' => array('fdUserRegistration'),
      'plObjectType'  => array('user'),
      'plSelfModify'  => FALSE,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'      => _('User registration'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new DisplayAttribute(
            '', '',
            'msg', FALSE
          ),
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->msg = _('This user object is a registration awaiting approval');
  }
}
