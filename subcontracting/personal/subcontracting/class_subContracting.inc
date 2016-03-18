<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2015-2016 OpenSides <benoit.mortier@opensides.be>

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class subContractingAccount extends simplePlugin
{
  // We set displayHeader to FALSE, because we don't want a header allowing to activate/deactivate this plugin,
  // we want it activated on all objects
  var $displayHeader = TRUE;

  // Here we indicate which LDAP classes our plugin is using.
  var $objectclasses = array('fdSubcontractingPerson');

  static function plInfo ()
  {
    return array(
      'plShortName'   => _('Subcontracting'),
      'plDescription' => _('Information needed for sub contracts'),
      'plObjectType'  => array('user'),
      'plIcon'        => 'geticon.php?context=applications&icon=subcontracting&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=subcontracting&size=16',

      // simplePlugin can generate the ACL list for us
      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('SubContracting Information'),
        'attrs' => array(
          new StringAttribute (
            _('Intermediate'),                            // Label
            _('Contact to the final customer'),           // Description
            'fdMissionIntermediate',                      // LDAP name
            FALSE,                                        // Not Mandatory
            ''                                            // Default value
          ),
          new StringAttribute (
            _('Final Customer'),                          // Label
            _('Final Customer for this mission'),         // Description
            'fdMissionFinalCustomer',                     // LDAP name
            FALSE,                                        // Not Mandatory
            ''                                            // Default value
          ),
          new MailAttribute (
            _('Mail Address'),                            // Label
            _('Mail address assigned for this mission'),  // Description
            'fdMissionMail',                              // LDAP name
            FALSE,                                        // Not Mandatory
            ''                                            // Default value
          ),
          new StringAttribute (
            _('Phone'),                                   // Label
            _('Phone number assigned for this mission'),  // Description
            'fdMissionPhone',                             // LDAP name
            FALSE,                                        // Not Mandatory
            ''                                            // Default value
          ),
          new StringAttribute (
            _('Street Address'),                          // Label
            _('Address where this mission is executed'),  // Description
            'fdMissionAddress',                           // LDAP name
            FALSE,                                        // Not Mandatory
            ''                                            // Default value
          ),
        )
      ),
    );
  }
}
?>
