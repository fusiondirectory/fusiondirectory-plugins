<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2018  FusionDirectory

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

class applicationRights extends simplePlugin
{
  var $objectclasses = array('fdApplicationRights');

  var $displayHeader = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Applications'),
      'plDescription' => _('Applications allowed for this role'),
      'plObjectType'  => array('role'),
      'plForeignKeys' => array(
        'fdApplicationAllowed' => 'webApplication',
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Application list'),
        'attrs' => array(
          new SetAttribute (
            new SelectAttribute (
              _('Applications'), _('The applications users with this role are allowed to launch'),
              'fdApplicationAllowed', TRUE
            )
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $apps = objects::ls('application');
    $apps = array_merge($apps, objects::ls('webApplication'));
    asort($apps);
    $this->attributesAccess['fdApplicationAllowed']->attribute->setChoices(
      array_keys($apps),
      array_values($apps)
    );
  }
}
?>
