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

class winstationGeneric extends simplePlugin
{
  var $mainTab = TRUE;

  var $netConfigDNS;

  var $objectclasses = array('posixAccount','account','sambaSamAccount','top');

  var $preInitAttributes = array('networkSettings');

  var $currently_installing = FALSE;
  var $sdaemon_available    = FALSE;

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Winstation'),
      'plDescription' => _('Windows workstation information'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array(
        'winstation' => array(
          'name'        => _('Windows workstation'),
          'description' => _('Windows workstation'),
          'filter'      => '(&(objectClass=sambaSamAccount)(uid=*$))',
          'icon'        => 'geticon.php?context=devices&icon=computer-windows&size=16',
          'ou'          => get_ou('sambaMachineAccountRDN'),
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ($word = NULL, $rdn = NULL)
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HiddenAttribute ('uid'),
          new HiddenAttribute ('homeDirectory', TRUE, '/dev/null'),
          new HiddenAttribute ('sambaSID'),
          new HiddenAttribute ('uidNumber', TRUE, 1000),
          new HiddenAttribute ('gidNumber', TRUE, 515),
          new HiddenAttribute ('sambaAcctFlags', TRUE, "[W          ]"),
          new CompositeAttribute (
            _('The name of the winstation'),
            'cn',
            array(
              new HostNameAttribute (
                _('Name'), _('The name of the winstation'),
                'cn_field', TRUE
              ),
            ),
            '/^([^\\$]+)\\$$/', '%s$'
          ),
          new StringAttribute (
            _('Description'), _('A short description of the winstation'),
            'description', FALSE
          ),
          new SelectAttribute (
            _('Domain'), _('Samba domain name'),
            'sambaDomainName', TRUE
          ),
          new StringAttribute (
            _('Location'), _('The location of the winstation'),
            'l', FALSE
          ),
          new BaseSelectorAttribute(get_winstations_ou()),
        )
      ),
      'actions' => array(
        'name' => _('Action'),
        'icon'  => get_template_path("images/rocket.png"),
        'attrs' => array(
          new ActionAttribute(
            _('Action'),
            _('Select action to execute for this terminal'),
            'FAIstate'
          ),
        )
      ),
      'network' => array(
        'name'      => _('Network settings'),
        'attrs'     => array(new NetworkSettingsAttribute('uid', array('ipHost', 'ieee802Device'))),
        'template'  => get_template_path('../network_section.tpl', TRUE, dirname(__FILE__)),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    if (class_available('supportDaemon')) {
      $s_daemon = new supportDaemon();
      $this->sdaemon_available = $s_daemon->is_available();
    } else {
      $this->sdaemon_available = FALSE;
    }

    parent::__construct($config, $dn, $object);

    /* Check if this host is currently in installation process*/
    if ($this->sdaemon_available) {
      $this->currently_installing = $s_daemon->is_currently_installing($this->netConfigDNS->macAddress);
    }

    $this->attributesAccess['cn']->setUnique(TRUE);
    $this->attributesAccess['uid']->setUnique(TRUE);
    $this->attributesAccess['sambaDomainName']->setChoices(array_keys($this->config->data['SERVERS']['SAMBA']));
    $this->attributesAccess['sambaDomainName']->setInLdap(FALSE);

    // Get samba domain and its sid/rid base
    if ($this->sambaSID != "") {
      $this->SID = preg_replace ("/-[^-]+$/", "", $this->sambaSID);
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=sambaDomain)(sambaSID=$this->SID))", array("sambaAlgorithmicRidBase","sambaDomainName"));
      if ($ldap->count() != 0) {
        $attrs = $ldap->fetch();
        if (isset($attrs['sambaAlgorithmicRidBase'])) {
          $this->ridBase = $attrs['sambaAlgorithmicRidBase'][0];
        } else {
          $this->ridBase = $this->config->get_cfg_value("sambaRidBase");
        }
        if ($this->sambaDomainName == "") {
          $this->sambaDomainName = $attrs['sambaDomainName'][0];
        }
      } else {
        // Fall back to a 'DEFAULT' domain, if none was found in LDAP.
        if ($this->sambaDomainName == "") {
          $this->sambaDomainName = "DEFAULT";
        }

        // Nothing in ldap, use configured sid and rid values.
        $this->ridBase  = $this->config->get_cfg_value("sambaRidBase");
        $this->SID      = $this->config->get_cfg_value("sambaSid");
      }
    }
  }

  function ping()
  {
    if (!$this->sdaemon_available) {
      return FALSE;
    }
    if ($this->netConfigDNS->macAddress == "") {
      return FALSE;
    }
    $sdaemon = new supportDaemon();
    if (!$sdaemon->is_available()) {
      $this->sdaemon_available = FALSE;
      return FALSE;
    }
    return $sdaemon->ping($this->netConfigDNS->macAddress);
  }

  function save_object()
  {
    parent::save_object();
    $this->attributesAccess['uid']->setValue($this->attributesAccess['cn']->getValue());
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();
    $this->netConfigDNS->resetCopyInfos();
  }

  function set_acl_base($base)
  {
    plugin::set_acl_base($base);
    $this->netConfigDNS->set_acl_base($base);
  }

  function set_acl_category($cat)
  {
    plugin::set_acl_category($cat);
    $this->netConfigDNS->set_acl_category($cat);
  }

  function remove_from_parent()
  {
    if (!$this->initially_was_account) {
      return;
    }

    if ($this->acl_is_removeable()) {
      $this->netConfigDNS->remove_from_parent();

      parent::remove_from_parent();

      /* Delete references to object groups */
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
      while ($ldap->fetch()) {
        $og = new ogroup($this->config, $ldap->getDN());
        unset($og->member[$this->dn]);
        $og->save();
      }
    }
  }

  function prepare_save()
  {
    $this->attributesAccess['uid']->setValue($this->attributesAccess['cn']->getValue());
    parent::prepare_save();

    //verify that the $this->sambaSID exist and that it is correct
    //yes do nothing
    //no get it from config and generate a new sid

    if (($this->sambaSID == "") || (substr_count($this->sambaSID, "-" ) < 7)) {
      $this->SID      = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'];
      $this->ridBase  = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'];

      // create sambaSID for workstation
      $uidNumber = $this->uidNumber;
      do {
        $sid = $this->SID."-".($uidNumber * 2 + $this->ridBase);
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(sambaSID=$sid)", array("sambaSID"));
        $uidNumber++;
      } while ($ldap->count() != 0);
      $uidNumber--;
      $this->attrs['sambaSID']  = $sid;
      $this->attrs['uidNumber'] = $uidNumber + $this->ridBase;
    }
  }

  protected function ldap_save($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);
    $this->netConfigDNS->cn = preg_replace("/\\\$\$/", "", $this->cn);
    $this->netConfigDNS->save();
  }

  function is_modal_dialog()
  {
    return (isset($this->netConfigDNS->dialog) && $this->netConfigDNS->dialog);
  }

  function compute_dn()
  {
    $this->attributesAccess['uid']->setValue($this->attributesAccess['cn']->getValue());
    return 'uid='.$this->attributesAccess['uid']->getValue().','.get_winstations_ou().$this->base;
  }
}
?>
