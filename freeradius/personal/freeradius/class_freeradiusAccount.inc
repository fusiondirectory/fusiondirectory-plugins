<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org/)
 * Copyright (C) 2003-2009 GONICUS GmbH
 * Copyright (C) 2011  FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*!
 * \file class_freeradiusAccount.inc
 * Source code for class freeradiusAccount
 */

/*!
 * \brief   FreeRadius plugin
 * \author  WILLIAME Jeremy <williamejeremy@hotmail.com>, Alejandro Escanero Blanco <aescanero@gmail.com>
 * \version 0.01
 * \date    25.06.2012
 * \see simplePlugin
 */

class freeradiusAccount extends simplePlugin {

  var $displayHeader = TRUE; // If we want the button Add/Delete
  var $objectclasses = array("radiusprofile");
  var $plHeadline    = "FreeRadius";
  var $plDescription = "This Plugin is for Radius accounting based in FreeRadius";


  /*!
   * \brief Plugin information
   */
  static function plInfo ()
  {
    return array(
    "plHeadline"        => _("Freeradius"),
    "plShortName"       => _("Freeradius"),
    "plIcon"            => _("plugins/freeradius/images/plugin.png"),
    "plDescription"     => _("This Plugin is for Radius accounting based in FreeRadius"),
    "plSelfModify"      => TRUE,                  // Does this plugin have an owner that might be able to edit its entry
    "plSection"         => array("personal"),     // Should be either addon, administration or personal
    "plCategory"        => array("user"),
    "plProvidedAcls"    => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=radiusprofile)",array('cn'));
    $groups = array();
    $firstcn = TRUE;
    while ($attrs = $ldap->fetch()) {
     // if(!$firstcn)
        $groups[] = $attrs['cn'][0];
      $firstcn = FALSE;
    }
    $this->attributesInfo['section2']['attrs']['radiusGroupName']->attribute->setChoices($groups);
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
            'IEEE 802'                            // default value
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
            FALSE,                          // Mandatory
            ''
          ),
          new StringAttribute(
              _('Expiration date'),                      // Label of the attribute
              _('Date of the expiration account'),       // Description
              'radiusExpiration',                        // LDAP name
              FALSE,                                     // Mandatory
              date('Y-m-d',time() + (7 * 24 * 60 * 60))  // Expire in one week
          ),
        )
      ),
      'section2' => array(
        'name'  => _('Groups'),
        'attrs' => array(
          new SetAttribute(               // This attribute is multi-valuated
              new SelectAttribute (
              _('Groups'),                // Label of the attribute
              _('FreeRadius Group'),      // Description
              'radiusGroupName',          // LDAP name
              FALSE,                      // Mandatory
              array("")                   // [SelectAttribute] Choices
            )
          ),
        ),
      ),
      'section3' => array(
        'name'  => _('User preferences for dorkboy'),
          'attrs' => array(
            new StringAttribute(
              _('Protocol'),
              _('Protocol'),
              'radiusFramedProtocol',
              FALSE,
              'PPP'
            ),
            new StringAttribute(
              _('IP Adress'),
              _('IP Adress'),
              'radiusFramedIPAddress',
              FALSE,
              '255.255.255.254'
            ),
            new StringAttribute(
              _('IP Netmask'),
              _('IP Netmask'),
              'radiusFramedIPNetmask',
              FALSE,
              ''
            ),
            new IntAttribute(
              _('Framed-MTU'),
              _('Framed-MTU'),
              'radiusFramedMTU',
              FALSE,
              0,                                // [FloatAttribute] Minimum value
              FALSE,                            // [FloatAttribute] No maximum value
              1500                              // [FloatAttribute] Default value
            ),
            new StringAttribute(
              _('Compression'),
              _('Compression'),
              'radiusFramedCompression',
              FALSE,
              'Van-Jacobsen-TCP-IP'
            ),
             new StringAttribute(
              _('Service type'),
              _('Service type'),
              'radiusServiceType',
              FALSE,
              'Framed-User'
            ),
            new IntAttribute(
              _('Session Timeout'),
              _('Session Timeout'),
              'radiusSessionTimeout',
              FALSE,                    // Mandatory
              FALSE,                    // no minimum value
              FALSE,                    // no maximum value
              ""                        // no default value
            ),
            new IntAttribute(
              _('Idle Timeout'),
              _('Idle Timeout'),
              'radiusIdleTimeout',
              FALSE,
              "",
              FALSE,
              10
            ),
            new IntAttribute(
              _('Port limit'),
              _('Port limit'),
              'radiusPortLimit',
              FALSE,
              0,
              FALSE,
              2
            )
          )
        )
    );
  }
}

?>
