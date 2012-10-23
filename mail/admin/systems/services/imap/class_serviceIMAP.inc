<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class goImapServer extends simpleService {

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goImapServer");

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("IMAP/POP3"),
          "plDescription" => _("IMAP/POP3")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plPriority"    => 94,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),
          "plProvidedAcls"=> parent::generatePlProvidedAcls(self::getAttributesInfo())
          ));
  }


  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['goImapSievePort']->setDisabled(TRUE); //disable write port access
  }


  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
      return array (
      'section1' => array (
        'name'  => '<img src="images/rightarrow.png" /> '._('Generic'),
        'attrs' => array (
          new StringAttribute (
            _('Server identifier'),
            _('Identifier for imap server'),
            'goImapName',
            TRUE
          ),
          new StringAttribute (
            _('Connect URL'),
            _('Connect URL for imap server'),
            'goImapConnect',
            TRUE,
            '',
            '',
            '/^\{[^:]+:[0-9]*\/.*\}$/',
            '{server-name:port/options}'
          ),
          new StringAttribute (
            _('Admin user'),
            _('Imap server admin user'),
            'goImapAdmin',
            TRUE
          ),
          new StringAttribute (
            _('Password'),
            _('Admin user password'),
            'goImapPassword',
            TRUE
          ),
          new StringAttribute (
            _('Sieve connect URL'),
            _('Sieve connect URL for imap server'),
            'goImapSieveServer',
            TRUE,
            '',
            '',
            '/^\{[^:]+:[0-9]*\/(no|)tls\}$/',
            '{server-name:port/options}. Valid options are: tls,notls'
          ),
          new IntAttribute (
            _('Port'),
            _('Port for sieve connect url of theimap server'),
            'goImapSievePort',
            FALSE,
            FALSE,
            FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Start IMAP service'),
            ' ',
            'cyrusImap',
            FALSE,
            FALSE,
            '',
            1,
            0
         ),
         new BooleanAttribute (
           _('Start IMAP SSL service'),
           ' ',
           'cyrusImapSSL',
           FALSE,
           FALSE,
           '',
           1,
           0
         ),
         new BooleanAttribute (
           _('Start POP3 service'),
           ' ',
           'cyrusPop3',
           FALSE,
           FALSE,
           '',
           1,
           0
         ),
         new BooleanAttribute (
           _('Start POP3 SSL service'),
           ' ',
           'cyrusPop3SSL',
           FALSE,
           FALSE,
           '',
           1,
           0
         )
        )
      ),
      'section2' => array (
        'name' => _('Action'),
        'icon' => '',
          'attrs' => array (
            new SelectAttribute (
              _('Set new status'),
              _('Set new status for the server'),
              'goImapServerStatus',
              FALSE,
              array (SERVICE_STOPPED,SERVICE_STARTED,SERVICE_RESTARTED,'repair_database'),
              '',
              array (SERVICE_STOPPED,SERVICE_STARTED,SERVICE_RESTARTED,_('Repair database'))
            )
          )
        )
    );
  }

  /**
   * \brief Save to LDAP
   */
  function save()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=goImapServer)",array('goImapSieveServer'));
    /* Search port from the URL */
    $url = $this->attributesInfo['section1']['attrs']['goImapSieveServer']-> getValue();
    $port = preg_replace("/^\{[^:]+:([0-9]*)\/.*$/","\\1",$url);
    $this->attributesInfo['section1']['attrs']['goImapSievePort']-> setValue($port);

    parent::save(); // save
  }

}

?>
