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

class NetworkSettingsDialog
{
  private $attribute;
  private $display;

  function __construct (&$attribute, &$display)
  {
    $this->attribute = &$attribute;
    $this->display = &$display;
  }

  function execute ()
  {
    $this->display = $this->attribute->netConfigDNS->execute();
    if (is_object($this->attribute->netConfigDNS->dialog)) {
      return $this->display;
    }
    return FALSE;
  }
}

class NetworkSettingsAttribute extends Attribute
{
  public $netConfigDNS;
  private $display;

  function __construct ()
  {
    parent::__construct("Network Settings", "", "networkSettings", FALSE, "");
    $this->setInLdap(FALSE);
  }

  function setManagedAttributes ($dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for BaseSelectorAttribute');
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->plugin->netConfigDNS =& $this->netConfigDNS;
      $this->netConfigDNS = new termDNS($this->plugin->config, $this->plugin, $this->plugin->objectclasses, TRUE);
      $this->netConfigDNS->MACisMust = TRUE;
      $this->display = $this->netConfigDNS->execute();
    }
  }

  function renderAttribute(&$attributes)
  {
    $attributes[$this->getLdapName()] = $this->display;
  }

  function loadPostValue ()
  {
  }

  function applyPostValue ()
  {
    if (!$this->disabled) {
      /* Refresh base */
      $this->netConfigDNS->save_object();
      $this->display = $this->netConfigDNS->execute();
      if (is_object($this->netConfigDNS->dialog)) {
        $this->plugin->openDialog(new NetworkSettingsDialog($this, $this->display));
      }
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      return $this->netConfigDNS->check();
    }
  }
}

class workgeneric extends simplePlugin
{
  var $mainTab = TRUE;

  var $netConfigDNS;

  /* attribute list for save action */
  //~ var $attributes= array("gotoMode", "gotoSyslogServer", "gotoNtpServer",
      //~ "gotoFloppyEnable", "gotoCdromEnable", "cn", "gotoSndModule",
      //~ "ghCpuType", "ghMemSize", "ghUsbSupport", "description",
      //~ "ghGfxAdapter", "ghSoundAdapter", "gotoLastUser", "l","FAIscript");

  var $objectclasses= array("top", "gotoWorkstation", "GOhard");

  var $validActions   = array("reboot" => 1, "localboot" => 1, "halt" => 1, "update" => 1, "reinstall" => 1,
                            "rescan" => 1, "wakeup" => 1, "memcheck" => 1, "sysinfo" => 1);

  var $fai_activated = FALSE;

  var $member_of_ogroup = FALSE;

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new StringAttribute (
            _('Workstation name'),
            _('The name of the workstation'),
            'cn',
            TRUE
          ),
          new StringAttribute (
            _('Description'),
            _('A short description of the workstation'),
            'description',
            FALSE
          ),
          new StringAttribute (
            _('Location'),
            _('The location of the workstation'),
            'l',
            FALSE
          ),
          new BaseSelectorAttribute(get_ou("workstationRDN")),
          new BooleanAttribute (
            _('Lock this workstation'),
            _('This will prevent the workstation from being reinstalled'),
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
        'attrs' => array(
          new SelectAttribute (
            _('Syslog server'),
            _('The syslog server the workstation should be using'),
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
              _('The NTP servers for the workstation'),
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
              _('The LDAP servers for the workstation'),
              'gotoLdapServer',
              FALSE
            )
          ),
        )
      ),
      'network' => array(
        'name' => _('Network settings'),
        'attrs' => array(new NetworkSettingsAttribute()),
        'template' => get_template_path('network_section.tpl', TRUE, dirname(__FILE__)),
      ),
      //~ 'actions' => array(
        //~ 'name' => _('Action'),
        //~ 'attrs' => array(
          //~ new ActionAttribute (),
        //~ )
      //~ ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    //~ $tmp = $config->search("faiManagement", "CLASS",array('menu','tabs'));
    //~ if (!empty($tmp)) {
      //~ $this->fai_activated = TRUE;
    //~ }
//~

    if (!isset($this->parent->by_object['ogroup'])) {
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gotoWorkstationTemplate)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
      $this->member_of_ogroup = ($ldap->count() >= 1);
    }

    /* Check if this host is currently in installation process*/
    //~ if (class_available("supportDaemon") && class_available("DaemonEvent")) {
      //~ $o = new supportDaemon();
      //~ $this->currently_installing = $o->is_currently_installing($this->netConfigDNS->macAddress);
    //~ }

    $this->attributesInfo['main']['attrs']['cn']->setUnique(TRUE);

    $this->attributesInfo['servers']['attrs']['inheritTimeServer']->setInLdap(FALSE);
    $this->attributesInfo['servers']['attrs']['inheritTimeServer']->setManagedAttributes(
      array(
        'erase' => array(
          TRUE => array(
            'servers' => array('gotoNtpServer'),
          )
        )
      )
    );

    $this->attributesInfo['servers']['attrs']['inheritLDAPServer']->setInLdap(FALSE);
    $this->attributesInfo['servers']['attrs']['inheritLDAPServer']->setManagedAttributes(
      array(
        'erase' => array(
          TRUE => array(
            'servers' => array('gotoLdapServer'),
          )
        )
      )
    );

    /* Create used ntp server array */
    $ntpServers = $this->attributesInfo['servers']['attrs']['gotoNtpServer']->getValue();
    /* Set inherit checkbox state */
    if ($this->member_of_ogroup && (in_array("default",$ntpServers) || (count($ntpServers) == 0))) {
      $this->attributesInfo['servers']['attrs']['inheritTimeServer']->setValue(TRUE);
      $this->attributesInfo['servers']['attrs']['gotoNtpServer']->setValue(array());
    } else {
      $this->attributesInfo['servers']['attrs']['inheritTimeServer']->setValue(FALSE);
    }

    /* Create used ldap server array */
    $ldapServers = $this->attributesInfo['servers']['attrs']['gotoLdapServer']->getValue();
    /* Set inherit checkbox state */
    if ($this->member_of_ogroup && (count($ldapServers) == 0)) {
      $this->attributesInfo['servers']['attrs']['inheritLDAPServer']->setValue(TRUE);
    } else {
      $this->attributesInfo['servers']['attrs']['inheritLDAPServer']->setValue(FALSE);
    }

    /* Disable inheritance if we're not in a group */
    if (!$this->member_of_ogroup) {
      $this->attributesInfo['servers']['attrs']['inheritTimeServer']->setDisabled(TRUE);
      $this->attributesInfo['servers']['attrs']['inheritLDAPServer']->setDisabled(TRUE);
    }

    /* Create available ntp options */
    $tmp = array();
    foreach ($this->config->data['SERVERS']['NTP'] as $key => $server) {
      if($server == "default") {
        continue;
      }
      $tmp[] = $server;
    }
    $this->attributesInfo['servers']['attrs']['gotoNtpServer']->attribute->setChoices($tmp);

    /* Create available ldap options */
    $tmp = array();
    foreach ($this->config->data['SERVERS']['LDAP'] as $dn => $data) {
      for ($i = 0; $i < $data['goLdapBase']['count']; $i++) {
        $tmp[] = $data["cn"][0].":".$data["goLdapBase"][$i];
      }
    }
    $this->attributesInfo['servers']['attrs']['gotoLdapServer']->attribute->setChoices($tmp);

    /* Create an array of all Syslog servers */
    $tmp = $this->config->data['SERVERS']['SYSLOG'];
    $tmp_output = $tmp;
    if (($key = array_search("default", $tmp)) !== FALSE) {
      $tmp[$key] = "";
      if ($this->member_of_ogroup) {
        $tmp_output[$key] = _('inherited');
      }
    }
    $this->attributesInfo['servers']['attrs']['gotoSyslogServer']->setChoices($tmp, $tmp_output);
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

  //~ function execute()
  //~ {
    //~ /* Call parent execute */
    //~ plugin::execute();

    //~ if ((isset($_POST['saction'])) &&
        //~ (isset($_POST['action'])) &&
        //~ ($this->acl_is_writeable("FAIstate")) &&
        //~ isset($this->validActions[$_POST['saction']])) {
      //~ $action= $_POST['saction'];
//~
      //~ /* Check if we have an DaemonEvent for this action */
      //~ if (class_available("supportDaemon") && class_available("DaemonEvent_".$action)) {
        //~ $events = DaemonEvent::get_event_types(SYSTEM_EVENT | HIDDEN_EVENT);
        //~ if(isset($events['TRIGGERED']["DaemonEvent_".$action])){
          //~ $evt = $events['TRIGGERED']["DaemonEvent_".$action];
          //~ $tmp = new $evt['CLASS_NAME']($this->config);
          //~ $tmp->add_targets(array($this->netConfigDNS->macAddress));
          //~ $tmp->set_type(TRIGGERED_EVENT);
          //~ $o_queue = new supportDaemon();
          //~ if(!$o_queue->append($tmp)){
            //~ msg_dialog::display(_("Service infrastructure"),msgPool::siError($o_queue->get_error()),ERROR_DIALOG);
          //~ }
        //~ }
      //~ } else {
        //~ msg_dialog::display(_("Event error"),
                    //~ sprintf(_("Event '%s' is not available!"),$action),ERROR_DIALOG);
      //~ }
//~
//~
    //~ }

    //~ /* Fill templating stuff */
    //~ $smarty= get_smarty();
    //~ $smarty->assign("usePrototype", "true");
//~
//~
    //~ /* Check if workstation is online */
    //~ $o_queue = new supportDaemon();
    //~ $actions = array(
                    //~ "reinstall"    => _("Reinstall"),
                    //~ "update"       => _("Software update"),
                    //~ "memcheck"     => _("Memory test")
                    //~ );
    //~ if ($o_queue->ping($this->netConfigDNS->macAddress)){
        //~ $actions["halt"]    = _("Switch off");
        //~ $actions["reboot"]  = _("Reboot");
        //~ $actions["rescan"]  = _("Rescan hardware");
    //~ } else {
        //~ $actions["wakeup"]  = _("Wake up");
    //~ }
    //~ $smarty->assign("actions", $actions);
    //~ /* Arrays */
    //~ $smarty->assign("nfsservers", $this->config->data['SERVERS']['NFS']);
    //~ $smarty->assign("fai_activated",$this->fai_activated);
//~
    //~ /* Show main page */
    //~ $smarty->assign("currently_installing", $this->currently_installing);
    //~ return($smarty->fetch (get_template_path('workstation.tpl', TRUE, dirname(__FILE__))));
  //~ }

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

  //~ /* Check supplied data */
  //~ function check()
  //~ {
    //~ /* Skip IP & Mac checks if this is a template */
    //~ if($this->cn != "wdefault"){
      //~ $message= array_merge($message, $this->netConfigDNS->check());
    //~ }

    //~ /* Check if given name is a valid host/dns name */
    //~ if(!tests::is_valid_hostname($this->cn)){
      //~ $message[] = msgPool::invalid(_("Name"));
    //~ }
//~
    //~ /* Only systems with a valid ldap handle can be activated
     //~ */
    //~ if($this->gotoMode == "active" && $this->initial_gotoMode != "active"){
//~
      //~ if(isset($this->parent->by_object['workstartup']) &&
          //~ !count($this->parent->by_object['workstartup']->gotoLdapServers) &&
          //~ !$this->parent->by_object['workstartup']->gotoLdap_inherit){
//~
        //~ $message[] = _("A valid LDAP server assignement is missing!");
      //~ }
    //~ }else{
      //~ /* Warn the user, that this host is currently installing */
      //~ if($this->currently_installing && !$this->currently_installing_warned &&
          //~ !preg_match("/".preg_quote(get_ou("systemIncomingRDN"), '/')."/i",$this->orig_dn)){
//~
        //~ /* Force aborting without message dialog */
        //~ $message[] = "";
        //~ $this->currently_installing_warned = TRUE;
        //~ msg_dialog::display(_("Software deployment"),
            //~ _("This host is currently installing. If you want to save it, press 'OK'."),
            //~ CONFIRM_DIALOG);
      //~ }
    //~ }
//~
    //~ return ($message);
  //~ }


  /* Save to LDAP */
  //~ function save()
  //~ {
    //~ plugin::save();
//~
    //~ /* cn=default and macAddress=- indicates that this is a template */
    //~ if($this->cn == "wdefault"){
      //~ $this->netConfigDNS->macAddress = "-";
    //~ }
//~
  //~ }
  protected function ldap_save($cleanup = TRUE)
  {
    $activate =  ($this->attributesInfo['main']['attrs']['gotoMode']->hasChanged() &&
                  $this->attributesInfo['main']['attrs']['gotoMode']->getValue());
    parent::ldap_save($cleanup);
    $this->netConfigDNS->cn = $this->cn;
    $this->netConfigDNS->save();

    if ($activate && class_available("DaemonEvent")) {
      /* Send installation activation
       */
      $events = DaemonEvent::get_event_types(SYSTEM_EVENT | HIDDEN_EVENT);
      $o_queue = new supportDaemon();
      if (isset($events['TRIGGERED']['DaemonEvent_installation_activation'])) {
        $evt = $events['TRIGGERED']['DaemonEvent_installation_activation'];
        $tmp = new $evt['CLASS_NAME']($this->config);
        $tmp->set_type(TRIGGERED_EVENT);
        $tmp->add_targets(array($this->netConfigDNS->macAddress));
        $o_queue->append($tmp);
        if ($o_queue->is_error()) {
          msg_dialog::display(_("Service infrastructure"),msgPool::siError($o_queue->get_error()),ERROR_DIALOG);
        }
      }
    }
  }


  /* Display generic part for server copy & paste */
  function getCopyDialog()
  {
    $smarty = get_smarty();
    $smarty->assign("cn" ,$this->cn);
    $smarty->assign("macAddress" ,$this->netConfigDNS->macAddress);
    $smarty->assign("ipHostNumber" ,$this->netConfigDNS->ipHostNumber);
    $smarty->assign("object","workstation");
    $str = $smarty->fetch(get_template_path("paste_generic.tpl",TRUE));
    $ret = array();
    $ret['string'] = $str;
    $ret['status'] = "";
    return $ret;
  }


  function saveCopyDialog()
  {
    if (isset($_POST['cn'])) {
      $this->cn = $_POST['cn'];
    }
    if (isset($_POST['macAddress'])) {
      $this->netConfigDNS->macAddress = $_POST['macAddress'];
    }
    if (isset($_POST['ipHostNumber'])) {
      $this->netConfigDNS->ipHostNumber = $_POST['ipHostNumber'];
    }
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);
    if(isset($source['macAddress'][0])){
      $this->netConfigDNS->macAddress = $source['macAddress'][0];
    }
    if(isset($source['ipHostNumber'][0])){
      $this->netConfigDNS->ipHostNumber = $source['ipHostNumber'][0];
    }

    /* Create used ntp server array */
    $this->gotoNtpServer= array();
    if(isset($source['gotoNtpServer'])){
      $this->inheritTimeServer = false;
      unset($source['gotoNtpServer']['count']);
      foreach($source['gotoNtpServer'] as $server){
        $this->gotoNtpServer[$server] = $server;
      }
    }

    /* Set inherit checkbox state */
    if((in_array("default",$this->gotoNtpServer)) || (count($this->gotoNtpServer)==0)){
      $this->inheritTimeServer = true;
      $this->gotoNtpServer=array();
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
      "plShortName"     => _("Generic"),
      "plDescription"   => _("Workstation generic"),
      "plSelfModify"    => FALSE,
      "plDepends"       => array(),
      "plPriority"      => 0,
      "plSection"       => array("administration"),
      "plCategory"      => array("workstation" => array("description"  => _("Workstation"),
                                                      "objectClass"  => "gotoWorkstation")),
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}
?>
