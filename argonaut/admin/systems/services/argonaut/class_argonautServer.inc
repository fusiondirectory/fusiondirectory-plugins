<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class argonautServer extends simpleService
{
  /* This plugin only writes its objectClass */
  var $objectclasses  = array("argonautServer");

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Argonaut server'),
      'plDescription' => _('Argonaut server').' ('._('Services').')',
      'plIcon'        => 'plugins/argonaut/images/iconMini.png',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

    /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
      'server' => array (
        'name'  => _('Argonaut server'),
        'attrs' => array (
          new BooleanAttribute (
            _('Delete finished tasks'),
            _('Wether or not argonaut server should delete successfully finished tasks'),
            'argonautDeleteFinished', FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('Get packages informations'),
            _('Wether or not argonaut server should get packages information from repositories at start and then once a day. Needed for FAI package list creation.'),
            'argonautFetchPackages', FALSE,
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
            1 /*min*/, FALSE /*no max*/,
            10
          ),
          new StringAttribute (
            _('Log directory'),
            _('Directory in which argonaut will store its log'),
            'argonautLogDir', FALSE,
            '/var/log/argonaut'
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
      ),
      'ssl' => array (
        'name'  => _('SSL paths'),
        'attrs' => array (
          new StringAttribute (
            _('Key'), _('Path to the private key file on Argonaut server'),
            'argonautKeyPath', FALSE,
            '/etc/ssl/private/argonaut.key'
          ),
          new StringAttribute (
            _('Certificate'), _('Path to the certificate file on Argonaut server'),
            'argonautCertPath', FALSE,
            '/etc/ssl/certs/argonaut.cert'
          ),
          new StringAttribute (
            _('CA certificate'), _('Path to the CA certificate file on Argonaut server'),
            'argonautCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
          new StringAttribute (
            _('CN of the certificate'), _('The CN in the server certificate'),
            'argonautCertCN', FALSE
          ),
          new HiddenAttribute ('argonautServerToken'),
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    if ($this->argonautServerToken == '') {
      $this->argonautServerToken = md5(mt_rand());
    }
    $this->attributesAccess['argonautProtocol']->setManagedAttributes(
      array(
        'disable' => array (
          'http' => array (
            'argonautKeyPath','argonautCertPath','argonautCaCertPath','argonautCertCN'
          )
        )
      )
    );
  }

  /*! \brief Get service information for serverService plugin */
  function getListEntry()
  {
    $fields = parent::getListEntry();

    /* Disallow some functions */
    $fields['AllowStart']   = FALSE;
    $fields['AllowStop']    = FALSE;
    $fields['AllowRestart'] = FALSE;

    return $fields;
  }
}
?>
