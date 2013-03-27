<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2013  FusionDirectory

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
  var $objectclasses  = array("argonautDNSConfig");
  var $showActions    = FALSE;

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("Argonaut DNS settings"),
      "plDescription"   => _("Argonaut DNS settings")." ("._("Services").")",
      "plIcon"          => "plugins/asterisk/images/iconMiniDnsConfig.png",


      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
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
            _('Bind directory'),
            _('The directory in which conf file must be created'),
            'argonautLdap2zoneBindDir',
            TRUE,
            '/etc/bind'
          ),
          new StringAttribute (
            _('Bind cache directory'),
            _('The directory in which zone files must be created'),
            'argonautLdap2zoneBindCacheDir',
            TRUE,
            '/var/cache/bind'
          ),
          new BooleanAttribute (
            _('Allow notify'),
            '',
            'argonautLdap2zoneAllowNotify',
            FALSE
          ),
          new StringAttribute (
            _('Allow update (semicolon separated and ended)'),
            '',
            'argonautLdap2zoneAllowUpdate',
            FALSE
          ),
          new StringAttribute (
            _('Allow transfer (semicolon separated and ended)'),
            '',
            'argonautLdap2zoneAllowTransfer',
            FALSE
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
            "/usr/sbin/rndc"
          )
        )
      )
    );
  }
}

?>
