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

class argonautClient extends plugin
{
  var $parent;
  var $config;

  var $objectclasses    = array("argonautClient");
  var $attributes = array("argonautClientPort","argonautClientWakeOnLanInterface",
                          "argonautTaskIdFile","argonautClientLogDir",
                          "argonautServiceName");
  var $argonautClientPort               = 8081;
  var $argonautClientWakeOnLanInterface = "eth0";
  var $argonautTaskIdFile               = "/tmp/argonaut";
  var $argonautClientLogDir             = "/var/log/argonaut";

  var $argonautServiceName = array();

  var $member_of_ogroup = FALSE;

  var $activateArgonaut = FALSE;

  var $ignore_account   = TRUE;

  function __construct(&$config,$dn,$parent)
  {
    $this->config = $config;
    $this->parent = $parent;

    if(is_object($parent) && $dn != "" && $dn != "new"){
      plugin::plugin($config,$dn,$parent);
      if(isset($this->attrs['objectClass'])) {
        $this->activateArgonaut = in_array("argonautClient",$this->attrs['objectClass']);
      }
    }

    if(!isset($this->parent->by_object['ogroup'])){
      $ldap = $this->config->get_ldap_link();
      $ldap->cd ($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gotoWorkstationTemplate)(member=".LDAP::prepare4filter($this->dn)."))",array("cn"));
      $this->member_of_ogroup = ($ldap->count() >= 1);
    }

    $argonautService_defaults = array(
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
    $argonautService_blacklist = array(
      "argonautDNSConfig",
      "argonautMirrorConfig",
      "argonautFuseConfig",
      "serviceQuota"
    );

    $this->argonautServiceName = array();
    $this->argonautServiceName["folder"] = $argonautService_defaults["folder"];
    foreach($this->config->data['TABS']['SERVERSERVICE'] as $tab) {
      $this->argonautServiceName[$tab['CLASS']] = "";
      if (isset($argonautService_defaults[$tab['CLASS']])) {
        $this->argonautServiceName[$tab['CLASS']] = $argonautService_defaults[$tab['CLASS']];
      }
    }
    if (isset($this->attrs['argonautServiceName'])) {
      for($i = 0 ; $i < $this->attrs['argonautServiceName']['count']; $i++) {
        $serv = explode(':',$this->attrs['argonautServiceName'][$i]);
        if (isset($this->argonautServiceName[$serv[0]])) {
          $this->argonautServiceName[$serv[0]] = $serv[1];
        }
      }
    }
    foreach($argonautService_blacklist as $service) {
      unset($this->argonautServiceName[$service]);
    }

  }


  function execute()
  {
    $smarty = get_smarty();
    foreach ($this->attributes as $attribute) {
      $smarty->assign("$attribute" , $this->$attribute);
    }
    $smarty->assign("member_of_ogroup", $this->member_of_ogroup);
    $smarty->assign("activateArgonaut", $this->activateArgonaut);

    $smarty->assign("argonautACL", $this->getacl("argonaut"));

    return($smarty->fetch(get_template_path('argonautclient.tpl', TRUE,dirname(__FILE__))));
  }

  function save()
  {
    plugin::save();

    if (!$this->activateArgonaut) {
      unset($this->attrs['objectClass'][array_search("argonautClient", $this->attrs['objectClass'])]);
      foreach ($this->attributes as $attribute) {
        $this->attrs[$attribute] = array();
      }
    } else {
      $this->attrs['argonautServiceName'] = array();
      foreach ($this->argonautServiceName as $nameFD => $namePC) {
        $this->attrs['argonautServiceName'][] = "$nameFD:$namePC";
      }
    }

    /* Check if this is a new entry ... add/modify */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->dn,array("objectClass"));
    if($ldap->count()){
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
    }else{
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
    }
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
    if($this->initially_was_account){
      $this->handle_post_events("modify");
      new log("modify","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      $this->handle_post_events("add");
      new log("create","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }
  }

  function save_object()
  {
    if(isset($_POST['argonautClientPosted'])) {
      plugin::save_object();
      if($this->acl_is_writeable("argonaut")) {
        if(isset($_POST['activateArgonaut'])) {
          $this->activateArgonaut = true;
        } else {
          $this->activateArgonaut = false;
        }

        if ($this->member_of_ogroup) {
          $this->activateArgonaut = !$this->activateArgonaut;
          /* if we're in a group, the checkbox means "inherit",
           therefore deactivate specific argonaut config for this computer.*/
        }

        foreach ($this->attributes as $attribute) {
          if($attribute=="activateArgonaut") {
            continue;
          }
          if(isset($_POST[$attribute])) {
            $this->$attribute = $_POST[$attribute];
          }
        }
        foreach ($this->argonautServiceName as $name => $value) {
          if(isset($_POST["service_$name"])) {
            $this->argonautServiceName[$name] = $_POST["service_$name"];
          }
        }
      }
    }
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Argonaut client"),
          "plDescription" => _("Argonaut client settings"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 30,
          "plSection"     => array("administration"),
          "plCategory"    => array("workstation","server","ogroups"),

          "plProvidedAcls"=> array()
            ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
