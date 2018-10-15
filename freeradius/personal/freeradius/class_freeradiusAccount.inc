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


class freeradiusAccount extends simplePlugin
{
  var $displayHeader = TRUE;

  static function plInfo ()
  {
    return array(
      'plShortName'   => _('Freeradius'),
      'plDescription' => _('This Plugin is for Radius accounting based in FreeRadius'),
      'plFilter'      => '(objectClass=radiusprofile)',
      'plIcon'        => 'geticon.php?context=applications&icon=freeradius&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=freeradius&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 11,
      'plObjectClass' => array('radiusprofile'),
      'plObjectType'  => array('user'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
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
            _('Tunnel medium type'), _('Name of the tunnel medium type'),
            'radiusTunnelMediumType', TRUE,
            'IEEE-802'
          ),
            new StringAttribute(
            _('Tunnel type'), _('Name of the tunnel type'),
            'radiusTunnelType', TRUE,
            'VLAN'
          ),
          new StringAttribute(
            _('VLAN id'), _('VLAN identifier'),
            'radiusTunnelPrivateGroupId', FALSE,
            ''
          ),
          new DateAttribute(
            _('Expiration date'), _('Date of the expiration account'),
            'radiusExpiration', FALSE,
            'd M Y', ''
          ),
        )
      ),
      'section2' => array(
        'name'  => _('Groups'),
        'attrs' => array(
          new SetAttribute(
              new SelectAttribute (
              _('Groups'),
              _('FreeRadius Group'),
              'radiusGroupName',
              FALSE,
              array("")
            )
          ),
        ),
      ),
      'section3' => array(
        'name'  => _('User preferences'),
          'attrs' => array(
            new StringAttribute(
              _('Protocol'), _('Protocol'),
              'radiusFramedProtocol', FALSE,
              'PPP'
            ),
            new StringAttribute(
              _('IP Address'), _('IP Address'),
              'radiusFramedIPAddress', FALSE,
              '255.255.255.254'
            ),
            new StringAttribute(
              _('IP Netmask'), _('IP Netmask'),
              'radiusFramedIPNetmask', FALSE,
              ''
            ),
            new IntAttribute(
              _('Framed-MTU'), _('Framed-MTU'),
              'radiusFramedMTU', FALSE,
              0,
              FALSE,
              1500
            ),
            new StringAttribute(
              _('Compression'), _('Compression'),
              'radiusFramedCompression', FALSE,
              'Van-Jacobsen-TCP-IP'
            ),
             new StringAttribute(
              _('Service type'), _('Service type'),
              'radiusServiceType', FALSE,
              'Framed-User'
            ),
            new IntAttribute(
              _('Session Timeout'), _('Session Timeout'),
              'radiusSessionTimeout', FALSE,
              FALSE,
              FALSE,
              ""
            ),
            new IntAttribute(
              _('Idle Timeout'), _('Idle Timeout'),
              'radiusIdleTimeout', FALSE,
              "",
              FALSE,
              10
            ),
            new IntAttribute(
              _('Port limit'), _('Port limit'),
              'radiusPortLimit', FALSE,
              0,
              FALSE,
              2
            )
          )
        )
    );
  }

  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=radiusprofile)', array('cn'));
    $groups = array();
    while ($attrs = $ldap->fetch()) {
      $groups[] = $attrs['cn'][0];
    }
    $this->attributesAccess['radiusGroupName']->attribute->setChoices($groups);
  }
}

?>
