<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2009 GONICUS GmbH
  Copyright (C) 2012 WILLIAME Jeremy
  Copyright (C) 2011-2016  FusionDirectory

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

class freeradiusGroup extends simplePlugin {

  var $displayHeader = TRUE;
  var $objectclasses = array('radiusprofile');

  /*!
   * \brief Plugin information
   */
  static function plInfo ()
  {
    return array(
      'plShortName'       => _('Freeradius'),
      'plDescription'     => _('This Plugin is for Radius accounting based in FreeRadius'),
      'plObjectType'      => array('group'),
      'plProvidedAcls'    => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
    // Attributes are grouped by section
    'section1' => array(
        'name'  => _('Support 802.1x'),
        'attrs' => array(
          new StringAttribute(
            _('Tunnel medium type'),              // Label of the attribute
            _('Name of the tunnel medium type'),  // Description
            'radiusTunnelMediumType',             // LDAP name
            TRUE,                                 // Mandatory
            'IEEE-802'                            // default value
          ),
            new StringAttribute(
            _('Tunnel type'),               // Label of the attribute
            _('Name of the tunnel type'),   // Description
            'radiusTunnelType',             // LDAP name
            TRUE,                           // Mandatory
            'VLAN'                          // default value
          ),
          new StringAttribute(
            _('VLAN id'),                   // Label of the attribute
            _('VLAN identifier'),           // Description
            'radiusTunnelPrivateGroupId',   // LDAP name
            FALSE,                           // Mandatory
            ''
          )
        )
      ),
    );
  }
}

?>
