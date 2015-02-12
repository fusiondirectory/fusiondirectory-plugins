<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2015  FusionDirectory

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

class phoneIAX extends simplePlugin
{
  var $objectClass  = array('top', 'AsteriskExtension', 'AsteriskIAXUser');
  var $mainTab      = TRUE;

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Phone'),
      'plDescription' => _('Asterisk phone'),
      'plObjectType'  => array('phone' => array(
        'description' => _('Phone hardware'),
        'name'        => _('Phone'),
        'filter'      => 'objectClass=AsteriskExtension'
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'phone' => array(
        'name'      => _('Telephone'),
        'attrs'     => array(
          new StringAttribute(
            _('Name'), _('Name of this phone'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Account name'), _('Name of the account'),
            'AstAccountName', FALSE
          ),
          /*AstAccountAMAFlags
          AstAccountCallerID
          AstAccountContext
          AstAccountFullContact
          AstAccountHost
          AstAccountMailbox
          AstMD5secret
          AstAccountDeny
          AstAccountPermit
          AstAccountPort
          AstAccountQualify
          AstAccountType
          AstAccountLanguage
          AstAccountDisallowedCodec
          AstAccountExpirationTimestamp
          AstAccountRegistrationContext
          AstAccountRegistrationExten
          AstAccountNoTransfer

          AstAccountLastQualifyMilliseconds
          AstAccountCallLimit
          AstAccountSubscribeContext
          AstAccountIPAddress
          AstAccountUserAgent*/
        ),
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }
}
?>
