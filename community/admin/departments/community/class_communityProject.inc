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

class communityProject extends communityOrganization
{
  var $objectclasses  = array('fdCommunityProject');
  var $displayHeader  = TRUE;

  static function plInfo ()
  {
    return array(
      'plShortName'     => _('Community project'),
      'plDescription'   => _('Community project dates and alternate address'),
      'plObjectType'    => array('department'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    $infos = array_merge(
      array(
        'project' => array(
          'name'  => _('Project'),
          'attrs' => array(
            new StringAttribute (
              _('Project full name'), _('Full name of this project'),
              'fdProjectFullName', FALSE
            ),
            new StringAttribute (
              _('Project key'), _('ID used for this project in other softwares or databases'),
              'fdProjectKey', FALSE
            ),
          )
        )
      ),
      parent::getAttributesInfo()
    );
    unset($infos['membership']);
    return $infos;
  }
}
?>
