<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

        if (class_available('supportDaemon')) {
          $events = argonautAction::get_event_types();
          if (isset($events[$action])) {
            $o_queue = new supportDaemon();
            if (!$o_queue->append_call($action, array($this->plugin->netConfigDNS->macAddress), array())) {
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
    //TODO : the list should be based on argonautAction::get_event_types
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

class systemSimplePluginGlue extends simplePlugin
{
  protected $previousNetworkSavedAttributes;

  public function __get($name)
  {
    if ($name == 'ipHostNumber') {
      return array_merge(array($this->netConfigDNS->$name), $this->netConfigDNS->additionalHostNumbers);
    } elseif ($name == 'macAddress') {
      return $this->netConfigDNS->$name;
    } else {
      return parent::__get($name);
    }
  }

  public function __set($name, $value)
  {
    if ($name == 'ipHostNumber') {
      if (is_array($value)) {
        $this->netConfigDNS->$name                  = array_shift($value);
        $this->netConfigDNS->additionalHostNumbers  = $value;
      } else {
        $this->netConfigDNS->$name = $value;
      }
    } elseif ($name == 'macAddress') {
      $this->netConfigDNS->$name = $value;
    } else {
      return parent::__set($name, $value);
    }
  }

  public function __isset($name)
  {
    if (($name == 'ipHostNumber') || ($name == 'macAddress')) {
      return isset($this->netConfigDNS->$name);
    } else {
      return parent::__isset($name);
    }
  }

  protected function attributeHaveChanged($field)
  {
    if ($field == 'ipHostNumber') {
      return array_differs($this->attributeInitialValue($field), $this->attributeValue($field));
    } else {
      return parent::attributeHaveChanged($field);
    }
  }

  protected function attributeValue($field)
  {
    if ($field == 'ipHostNumber') {
      return $this->ipHostNumber;
    } else {
      return parent::attributeValue($field);
    }
  }

  protected function attributeInitialValue($field)
  {
    if ($field == 'ipHostNumber') {
      $initialValue = $this->previousNetworkSavedAttributes['ipHostNumber'];
      if (is_array($initialValue)) {
        return $initialValue;
      } else {
        return array($initialValue);
      }
    } else {
      return parent::attributeInitialValue($field);
    }
  }

  protected function ldap_save($cleanup = TRUE)
  {
    $this->previousNetworkSavedAttributes = $this->netConfigDNS->saved_attributes;
    return parent::ldap_save($cleanup);
  }
}

class workstationGeneric extends systemSimplePluginGlue
{
  var $mainTab = TRUE;

  var $netConfigDNS;

  var $objectclasses = array("top", "gotoWorkstation", "GOhard");

  var $fai_activated = FALSE;

  var $currently_installing = FALSE;
  var $sdaemon_available    = FALSE;

  var $inheritance = array("gotoWorkstationTemplate" => "member");

  var $preInitAttributes = array('cn', 'networkSettings');

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

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ($word = NULL, $rdn = NULL)
  {
    if ($word === NULL) {
      $word = _('workstation');
    }
    if ($rdn === NULL) {
      $rdn = get_ou('workstationRDN');
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
    return 'cn='.ldap_escape_dn($this->cn).','.get_ou('workstationRDN').$this->base;
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

    /* Create used ntp server array */
    $ntpServers = $this->attributesAccess['gotoNtpServer']->getValue();
    /* Set inherit checkbox state */
    if ($this->member_of_group && (in_array("default", $ntpServers) || (count($ntpServers) == 0))) {
      $this->attributesAccess['inheritTimeServer']->setValue(TRUE);
      $this->attributesAccess['gotoNtpServer']->setValue(array());
    } else {
      $this->attributesAccess['inheritTimeServer']->setValue(FALSE);
    }

    /* Disable inheritance if we're not in a group */
    if (!$this->member_of_group) {
      $this->attributesAccess['inheritTimeServer']->setDisabled(TRUE);
      $this->attributesAccess['inheritTimeServer']->setVisible(FALSE);
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
      /* Send installation activation */
      $events = argonautAction::get_event_types();
      if (isset($events['installation_activation'])) {
        $o_queue = new supportDaemon();
        $o_queue->append_call('installation_activation', array($this->netConfigDNS->macAddress), array());
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
}
?>
