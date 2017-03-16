<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class argonautDNSConfig extends simpleService
{
  var $objectclasses  = array('argonautDNSConfig');
  var $showActions    = FALSE;

  /* Return plugin information for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Argonaut DNS settings'),
      'plDescription'   => _('Argonaut DNS settings').' ('._('Services').')',
      'plIcon'          => 'geticon.php?context=applications&icon=argonaut-dns&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }


  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
    'section1' => array (
        'name'  => _('Argonaut Ldap2zone settings:'),
        'attrs' => array (
          new StringAttribute (
            _('Bind directory'), _('The directory in which conf file must be created'),
            'argonautLdap2zoneBindDir', TRUE,
            '/etc/bind'
          ),
          new StringAttribute (
            _('Bind cache directory'), _('The directory in which zone files must be created'),
            'argonautLdap2zoneBindCacheDir', TRUE,
            '/var/cache/bind'
          ),
          new BooleanAttribute (
            _('Ignore reverse zone'), _('Do not write reverse zone'),
            'argonautLdap2zoneNoReverse', FALSE
          ),
          new BooleanAttribute (
            _('Allow notify'), '',
            'argonautLdap2zoneAllowNotify', FALSE
          ),
          new StringAttribute (
            _('Allow update'), _('Allow update (semicolon separated and ended)'),
            'argonautLdap2zoneAllowUpdate', FALSE
          ),
          new StringAttribute (
            _('Allow transfer'), _('Allow transfer (semicolon separated and ended)'),
            'argonautLdap2zoneAllowTransfer', FALSE
          ),
          new SelectAttribute (
            _('Check names'), _('Cause any host name for the zone to be checked for compliance with RFC 952 and RFC 1123 and take the defined action'),
            'argonautLdap2zoneCheckNames', FALSE,
            array('','warn','fail','ignore'), ''
          ),
          new IntAttribute (
            _('TTL'),
            _('Time to live'),
            'argonautLdap2zoneTTL',
            FALSE,
            FALSE,
            FALSE,
            500
          ),
          new StringAttribute (
            _('rndc path'),
            _('path to rndc binary'),
            'argonautLdap2zoneRndc',
            FALSE,
            '/usr/sbin/rndc'
          ),
          new StringAttribute (
            _('Search base'), _('LDAP base in which ldap2zone should search. Only usefull if you got several nodes for the same zone.'),
            'argonautLdap2zoneSearchBase', FALSE
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Slave zones'), _('DNS zones this server should be declared as slave of'),
              'argonautLdap2zoneSlaveZonesDn', TRUE
            )
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $parent = NULL, $attributesInfo = NULL)
  {
    global $config;
    parent::__construct($dn, $parent, $attributesInfo);
    $zonesCache = objects::ls('dnsZone');
    $counts = array_count_values($zonesCache);
    foreach ($zonesCache as $dn => &$name) {
      if ($counts[$name] > 1) {
        $name .= ' ('.preg_replace(array('/^[^,]+,/', '/,'.preg_quote($config->current['BASE']).'$/'), '', $dn).')';
      }
    }
    unset($name);
    $this->attributesAccess['argonautLdap2zoneSlaveZonesDn']->attribute->setChoices(array_keys($zonesCache), array_values($zonesCache));
  }
}

?>
