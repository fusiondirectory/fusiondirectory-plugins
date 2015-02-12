<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class serviceLDAP extends simpleService
{
  /* This plugin only writes its objectClass */
  var $objectclasses    = array('goLdapServer');

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    global $config;
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('LDAP service'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new StringAttribute (
            _('LDAP Base'), _('The LDAP base to use for this LDAP server'),
            'goLdapBase', TRUE,
            $config->current['BASE']
          ),
          new StringAttribute (
            _('LDAP URI'), _('The LDAP URI to use in order to contact this LDAP server'),
            'goLdapURI', TRUE,
            'ldap://'._('fill-in-your-servers-dns-name').':389/',
            '',
            "/^ldap[si]?:\/\/[0-9a-z_.-]+(:[0-9]+)?\/.*$/"
          ),
          new IntAttribute (
            _('Size limit'), _('Limit the number of record returned'),
            'goLdapSizeLimit', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute (
            _('Time limit'), _('Time limit for the result to be returned'),
            'goLdapTimeLimit', FALSE,
            0, FALSE, 15
          ),
          new SelectAttribute (
            _('Deref'), _('Specifies how alias dereferencing is done when performing a search'),
            'goLdapDeref', TRUE,
            array('never','searching','finding','always'), 'never'
          ),
          new StringAttribute (
            _('TLS Cert'), _('Filepath to tls certificate'),
            'goLdapTlsCert', FALSE
          ),
          new StringAttribute (
            _('TLS Key'), _('Filepath to tls key'),
            'goLdapTlsKey', FALSE
          ),
          new StringAttribute (
            _('TLS CaCert'), _('Filepath to tls ca certificate'),
            'goLdapTlsCaCert', FALSE
          ),
          new SelectAttribute (
            _('TLS ReqCert'), _('Specifies what checks to perform on server certificates in a TLS session, if any'),
            'goLdapReqCert', TRUE,
            array('never','allow','try','demand'), 'never'
          ),
          new SelectAttribute (
            _('TLS CrlCheck'), _('Specifies if the Certificate Revocation List (CRL) of the CA should be used to verify if the server certificates have  not  been  revoked'),
            'goLdapCrlCheck', TRUE,
            array('none','peer','all'), 'none'
          ),
        )
      ),
    );
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('LDAP service'),
      'plDescription' => _('LDAP').' ('._('Services').')',
      'plIcon'        => 'plugins/systems/images/service_ldap.png',

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}
?>
