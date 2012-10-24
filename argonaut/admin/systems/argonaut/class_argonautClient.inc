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

class ArgonautServiceNameAttribute extends CompositeAttribute
{
  static private $argonautService_defaults = array(
    "folder"              => "/etc/init.d",
    "serviceCUPS"         => "cups",
    "serviceMail"         => "postfix",
    "serviceNTP"          => "ntpd",
    "serviceIMAP"         => "cyrus-imapd",
    "serviceTerminal"     => "ssh",
    "serviceLDAP"         => "slapd",
    "serviceShare"        => "samba",
    "serviceSpamAssassin" => "spamassassin",
    "serviceAntiVirus"    => "clamav-daemon",
    "serviceDHCP"         => "isc-dhcp-server",
    "serviceDNS"          => "bind9",
    "argonautServer"      => "argonaut-server"
  );

  static private $argonautService_blacklist = array(
    "argonautDNSConfig",
    "argonautMirrorConfig",
    "argonautFuseConfig",
    "serviceQuota"
  );

  function __construct ($description, $ldapName, $acl = "")
  {
    parent::__construct ($description, $ldapName, array(), "", "", $acl);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    $this->loadServiceNames();
  }

  function loadServiceNames ()
  {
    $this->attributes = array();
    // Get services tabs list from config
    $services = $this->plugin->config->data['TABS']['SERVERSERVICE'];
    // Add fake "folder" tab at the beginning
    $services = array_merge(array(array('CLASS' => 'folder')), $services);

    foreach ($services as $tab) {
      $name = $tab['CLASS'];
      if (in_array($name, self::$argonautService_blacklist)) {
        continue;
      }
      $value = (isset(self::$argonautService_defaults[$name])?self::$argonautService_defaults[$name]:"");
      $this->attributes[$name] = new StringAttribute ($name, "Service $name",
                                                      "service_$name", FALSE,
                                                      $value, $this->getAcl());
    }
  }

  function loadAttrValue ($attrs)
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
    return array_values(array_map (function ($a) { return $a->getLabel().":".$a->getValue(); },
                      $this->attributes));
  }

  function getServiceNames ()
  {
    return array_map (function ($a) { return $a->getLdapName(); },
                      $this->attributes);
  }
}

class argonautClient extends simplePlugin
{
  var $objectclasses  = array("argonautClient");
  var $displayHeader  = TRUE;

  var $member_of_ogroup = FALSE;

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _("Argonaut client settings"),
        'attrs' => array(
          new IntAttribute (
            _('Client port'), _('Port used by argonaut client for JSON-RPC'),
            'argonautClientPort', TRUE,
            0 /*min*/, FALSE /*no max*/, 8081 /*default value*/
          ),
          new StringAttribute (
            _('WakeOnLan interface'), _('Interface used by argonaut for WakeOnLan'),
            'argonautClientWakeOnLanInterface', TRUE,
            'eth0'
          ),
          new StringAttribute (
            _('TaskId file'), _('File which argonaut will use to store its task id when booting'),
            'argonautTaskIdFile', TRUE,
            '/tmp/argonaut'
          ),
          new StringAttribute (
            _('Log directory'), _('Directory in which argonaut client should write its logs'),
            'argonautClientLogDir', TRUE,
            '/var/log/argonaut'
          ),
        )
      ),
      'serviceNames' => array(
        'name'  => _("Service names"),
        'attrs' => array(
          new ArgonautServiceNameAttribute (_("Argonaut service names"), "argonautServiceName"),
        )
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search(
      "(&(objectClass=argonautClient)(objectClass=gotoWorkstationTemplate)(member=".LDAP::prepare4filter($this->dn)."))",
      array("cn", "argonautClientPort", "argonautClientWakeOnLanInterface", "argonautTaskIdFile", "argonautClientLogDir")
    );
    if ($ldap->count() == 1) {
      $this->member_of_ogroup = TRUE;
      $attrs = $ldap->fetch();
      $this->o_group_attrs = $attrs;
    }
  }

  function execute()
  {
    $smarty = get_smarty();
    $display = parent::execute();
    if (!$this->displayPlugin && $this->member_of_ogroup) {
      $attrs = array("cn", "argonautClientPort", "argonautClientWakeOnLanInterface", "argonautTaskIdFile", "argonautClientLogDir");

      foreach ($attrs as $attr) {
        $value = (isset($this->o_group_attrs[$attr][0])?$this->o_group_attrs[$attr][0]:"");
        $smarty->assign("group_$attr", $value);
      }

      $display .= $smarty->fetch(get_template_path('argonaut_client_inheritance.tpl', TRUE, dirname(__FILE__)));
    }
    return $display;
  }

  // We also need this function that return some information about the plugin
  static function plInfo ()
  {
    return array(
      "plShortName"     => _("Argonaut client"),
      "plDescription"   => _("Edit argonaut client settings"),
      "plSelfModify"    => FALSE,
      "plSection"       => array("administration"),
      "plCategory"      => array("workstation","server","ogroups"),
      "plObjectType"    => array("workstation","server","ogroup"),

      // But simplePlugin can generate the ACL list for us
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}

?>
