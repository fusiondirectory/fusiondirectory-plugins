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

class ArgonautServiceNameAttribute extends CompositeAttribute
{
  static private $argonautService_defaults = [
    "folder"              => "/etc/init.d",
    "serviceCUPS"         => "cups",
    "serviceMail"         => "postfix",
    "serviceNTP"          => "ntpd",
    "serviceCyrus"        => "cyrus-imapd",
    "serviceTerminal"     => "ssh",
    "serviceLDAP"         => "slapd",
    "serviceShare"        => "samba",
    "serviceSpamAssassin" => "spamassassin",
    "serviceAntiVirus"    => "clamav-daemon",
    "serviceDHCP"         => "isc-dhcp-server",
    "serviceDNS"          => "bind9",
    "serviceQuota"        => "quota",
    "serviceOPSI"         => "opsiconfd",
    "argonautServer"      => "argonaut-server",
    "argonautFAIMonitor"  => "argonaut-fai-monitor",
    "argonautFuseConfig"  => "argonaut-fuse",
  ];

  static private $argonautService_blacklist = [
    "argonautDNSConfig",
    "argonautMirrorConfig",
    "serviceRepository",
  ];

  function __construct ($description, $ldapName, $acl = "")
  {
    parent::__construct($description, $ldapName, [], "", "", $acl);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    $this->loadServiceNames();
  }

  function loadServiceNames ()
  {
    global $config;
    $this->attributes = [];
    // Get services tabs list from config
    $services = $config->data['TABS']['SERVERSERVICE'];
    // Add fake "folder" tab at the beginning
    $services = array_merge([['CLASS' => 'folder']], $services);

    foreach ($services as $tab) {
      $name = $tab['CLASS'];
      if (in_array($name, static::$argonautService_blacklist)) {
        continue;
      }
      $value = (isset(static::$argonautService_defaults[$name]) ? static::$argonautService_defaults[$name] : "");
      $this->attributes[$name] = new StringAttribute($name, "Service $name",
                                                      "service_$name", FALSE,
                                                      $value, $this->getAcl());
    }
  }

  function loadAttrValue (array $attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      for ($i = 0; $i < $attrs[$this->getLdapName()]['count']; $i++) {
        $serv = explode(':', $attrs[$this->getLdapName()][$i]);
        if (isset($this->attributes[$serv[0]])) {
          $this->attributes[$serv[0]]->value = $serv[1];
        }
      }
    }
  }

  function getValue ()
  {
    return array_values(array_map(
      function ($a)
      {
        return $a->getLabel().":".$a->getValue();
      },
      $this->attributes
    ));
  }

  function getServiceNames ()
  {
    return array_map(
      function ($a)
      {
        return $a->getLdapName();
      },
      $this->attributes
    );
  }
}

class argonautClient extends simplePlugin
{
  protected $displayHeader  = TRUE;

  var $inheritance = ['gosaGroupOfNames' => 'member'];

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Argonaut client'),
      'plDescription'   => _('Edit argonaut client settings'),
      'plSmallIcon'     => 'geticon.php?context=applications&icon=argonaut&size=16',
      'plObjectClass'   => ['argonautClient'],
      'plObjectType'    => ['workstation','server','terminal','ogroup-dynamic'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _("Argonaut client settings"),
        'attrs' => [
          new IntAttribute(
            _('Client port'), _('Port used by argonaut client for JSON-RPC'),
            'argonautClientPort', TRUE,
            0 /*min*/, FALSE /*no max*/, 8081 /*default value*/
          ),
          new SelectAttribute(
            _('Protocol'), _('Protocol to use for argonaut'),
            'argonautClientProtocol', TRUE,
            ['http', 'https']
          ),
          new StringAttribute(
            _('WakeOnLan interface'), _('Interface used by argonaut for WakeOnLan'),
            'argonautClientWakeOnLanInterface', TRUE,
            'eth0'
          ),
          new StringAttribute(
            _('TaskId file'), _('File which argonaut will use to store its task id when booting'),
            'argonautTaskIdFile', TRUE,
            '/tmp/argonaut'
          ),
          new StringAttribute(
            _('Log directory'), _('Directory in which argonaut client should write its logs'),
            'argonautClientLogDir', TRUE,
            '/var/log/argonaut'
          ),
        ]
      ],
      'ssl' => [
        'name'  => _('SSL paths'),
        'attrs' => [
          new TrimmedStringAttribute(
            _('Key'), _('Path to the private key file on Argonaut client'),
            'argonautClientKeyPath', FALSE,
            '/etc/ssl/private/argonaut.key'
          ),
          new TrimmedStringAttribute(
            _('Certificate'), _('Path to the certificate file on Argonaut client'),
            'argonautClientCertPath', FALSE,
            '/etc/ssl/certs/argonaut.cert'
          ),
          new TrimmedStringAttribute(
            _('CA certificate'), _('Path to the CA certificate file on Argonaut client'),
            'argonautClientCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
          new StringAttribute(
            _('Name for peer verification'), _('The certificate name used for peer verification'),
            'argonautClientCertCN', FALSE
          ),
        ]
      ],
      'serviceNames' => [
        'name'  => _("Service names"),
        'attrs' => [
          new ArgonautServiceNameAttribute(_("Argonaut service names"), "argonautServiceName"),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['argonautClientProtocol']->setManagedAttributes(
      [
        'disable' => [
          'http' => [
            'argonautClientKeyPath','argonautClientCertPath','argonautClientCaCertPath',
            'argonautClientCertCN'
          ]
        ]
      ]
    );
  }

  /*!
   * \brief Test whether argonaut client is active, either directly or through a group
   */
  public function isActiveOrInherited (): bool
  {
    return (parent::isActive() || $this->member_of_group);
  }
}
