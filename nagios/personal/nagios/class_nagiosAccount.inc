<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2005 Guillame Delecourt
  Copyright (C) 2006 Vincent Seynhaeve
  Copyright (C) 2006 Benoit Mortier
  Copyright (C) 2011  FusionDirectory

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

/*! \brief   mail plugin
  \author  Guillame Delecourt <guillaume@opensides.be>
  \author  Benoit Mortier <benoit.mortier@opensides.be>
  \author  Vincent Seynhaeve <vincent@opensides.be>
  \version 1.00
  \date    25.09.2005

  This class provides the functionality to read and write all attributes
  relevant for nagiosAccount from/to the LDAP. It does syntax checking
  and displays the formulars required.
 */

class nagiosAccount extends simplePlugin
{
  var $displayHeader = TRUE; // If we want the button Add/Delete
  var $objectclasses = array("nagiosContact","nagiosAuth");
  var $plHeadline= "Nagios";
  var $plDescription= "Nagios account settings";

  static function plInfo()
  {
    return array(
      "plShortName"   => _("Nagios"),
      "plDescription" => _("Nagios account settings"),
      "plSelfModify"  => TRUE,
      "plDepends"     => array("user"),
      "plPriority"    => 8,                                 // Position in tabs
      "plSection"     => array("personal" => _("My account")),
      "plCategory"    => array("users"),
      "plObjectType"  => array("user"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
    'section1' => array(
        'name'  => _('Nagios Account'),
        'attrs' => array(
          new StringAttribute(
            _('Alias'),                       // Label of the attribute
            _('Name of the nagios alias'),    // Description
            'NagiosAlias',                    // LDAP name
            TRUE,                             // Mandatory
            ''                                // default value
          ),
          new StringAttribute(
            _('Mail address'),                // Label of the attribute
            _('Email of the nagios alias'),   // Description
            'NagiosMail',                     // LDAP name
            TRUE,                             // Mandatory
            '',                                // default value
            '',
            "/^[^@]*@[^@]*$/"                 // check if it's a valid mail address
          ),
          new SelectAttribute (
            _('Host notification period'),         // Label of the attribute
            _('Host notification period'),         // Description
            'HostNotificationPeriod',              // LDAP name
            TRUE,                                  // Mandatory
            array('24x7','24x5','8x5'),            // [SelectAttribute] Choices
            "",                                    // We don't set any default value, it will be the first one
            array('24x7','24x5','8x5')             // [SelectAttribute] Output choices
          ),
          new SelectAttribute (
            _('Service notification period'),         // Label of the attribute
            _('Service notification period'),         // Description
            'ServiceNotificationPeriod',              // LDAP name
            TRUE,                                     // Mandatory
            array('24x7','24x5','8x5'),               // [SelectAttribute] Choices
            "",                                       // We don't set any default value, it will be the first one
            array('24x7','24x5','8x5')                // [SelectAttribute] Output choices
          ),
          new SelectAttribute (
            _('Service notification options'),                         // Label of the attribute
            _('Service notification options'),                         // Description
            'ServiceNotificationOptions',                              // LDAP name
            TRUE,                                                      // Mandatory
            array('w,u,c,r,f','w,u,c,r','w,u,c','c,w','n'),            // [SelectAttribute] Choices
            "",                                                        // We don't set any default value, it will be the first one
            array('w,u,c,r,f','w,u,c,r','w,u,c','c,w','n')             // [SelectAttribute] Output choices
          ),
          new SelectAttribute (
            _('Host notification options'),                          // Label of the attribute
            _('Host notification options'),                          // Description
            'HostNotificationOptions',                               // LDAP name
            TRUE,                                                    // Mandatory
            array('d,u,r,f','d,u,r','d,u','n'),                      // [SelectAttribute] Choices
            "",                                                      // We don't set any default value, it will be the first one
            array('d,u,r,f','d,u,r','d,u','n')                       // [SelectAttribute] Output choices
          ),
          new StringAttribute(
            _('Pager'),                        // Label of the attribute
            _('Nagios Pager'),                 // Description
            'NagiosPager',                     // LDAP name
            FALSE,                             // Mandatory
            ''                                 // default value
          ),
          new StringAttribute(
            _('Service notification commands'),                // Label of the attribute
            _('Service notification commands'),                // Description
            'ServiceNotificationCommands',                     // LDAP name
            FALSE,                                             // Mandatory
            ''                                                 // default value
          ),
          new StringAttribute(
            _('Host notification commands'),   // Label of the attribute
            _('Host notification commands'),   // Description
            'HostNotificationCommands',        // LDAP name
            FALSE,                             // Mandatory
            ''                                 // default value
          )
        )
      ),
      'section2' => array(
        'name'  => _('Nagios authentification'),
          'attrs' => array(
            new BooleanAttribute (
              _('view system informations'),      // Label
              _('view system informations'),      // Description
              'AuthorizedSystemInformation',      // LDAP name
              FALSE,                              // Not mandatory
              FALSE                               // Default value
            ),
            new BooleanAttribute (
              _('view configuration information'),      // Label
              _('view configuration information'),      // Description
              'AuthorizedConfigurationInformation',     // LDAP name
              FALSE,                                    // Not mandatory
              FALSE                                     // Default value
            ),
            new BooleanAttribute (
              _('trigger system commands'),       // Label
              _('trigger system commands'),       // Description
              'AuthorizedSystemCommands',         // LDAP name
              FALSE,                              // Not mandatory
              FALSE                               // Default value
            ),
            new BooleanAttribute (
              _('view all services'),       // Label
              _('view all services'),       // Description
              'AuthorizedAllServices',       // LDAP name
              FALSE,                        // Not mandatory
              FALSE                         // Default value
            ),
            new BooleanAttribute (
              _('view all hosts'),       // Label
              _('view all hosts'),       // Description
              'AuthorizedAllHosts',      // LDAP name
              FALSE,                     // Not mandatory
              FALSE                      // Default value
            ),
            new BooleanAttribute (
              _('trigger all service commands'),      // Label
              _('trigger all service commands'),      // Description
              'AuthorizedAllServiceCommands',         // LDAP name
              FALSE,                                  // Not mandatory
              FALSE                                   // Default value
            ),
            new BooleanAttribute (
              _('trigger all host commands'),       // Label
              _('trigger all host commands'),       // Description
              'AuthorizedAllHostCommands',          // LDAP name
              FALSE,                                // Not mandatory
              FALSE                                 // Default value
            )
        )
      )
    );
  }
}
