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

class communityAccount extends simplePlugin
{
  var $displayHeader = TRUE;
  
  var $objectclasses = array('fdCommunityAccount');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Community'),
      'plDescription'   => _('Community account'),
      'plSelfModify'    => TRUE,
      'plObjectType'    => array('user'),
      'plIcon'          => 'geticon.php?context=applications&icon=community&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=community&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    global $config;
    return array(
      'main' => array(
        'name'  => _('Membership'),
        'attrs' => array(
          new SetAttribute(
            new SelectAttribute (
              _('Membership types'), _('Membership types of this user'),
              'fdCommunityMembershipType', FALSE,
              $config->get_cfg_value('communityMembershipTypeChoices', array())
            )
          ),
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }
}
?>
