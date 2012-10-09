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

class NetworkSettingsAttribute extends Attribute
{
  public $netConfigDNS;

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
      $this->netConfigDNS = new termDNS($this->plugin->config, $this->plugin, $this->plugin->objectclasses);
      $this->netConfigDNS->MACisMust = TRUE;
    }
  }

  function renderAttribute(&$attributes)
  {
    $str = $this->netConfigDNS->execute();
    if (is_object($this->netConfigDNS->dialog)) {
      return $str;
    }
    $attributes[$this->getLdapName()] = $str;
  }

  function loadPostValue ()
  {
  }

  function applyPostValue ()
  {
    if (!$this->disabled) {
      /* Refresh base */
      $this->netConfigDNS->save_object();
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
            'active',
            'locked'
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
            TRUE, // TODO CHECK inherited empty
            array('')
          ),
          new BooleanAttribute (
            _('Inherit NTP servers'),
            _('Inherit time server attributes from ogroup'),
            'inheritTimeServer',
            FALSE //TODO managed attributes, disabled if !ogroup, etc…
          ),
          new SetAttribute (
            new SelectAttribute (
              _('NTP servers'),
              _('The NTP servers for the workstation'),
              'gotoNtpServer',
              FALSE,
              array('')
            )
          )
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
    //~ if (class_available("krbHostKeys")) {
      //~ $this->kerberos_key_service = new krbHostKeys($this->config,$this);
    //~ }

    if (!isset($this->parent->by_object['ogroup'])) {
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gotoWorkstationTemplate)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
      $this->member_of_ogroup = ($ldap->count() >= 1);
    }

    //~ $this->netConfigDNS = new termDNS($this->config, $this,$this->objectclasses);
    //~ $this->netConfigDNS->MACisMust = TRUE;

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
    /* Create used ntp server array */
    $ntpServers = $this->attributesInfo['servers']['attrs']['gotoNtpServer']->getValue();
    /* Set inherit checkbox state */
    if ($this->member_of_ogroup && (in_array("default",$ntpServers) || (count($ntpServers) == 0))) {
      $this->attributesInfo['servers']['attrs']['inheritTimeServer']->setValue(TRUE);
      $this->attributesInfo['servers']['attrs']['gotoNtpServer']->setValue(array());
    } else {
      $this->attributesInfo['servers']['attrs']['inheritTimeServer']->setValue(FALSE);
    }
    if (!$this->member_of_ogroup) {
      $this->attributesInfo['servers']['attrs']['inheritTimeServer']->setDisabled(TRUE);
    }

    /* Create available ntp options */
    $tmp = array();
    foreach($this->config->data['SERVERS']['NTP'] as $key => $server){
      if($server == "default") continue;
      $tmp[] = $server;
    }
    $this->attributesInfo['servers']['attrs']['gotoNtpServer']->attribute->setChoices($tmp);

    /* Create an array of all Syslog servers */
    $tmp = $this->config->data['SERVERS']['SYSLOG'];
    $tmp_output = $tmp;
    if(($key = array_search("default", $tmp)) !== FALSE) {
      $tmp[$key] = "";
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
//~
    //~ if($this->is_account && !$this->view_logged){
      //~ $this->view_logged = TRUE;
      //~ new log("view","workstation/".get_class($this),$this->dn);
    //~ }
//~
    //~ /* Do we need to flip is_account state? */
    //~ if(isset($_POST['modify_state'])){
      //~ if($this->is_account && $this->acl_is_removeable()){
        //~ $this->is_account= FALSE;
      //~ }elseif(!$this->is_account && $this->acl_is_createable()){
        //~ $this->is_account= TRUE;
      //~ }
    //~ }
//~
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
//~
    //~ /* Do we represent a valid terminal? */
    //~ if (!$this->is_account && $this->parent === NULL){
      //~ $display= "<img alt=\"\" src=\"images/small-error.png\" align=middle>&nbsp;<b>".
        //~ msgPool::noValidExtension(_("workstation"))."</b>";
      //~ return($display);
    //~ }
//~
    //~ /* Add new ntp Server to our list */
    //~ if((isset($_POST['addNtpServer'])) && (isset($_POST['gotoNtpServers'])) && $this->acl_is_writeable("gotoNtpServer")){
      //~ $this->gotoNtpServer[$_POST['gotoNtpServers']] = $_POST['gotoNtpServers'];
    //~ }
//~
    //~ /* Delete selected NtpServer for list of used servers  */
    //~ if((isset($_POST['delNtpServer'])) && (isset($_POST['gotoNtpServerSelected'])) && $this->acl_is_writeable("gotoNtpServer")){
      //~ foreach($_POST['gotoNtpServerSelected'] as $name){
        //~ unset($this->gotoNtpServer[$name]);
      //~ }
    //~ }
//~
    //~ /* Fill templating stuff */
    //~ $smarty= get_smarty();
    //~ $smarty->assign("usePrototype", "true");
//~
    //~ /* Set acls */
    //~ $tmp = $this->plInfo();
    //~ foreach($tmp['plProvidedAcls'] as $name => $translation){
      //~ $smarty->assign($name."ACL",$this->getacl($name));
    //~ }
//~
    //~ $smarty->assign("cn", $this->cn);
    //~ $smarty->assign("description", $this->description);
    //~ $smarty->assign("l", $this->l);
    //~ $smarty->assign("staticAddress", "");
//~
    //~ $tmp = array();
    //~ foreach($this->gotoNtpServers as $server){
      //~ if(!in_array($server,$this->gotoNtpServer)){
        //~ $tmp[$server] = $server;
      //~ }
    //~ }
    //~ $smarty->assign("gotoNtpServers",$tmp);
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
    //~ $smarty->assign("modes", $this->modes);
    //~ $smarty->assign("nfsservers", $this->config->data['SERVERS']['NFS']);
    //~ $smarty->assign("syslogservers", $this->gotoSyslogServers);
    //~ $smarty->assign("fai_activated",$this->fai_activated);
//~
    //~ $ntpser = array();
    //~ foreach($this->gotoNtpServers as $server){
      //~ if(!in_array($server,$this->gotoNtpServer)){
        //~ $ntpser[$server] = $server;
      //~ }
    //~ }
    //~ $smarty->assign("gotoNtpServers", $ntpser);
//~
    //~ /* Variables */
    //~ foreach(array("gotoMode", "gotoSyslogServer", "gotoNtpServer") as $val){
      //~ $smarty->assign($val."_select", $this->$val);
    //~ }
    //~ $smarty->assign("base", $this->baseSelector->render());
//~
    //~ /* tell smarty the inherit checkbox state */
    //~ $smarty->assign("inheritTimeServer",$this->inheritTimeServer);
    //~ $smarty->assign("member_of_ogroup",$this->member_of_ogroup);
//~
    //~ $str = $this->netConfigDNS->execute();
    //~ if(is_object($this->netConfigDNS->dialog)){
      //~ return($str);
    //~ }
    //~ $smarty->assign("netconfig", $str);
//~
    //~ /* Display kerberos host key options */
    //~ $smarty->assign("host_key","");
    //~ if(is_object($this->kerberos_key_service)){
      //~ $smarty->assign("host_key",$this->kerberos_key_service->execute_by_prefix("host/"));
    //~ }
//~
    //~ /* Show main page */
    //~ $smarty->assign("currently_installing", $this->currently_installing);
    //~ return($smarty->fetch (get_template_path('workstation.tpl', TRUE, dirname(__FILE__))));
  //~ }

  //~ function remove_from_parent()
  //~ {
    //~ if($this->acl_is_removeable()){
//~
      //~ $this->netConfigDNS->remove_from_parent();
      //~ $ldap= $this->config->get_ldap_link();
      //~ $ldap->rmdir($this->dn);
      //~ new log("remove","workstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      //~ if (!$ldap->success()){
        //~ msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
      //~ }
//~
      //~ /* Remove kerberos key dependencies too */
      //~ if(is_object($this->kerberos_key_service)){
        //~ $this->kerberos_key_service->remove_from_parent_by_prefix("host/");
      //~ }
//~
      //~ /* Optionally execute a command after we're done */
      //~ $this->handle_post_events("remove", array("macAddress" => $this->netConfigDNS->macAddress,"ipHostNumber" => $this->netConfigDNS->ipHostNumber));
//~
      //~ /* Delete references to object groups */
      //~ $ldap->cd ($this->config->current['BASE']);
      //~ $ldap->search ("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
      //~ while ($ldap->fetch()){
        //~ $og= new ogroup($this->config, $ldap->getDN());
        //~ unset($og->member[$this->dn]);
        //~ $og->save ();
      //~ }
//~
      //~ /* Remove all accessTo/trust dependencies */
      //~ update_accessTo($this->cn,"");
    //~ }
//~
    //~ /* Clean queue form entries with this mac
     //~ */
    //~ if(class_available("supportDaemon") && tests::is_mac($this->netConfigDNS->orig_macAddress)){
      //~ $q = new supportDaemon();
      //~ $q->clean_queue_from_mac($this->netConfigDNS->orig_macAddress);
    //~ }
//~
    //~ if(isset($_POST["inheritAll"])){
      //~ $this->set_everything_to_inherited();
    //~ }
  //~ }


  //~ /* Save data to object */
  //~ function save_object()
  //~ {
//~
    //~ /* Create a base backup and reset the
       //~ base directly after calling plugin::save_object();
       //~ Base will be set seperatly a few lines below */
    //~ $base_tmp = $this->base;
    //~ plugin::save_object();
    //~ $this->base = $base_tmp;
//~
    //~ /* Refresh base */
    //~ if ($this->acl_is_moveable($this->base)){
      //~ if (!$this->baseSelector->update()) {
        //~ msg_dialog::display(_("Error"), msgPool::permMove(), ERROR_DIALOG);
      //~ }
      //~ if ($this->base != $this->baseSelector->getBase()) {
        //~ $this->base= $this->baseSelector->getBase();
        //~ $this->is_modified= TRUE;
      //~ }
    //~ }
//~
    //~ $this->netConfigDNS->save_object();
//~
    //~ /* Set inherit mode */
    //~ if((isset($_POST['workgeneric_posted'])) && ($this->acl_is_writeable("gotoNtpServer"))){
      //~ if(isset($_POST["inheritTimeServer"]) && $this->member_of_ogroup){
        //~ $this->inheritTimeServer = true;
      //~ }else{
        //~ $this->inheritTimeServer = false;
      //~ }
    //~ }
//~
    //~ if(isset($_POST["inheritAll"])){
      //~ $this->set_everything_to_inherited();
    //~ }
//~
    //~ /* Hanle kerberos host key plugin */
    //~ if(is_object($this->kerberos_key_service)){
      //~ $this->kerberos_key_service->save_object_by_prefix("host/");
    //~ }
  //~ }


  //~ /* Check supplied data */
  //~ function check()
  //~ {
    //~ /* Call common method to give check the hook */
    //~ $message= plugin::check();
//~
    //~ /* Skip IP & Mac checks if this is a template */
    //~ if($this->cn != "wdefault"){
      //~ $message= array_merge($message, $this->netConfigDNS->check());
    //~ }
//~
    //~ $this->dn= "cn=".$this->cn.",".get_ou('workstationRDN').$this->base;
//~
    //~ if ($this->cn == ""){
      //~ $message[]= msgPool::required(_("Name"));
    //~ }
//~
    //~ /* Check if given name is a valid host/dns name */
    //~ if(!tests::is_valid_hostname($this->cn)){
      //~ $message[] = msgPool::invalid(_("Name"));
    //~ }
//~
    //~ // Check if a wrong base was supplied
    //~ if(!$this->baseSelector->checkLastBaseUpdate()){
      //~ $message[]= msgPool::check_base();
    //~ }
//~
    //~ if ($this->orig_dn != $this->dn){
      //~ $ldap= $this->config->get_ldap_link();
      //~ $ldap->cd ($this->base);
//~
      //~ if($this->cn == "wdefault"){
        //~ $ldap->cat($this->dn);
      //~ }else{
        //~ $ldap->search ("(&(cn=".$this->cn.")(objectClass=gotoWorkstation))", array("cn"));
      //~ }
      //~ if ($ldap->count() != 0){
        //~ while ($attrs= $ldap->fetch()){
          //~ if (preg_match("/cn=dhcp,/",$attrs['dn']) || preg_match ("/,".preg_quote(get_ou('systemIncomingRDN'), '/')."/i", $ldap->getDN())){
            //~ continue;
          //~ } else {
            //~ if ($attrs['dn'] != $this->orig_dn){
              //~ $message[]= msgPool::duplicated(_("Name"));
              //~ break;
            //~ }
          //~ }
        //~ }
      //~ }
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
    //~ /* Check if we are allowed to create or move this object
     //~ */
    //~ if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
      //~ $message[] = msgPool::permCreate();
    //~ }elseif($this->orig_dn != "new" && $this->base != $this->orig_base && !$this->acl_is_moveable($this->base)){
      //~ $message[] = msgPool::permMove();
    //~ }
//~
    //~ return ($message);
  //~ }


  /* Save to LDAP */
  //~ function save()
  //~ {
    //~ /* Detect mode changes */
    //~ $activate= (isset($this->saved_attributes['gotoMode']) &&
        //~ $this->gotoMode != $this->saved_attributes['gotoMode'] &&
        //~ $this->gotoMode == "active" &&
        //~ tests::is_ip($this->netConfigDNS->ipHostNumber)) || $this->auto_activate;
    //~ plugin::save();
//~
    //~ /* Strip out 'default' values */
    //~ foreach (array("gotoSyslogServer") as $val){
//~
      //~ if (isset($this->attrs[$val]) && $this->attrs[$val] == "default"){
        //~ $this->attrs[$val]= array();
      //~ }
    //~ }
//~
    //~ /* Remove all empty values */
    //~ if ($this->orig_dn == 'new'){
      //~ $attrs= array();
      //~ foreach ($this->attrs as $key => $val){
        //~ if (is_array($val) && count($val) == 0){
          //~ continue;
        //~ }
        //~ $attrs[$key]= $val;
      //~ }
      //~ $this->attrs= $attrs;
    //~ }
//~
    //~ /* Update ntp server settings */
    //~ if($this->inheritTimeServer){
      //~ if($this->is_new){
        //~ if(isset($this->attrs['gotoNtpServer'])){
          //~ unset($this->attrs['gotoNtpServer']);
        //~ }
      //~ }else{
        //~ $this->attrs['gotoNtpServer'] = array();
      //~ }
    //~ }else{
      //~ /* Set ntpServers */
      //~ $this->attrs['gotoNtpServer'] = array();
      //~ foreach($this->gotoNtpServer as $server){
        //~ $this->attrs['gotoNtpServer'][] = $server;
      //~ }
    //~ }
//~
    //~ /* cn=default and macAddress=- indicates that this is a template */
    //~ if($this->cn == "wdefault"){
      //~ $this->netConfigDNS->macAddress = "-";
    //~ }
//~
    //~ /* Write back to ldap */
    //~ $ldap= $this->config->get_ldap_link();
    //~ if ($this->orig_dn == 'new'){
      //~ if(empty($this->attrs['gotoNtpServer'])) {
        //~ unset($this->attrs['gotoNtpServer']);
      //~ }
      //~ $ldap->cd($this->config->current['BASE']);
      //~ $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      //~ $ldap->cd($this->dn);
      //~ $ldap->add($this->attrs);
      //~ new log("create","workstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      //~ if (!$ldap->success()){
        //~ msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
      //~ }
//~
      //~ $this->netConfigDNS->cn = $this->cn;
      //~ $this->netConfigDNS->save();
//~
      //~ $this->handle_post_events("add",array("macAddress" => $this->netConfigDNS->macAddress,"ipHostNumber" => $this->netConfigDNS->ipHostNumber));
    //~ } else {
      //~ if ($this->orig_dn != $this->dn){
//~
        //~ /* Remove all accessTo/trust dependencies */
        //~ update_accessTo($this->orig_cn,$this->cn);
      //~ }
      //~ $ldap->cd($this->dn);
      //~ $this->cleanup();
      //~ $ldap->modify ($this->attrs);
      //~ if (!$ldap->success()){
        //~ msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
      //~ }
      //~ new log("modify","workstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
//~
      //~ $this->netConfigDNS->cn = $this->cn;
      //~ $this->netConfigDNS->save();
//~
      //~ $this->handle_post_events("modify",array("macAddress" => $this->netConfigDNS->macAddress,"ipHostNumber" => $this->netConfigDNS->ipHostNumber));
    //~ }
//~
    //~ if ($activate && class_available("DaemonEvent")){
//~
      //~ /* Send installation activation
       //~ */
      //~ $events = DaemonEvent::get_event_types(SYSTEM_EVENT | HIDDEN_EVENT);
      //~ $o_queue = new supportDaemon();
      //~ if(isset($events['TRIGGERED']['DaemonEvent_installation_activation'])){
        //~ $evt = $events['TRIGGERED']['DaemonEvent_installation_activation'];
        //~ $tmp = new $evt['CLASS_NAME']($this->config);
        //~ $tmp->set_type(TRIGGERED_EVENT);
        //~ $tmp->add_targets(array($this->netConfigDNS->macAddress));
        //~ if(!$o_queue->append($tmp)){
          //~ msg_dialog::display(_("Service infrastructure"),msgPool::siError($o_queue->get_error()),ERROR_DIALOG);
        //~ }
      //~ }
    //~ }
  //~ }


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


  //~ function set_everything_to_inherited()
  //~ {
    //~ $this->gotoSyslogServer  = "default";
    //~ $this->inheritTimeServer = TRUE;
//~
    //~ /* Set workstation service attributes to inherited */
    //~ if($this->member_of_ogroup && isset($this->parent->by_object['workservice'])){
      //~ foreach(array("gotoXKbLayout","gotoXKbModel","gotoXKbVariant","gotoXDriver",
            //~ "gotoXResolution","gotoXColordepth","gotoXMouseType","gotoXMouseport") as $name){
        //~ if($this->parent->by_object['workservice']->acl_is_writeable($name)){
          //~ $this->parent->by_object['workservice']->$name = "default";
        //~ }
      //~ }
    //~ }
//~
    //~ /* Set workstation startup attributes to inherited */
    //~ if($this->member_of_ogroup && isset($this->parent->by_object['workstartup'])){
      //~ $obj = $this->parent->by_object['workstartup'];
      //~ if($obj->acl_is_writeable("gotoBootKernel")){
        //~ $this->parent->by_object['workstartup']->gotoBootKernel = "default-inherited";
      //~ }
      //~ if($obj->acl_is_writeable("gotoLdapServer")){
        //~ $this->parent->by_object['workstartup']->gotoLdapServer = "default-inherited";
        //~ $this->parent->by_object['workstartup']->gotoLdap_inherit = TRUE;
        //~ $this->parent->by_object['workstartup']->gotoLdapServers = array();
      //~ }
      //~ if($obj->acl_is_writeable("FAIdebianMirror")){
        //~ $this->parent->by_object['workstartup']->FAIdebianMirror= "inherited";
      //~ }
    //~ }
  //~ }


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
