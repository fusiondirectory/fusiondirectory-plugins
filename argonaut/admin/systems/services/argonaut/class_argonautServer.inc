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

class argonautServer extends simpleService {

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("argonautServer");
  var $StatusFlag       = "argonautServerStatus";
  var $argonautServerStatus = "";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Argonaut Server"),
          "plDescription" => _("Argonaut Server")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 91,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),
          "plProvidedAcls"=> parent::generatePlProvidedAcls(self::getAttributesInfo())
          ));
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
            '',
            'argonautDeleteFinished',
            FALSE,
            TRUE
          ),
          new IntAttribute (
            _('Port'),
            '',
            'argonautPort',
            FALSE,
            FALSE,
            FALSE,
            8080
          ),
          new StringAttribute (
            _('Interface for WakeOnLan'),
            '',
            'argonautWakeOnLanInterface',
            FALSE,
            'eth0'
          ),
          new StringAttribute (
            _('IP tool'),
            '',
            'argonautIpTool',
            FALSE,
            '/sbin/ifconfig'
          ),
          new StringAttribute (
            _('Log directory'),
            '',
            'argonautLogDir',
            FALSE,
            '/var/log/argonaut'
          ),
          new StringAttribute (
            _('Key path'),
            '',
            'argonautKeyPath',
            FALSE
          ),
          new StringAttribute (
            _('Cert path'),
            '',
            'argonautCertPath',
            FALSE
          )
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['argonautKeyPath']->setVisible(FALSE);
    $this->attributesInfo['section1']['attrs']['argonautCertPath']->setVisible(FALSE);
    $argonautKeyPath  = CONFIG_DIR."/cert/server.key";
    $argonautCertPath = CONFIG_DIR."/cert/server.crt";
    $this->attributesInfo['section1']['attrs']['argonautKeyPath']->setValue($argonautKeyPath);
    $this->attributesInfo['section1']['attrs']['argonautCertPath']->setValue($argonautCertPath);
  }

  function save ()
  {
    parent::prepare_save();
    parent::ldap_save();
  }

}
?>
