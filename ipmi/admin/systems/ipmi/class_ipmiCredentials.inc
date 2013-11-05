<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class ipmiCredentials extends simplePlugin
{
  var $objectclasses  = array();
  var $displayHeader  = FALSE;

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Credentials'),
      'plDescription' => _('IPMI credentials'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('ipmi'),
      'plObjectType'  => array('ipmi'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ($word = NULL, $rdn = NULL)
  {
    return array(
      'main' => array(
        'name'  => _('IPMI client settings'),
        'attrs' => array(
          new StringAttribute (
            _('User login'), _('IPMI user login'),
            'fdIpmiLogin', FALSE
          ),
          new PasswordAttribute (
            _('User password'), _('IPMI user password'),
            'fdIpmiPassword', FALSE
          ),
        )
      ),
    );
  }
}
?>
