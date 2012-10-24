<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class argonautServer extends simpleService
{
  /* This plugin only writes its objectClass */
  var $objectclasses  = array("argonautServer");

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("Argonaut Server"),
      "plDescription"   => _("Argonaut Server")." ("._("Services").")",
      "plSelfModify"    => FALSE,
      "plDepends"       => array(),
      "plSection"       => array("administration"),
      "plCategory"      => array("server"),

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
        'name'  => _('Argonaut server'),
        'attrs' => array (
          new BooleanAttribute (
            _('Delete finished tasks'),
            _('Wether or not argonaut server should delete successfully finished tasks'),
            'argonautDeleteFinished', FALSE,
            TRUE
          ),
          new SelectAttribute (
            _('Protocol'), _('Protocol to use for argonaut'),
            'argonautProtocol', TRUE,
            array('http', 'https')
          ),
          new IntAttribute (
            _('Port'),
            _('Port on which the argonaut server will be listening'),
            'argonautPort', TRUE,
            0 /*min*/, FALSE /*no max*/, 8080 /*default value*/
          ),
          new IntAttribute (
            _('Timeout'),
            _('Time in seconds before to consider an argonaut client or server as down'),
            'argonautTimeout', FALSE,
            1 /*min*/, FALSE /*no max*/
          ),
          new StringAttribute (
            _('Log directory'),
            _('Directory in which argonaut will store its log'),
            'argonautLogDir', FALSE,
            '/var/log/argonaut'
          ),
          new StringAttribute (
            _('Key path'), _('Path to the private key file'),
            'argonautKeyPath', FALSE,
            CONFIG_DIR."/cert/server.key"
          ),
          new StringAttribute (
            _('Certificate path'), _('Path to the certificate file'),
            'argonautCertPath', FALSE,
            CONFIG_DIR."/cert/server.crt"
          )
        )
      ),
      'wakeonlan' => array (
        'name'  => _('Wake on lan'),
        'attrs' => array (
          new StringAttribute (
            _('Interface'),
            _('Interface to use for sending WakeOnLan requests'),
            'argonautWakeOnLanInterface', FALSE,
            'eth0'
          ),
          new StringAttribute (
            _('IP tool'), _('IP tool to use'),
            'argonautIpTool', FALSE,
            '/sbin/ifconfig'
          ),
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['argonautProtocol']->setVisible(FALSE);
    $this->attributesInfo['section1']['attrs']['argonautKeyPath']->setVisible(FALSE);
    $this->attributesInfo['section1']['attrs']['argonautCertPath']->setVisible(FALSE);
  }

  function save ()
  {
    parent::prepare_save();
    parent::ldap_save();
  }

}
?>
