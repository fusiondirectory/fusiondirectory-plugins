<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013 FusionDirectory

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

class boardConfig extends simplePlugin
{
  var $objectclasses  = array('fdBoardPluginConf');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Board configuration'),
      'plDescription'   => _('FusionDirectory board plugin configuration'),
      'plSelfModify'    => FALSE,
      'plCategory'      => array('configuration'),
      'plObjectType'    => array('smallConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'systems' => array(
        'name'  => _('Board name schema'),
        'attrs' => array(
          new IntAttribute (
            _('Number of digits'), _('Number of digits to use after prefix'),
            'fdBoardNumberOfDigit', FALSE,
            1, 20, 3
          ),
          new SetAttribute (
            new StringAttribute (
              _('Prefixes'), _('Prefixes to be used for computer ids'),
              'fdBoardPrefix', TRUE
            ),
            array("PC")
          )
        )
      ),
      'users' => array(
        'name'  => _('Board expired users'),
        'attrs' => array(
          new IntAttribute (
            _('Number of days'), _('Number of days before expiration needed for an account to show in the board next expired accounts list'),
            'fdBoardExpiredAccountsDays', FALSE,
            1, FALSE, 15
          ),
        )
      ),
    );
  }
}
?>
