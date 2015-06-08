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

class ActionAttribute extends SelectAttribute
{
  private $isTerminal;

  function __construct ($label, $description, $ldapName, $isTerminal = FALSE)
  {
    parent::__construct($label, $description, $ldapName, FALSE);
    $this->setInLdap(FALSE);
    $this->isTerminal = $isTerminal;
  }

  function setIsTerminal ($bool)
  {
    $this->isTerminal = $bool;
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->updateChoices();
    }
  }

  function loadPostValue ()
  {
    $this->postValue = '';
    $id = $this->getHtmlId();
    if (isset($_POST[$id.'_action'])) {
      parent::loadPostValue();
    }
  }

  function applyPostValue ()
  {
    if (!$this->disabled && $this->postValue != '') {
      $this->updateChoices();
      if (in_array($this->postValue, $this->choices)) {
        $action = $this->postValue;

        /* Check if we have an DaemonEvent for this action */
        if (class_available("supportDaemon") && class_available("DaemonEvent")) {
          $events = DaemonEvent::get_event_types();
          if (isset($events[$action])) {
            $tmp = new DaemonEvent($this->plugin->config, $action);
            $tmp->add_targets(array($this->plugin->netConfigDNS->macAddress));
            $tmp->set_type(TRIGGERED_EVENT);
            $o_queue = new supportDaemon();
            if (!$o_queue->append($tmp)) {
              msg_dialog::display(_("Infrastructure service"), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
            }
          } else {
            msg_dialog::display(_("Event error"), sprintf(_("Event %s is not available!"), $action), ERROR_DIALOG);
          }
        } else {
          msg_dialog::display(_("Event error"), _("Events are not available!"), ERROR_DIALOG);
        }
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $disabled = ($this->plugin->currently_installing?' disabled="disabled"':'');
    $button = '<input type="submit" name="'.$id.'_action" id="'.$id.'_action" value="{t}Execute{/t}"'.$disabled.'/>';
    return parent::renderFormInput().$this->renderAcl($button);
  }

  function updateChoices()
  {
    if (isset($this->plugin->gotoMode) && $this->plugin->gotoMode) {
      $this->setDisabled(TRUE);
      return;
    }
    $this->setDisabled(FALSE);
    //TODO : the list should be based on DaemonEvent::get_event_types
    /* Check if workstation is online */
    if ($this->isTerminal) {
      $actions = array('' => '');
    } else {
      $actions = array( // TODO : remove these action if there is no deployment plugin to use them (FAI, OPSI, …)
        ''          => '',
        'Deployment.reinstall' => _('(Re)Install'),
        'Deployment.update'    => _('Software update'),
        'Deployment.memcheck'  => _('Memory test')
      );
    }
    if ($this->plugin->ping()) {
      $actions['System.halt']         = _('Switch off');
      $actions['Deployment.reboot']   = _('Reboot');
    } else {
      $actions['Deployment.wake'] = _('Wake up');
    }
    $this->setChoices(array_keys($actions), array_values($actions));
  }
}

class workstationGeneric extends simplePlugin
{
  var $mainTab = TRUE;

  var $netConfigDNS;

  var $objectclasses = array("top", "gotoWorkstation", "GOhard");

  var $fai_activated = FALSE;

  var $currently_installing = FALSE;
  var $sdaemon_available    = FALSE;

  var $inheritance = array("gotoWorkstationTemplate" => "member");

  var $preInitAttributes = array('networkSettings');

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ($word = NULL, $rdn = NULL)
  {
    if ($word === NULL) {
      $word = _('workstation');
    }
    if ($rdn === NULL) {
      $rdn = get_ou("workstationRDN");
    }
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute (
            _('Name'),
            sprintf(_('The name of the %s'), $word),
            'cn',
            TRUE
          ),
          new StringAttribute (
            _('Description'),
            sprintf(_('A short description of the %s'), $word),
            'description',
            FALSE
          ),
          new StringAttribute (
            _('Location'),
            sprintf(_('The location of the %s'), $word),
            'l',
            FALSE
          ),
          new BaseSelectorAttribute($rdn),
          new BooleanAttribute (
            sprintf(_('Lock this %s'), $word),
            sprintf(_('This will prevent the %s from being reinstalled'), $word),
            'gotoMode',
            FALSE,
            FALSE,
            '',
            'locked',
            'active'
          ),
        )
      ),
      'servers' => array(
        'name'  => _('Servers'),
        'icon'  => 'geticon.php?context=devices&icon=server&size=16',
        'attrs' => array(
          new SelectAttribute (
            _('Syslog server'),
            sprintf(_('The syslog server the %s should be using'), $word),
            'gotoSyslogServer',
            FALSE
          ),
          new BooleanAttribute (
            _('Inherit NTP servers'),
            _('Inherit time server attributes from ogroup'),
            'inheritTimeServer',
            FALSE
          ),
          new SetAttribute (
            new SelectAttribute (
              _('NTP servers'),
              sprintf(_('The NTP servers for the %s'), $word),
              'gotoNtpServer',
              FALSE
            )
          ),
          new BooleanAttribute (
            _('Inherit LDAP servers'),
            _('Inherit LDAP server attributes from ogroup'),
            'inheritLDAPServer',
            FALSE
          ),
          new OrderedArrayAttribute (
            new SelectAttribute (
              _('LDAP servers'),
              sprintf(_('The LDAP servers for the %s'), $word),
              'gotoLdapServer',
              FALSE
            )
          ),
        )
      ),
      'actions' => array(
        'name' => _('Action'),
        'icon'  => 'geticon.php?context=types&icon=action&size=16',
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
        'template'  => get_template_path('network_section.tpl', TRUE, dirname(__FILE__)),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
        'attrs'     => array(new NetworkSettingsAttribute()),
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

    $this->init_server_section();

    $this->netConfigDNS->MACisMust = FALSE;

    $this->updateAttributesValues();
    $this->prepareSavedAttributes();
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'cn='.$this->cn.','.get_ou('workstationRDN').$this->base;
  }

  protected function init_server_section()
  {
    $this->attributesAccess['inheritTimeServer']->setInLdap(FALSE);
    $this->attributesAccess['inheritTimeServer']->setManagedAttributes(
      array(
        'erase' => array(
          TRUE => array('gotoNtpServer'),
        )
      )
    );

    $this->attributesAccess['inheritLDAPServer']->setInLdap(FALSE);
    $this->attributesAccess['inheritLDAPServer']->setManagedAttributes(
      array(
        'erase' => array(
          TRUE => array('gotoLdapServer'),
        )
      )
    );

    /* Create used ntp server array */
    $ntpServers = $this->attributesAccess['gotoNtpServer']->getValue();
    /* Set inherit checkbox state */
    if ($this->member_of_group && (in_array("default", $ntpServers) || (count($ntpServers) == 0))) {
      $this->attributesAccess['inheritTimeServer']->setValue(TRUE);
      $this->attributesAccess['gotoNtpServer']->setValue(array());
    } else {
      $this->attributesAccess['inheritTimeServer']->setValue(FALSE);
    }

    /* Create used ldap server array */
    $ldapServers = $this->attributesAccess['gotoLdapServer']->getValue();
    /* Set inherit checkbox state */
    if ($this->member_of_group && (count($ldapServers) == 0)) {
      $this->attributesAccess['inheritLDAPServer']->setValue(TRUE);
    } else {
      $this->attributesAccess['inheritLDAPServer']->setValue(FALSE);
    }

    /* Disable inheritance if we're not in a group */
    if (!$this->member_of_group) {
      $this->attributesAccess['inheritTimeServer']->setDisabled(TRUE);
      $this->attributesAccess['inheritTimeServer']->setVisible(FALSE);
      $this->attributesAccess['inheritLDAPServer']->setDisabled(TRUE);
      $this->attributesAccess['inheritLDAPServer']->setVisible(FALSE);
    }

    /* Create available ntp options */
    $tmp = array();
    foreach ($this->config->data['SERVERS']['NTP'] as $key => $server) {
      if ($server == "default") {
        continue;
      }
      $tmp[] = $server;
    }
    $this->attributesAccess['gotoNtpServer']->attribute->setChoices($tmp);
    if (count($tmp) == 0) {
      $this->attributesAccess['gotoNtpServer']->setDisabled(TRUE);
      $this->attributesAccess['inheritTimeServer']->setValue(TRUE);
      $this->attributesAccess['inheritTimeServer']->setDisabled(TRUE);
    }

    /* Create available ldap options */
    $tmp = array();
    foreach ($this->config->data['SERVERS']['LDAP'] as $dn => $infos) {
      $tmp[$dn] = $infos['goLdapURI'][0].$infos['goLdapBase'][0];
    }
    $this->attributesAccess['gotoLdapServer']->attribute->setChoices(
      array_keys($tmp), array_values($tmp)
    );
    if (count($tmp) == 0) {
      $this->attributesAccess['gotoLdapServer']->setDisabled(TRUE);
      $this->attributesAccess['inheritLDAPServer']->setValue(TRUE);
      $this->attributesAccess['inheritLDAPServer']->setDisabled(TRUE);
    }

    /* Create an array of all Syslog servers */
    $tmp        = $this->config->data['SERVERS']['SYSLOG'];
    $tmp_output = $tmp;
    if (($key = array_search("default", $tmp)) !== FALSE) {
      $tmp[$key] = "";
      if ($this->member_of_group) {
        $tmp_output[$key] = _('inherited');
      }
    }
    $this->attributesAccess['gotoSyslogServer']->setChoices($tmp, $tmp_output);
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();
    $this->netConfigDNS->resetCopyInfos();
  }

  function ping()
  {
    if (!$this->sdaemon_available) {
      return FALSE;
    }
    $sdaemon = new supportDaemon();
    if (!$sdaemon->is_available()) {
      $this->sdaemon_available = FALSE;
      return FALSE;
    }
    return $sdaemon->ping($this->netConfigDNS->macAddress);
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

      /* Remove all accessTo/trust dependencies */
      update_accessTo($this->cn, "");

      /* Clean queue form entries with this mac
       */
      if (class_available("supportDaemon") && tests::is_mac($this->netConfigDNS->orig_macAddress)) {
        $q = new supportDaemon();
        $q->clean_queue_from_mac($this->netConfigDNS->orig_macAddress);
      }
    }
  }

  protected function ldap_save($cleanup = TRUE)
  {
    $activate = ($this->attributesAccess['gotoMode']->hasChanged() &&
                 $this->attributesAccess['gotoMode']->getValue());
    parent::ldap_save($cleanup);
    $this->netConfigDNS->cn = $this->cn;
    $this->netConfigDNS->save();

    if ($activate && $this->sdaemon_available) {
      /* Send installation activation
       */
      $events = DaemonEvent::get_event_types();
      $o_queue = new supportDaemon();
      if (isset($events['installation_activation'])) {
        $evt = 'installation_activation';
        $tmp = new DaemonEvent($this->config, $evt);
        $tmp->set_type(TRIGGERED_EVENT);
        $tmp->add_targets(array($this->netConfigDNS->macAddress));
        $o_queue->append($tmp);
        if ($o_queue->is_error()) {
          msg_dialog::display(_("Infrastructure service"), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
        }
      }
    }
  }

  function is_modal_dialog()
  {
    return (isset($this->netConfigDNS->dialog) && $this->netConfigDNS->dialog);
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Workstation'),
      'plDescription' => _('Workstation information'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('workstation' =>
        array(
          'name'        => _('Workstation'),
          'description' => _('Workstation'),
          'filter'      => 'objectClass=gotoWorkstation',
          'ou'          => get_ou('workstationRDN'),
          'icon'        => 'geticon.php?context=devices&icon=computer&size=16',
          'mainAttr'    => 'cn',
        )
      ),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}
?>
