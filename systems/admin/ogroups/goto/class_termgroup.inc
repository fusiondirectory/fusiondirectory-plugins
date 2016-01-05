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

class termgroup extends plugin
{
  /* attribute list for save action */
  var $members= array();

  /* Stores a list of the member dn's */
  var $member_dn = array();

  var $gotoMode     = "locked";
  var $gotoSyslogServer   = "";
  var $gotoNtpServer    = array();
  var $gotoTerminalPath   = array();
  var $gotoSwapServer   = array();
  var $modes      = array();
  var $inheritTimeServer  = true;
  var $is_account   = true;
  var $orig_dn      = "";
  var $didAction    = FALSE;
  var $mapActions   = array("halt"            => "halt",
                                "reboot"          => "reboot",
                                "update"          => "update",
                                "reinstall"       => "reinstall",
                                "wake"            => "wakeup",
         # These are currently not supported by the tftp daemon
                           # "memcheck"        => "memcheck",
                           # "sysinfo"         => "sysinfo"
         );

  var $attributes     = array("gotoMode","gotoSyslogServer", "gotoNtpServer", "gotoTerminalPath", "gotoSwapServer");
  var $objectclasses  = array("gotoWorkstationTemplate");
  var $CopyPasteVars  = array("gotoNtpServers","modes","inheritTimeServer","members");
  var $view_logged    = FALSE;
  var $nfsservers     = array();
  var $swapservers    = array();
  var $member_of_ogroup= false;
  var $members_inherit_from_group = false;

  function termgroup (&$config, $dn= NULL, $parent= NULL)
  {
    /***************
      Some  initialisations
     ***************/

    parent::__construct($config, $dn, $parent);
    $ldap= $config->get_ldap_link();

    $this->is_account = true;
    $this->modes["active"]= _("Activated");
    $this->modes["locked"]= _("Locked");
//    $this->modes["memcheck"]= _("Memory test");
//    $this->modes["sysinfo"]= _("System analysis");

    $this->orig_dn =  $this->dn;

    /***************
      Get mac addresses from member objects
     ***************/

    /* We're only interested in the terminal members here, evaluate
       these... */
    if(isset($this->attrs['member'])){
      for ($i= 0; $i<$this->attrs['member']['count']; $i++){
        $member= $this->attrs['member'][$i];
        array_push($this->member_dn, $member);
        $ldap->cat($member, array('objectClass', 'macAddress', 'cn'));
        if ($ldap->success()){
          $attrs = $ldap->fetch();
          if (in_array("gotoTerminal", $attrs['objectClass']) ||
              in_array("gotoWorkstation", $attrs['objectClass'])){
            if (isset($attrs['macAddress'])){
              $this->members[$attrs['cn'][0]]= $attrs['macAddress'][0];
            } else {
              $this->members[$attrs['cn'][0]]= "";
            }
          }
        }
      }
    }

    /***************
      Perpare NTP settings
     ***************/

    /* Create used ntp server array */
    $this->gotoNtpServer= array();
    if(isset($this->attrs['gotoNtpServer'])){
      $this->inheritTimeServer = false;
      unset($this->attrs['gotoNtpServer']['count']);
      foreach($this->attrs['gotoNtpServer'] as $server){
        $this->gotoNtpServer[$server] = $server;
      }
    }

    /* Get Share servers */
    $tmp2 = array();
    $tmp2['!']= _("Local swap");
    foreach($this->config->data['SERVERS']['NBD'] as $server){
      if($server != "default"){
        $tmp2[$server]= $server;
      }else{
        if($this->member_of_ogroup){
          $tmp2[$server]="["._("inherited")."]";
        }
      }
    }
    $this->swapservers= $tmp2;

    $tmp2 = array();
    foreach($this->config->data['SERVERS']['NFS'] as $server){
      if($server != "default"){
        $tmp2[$server]= $server;
      }else{
        if($this->member_of_ogroup){
          $tmp2[$server]="["._("inherited")."]";
        }
      }
    }
    $this->nfsservers= $tmp2;

    /* Set inherit checkbox state */
    if(in_array("default",$this->gotoNtpServer)){
      $this->inheritTimeServer = true;
      $this->gotoNtpServer=array();
    }

    /* Create available ntp options */
    $this->gotoNtpServers = $this->config->data['SERVERS']['NTP'];
    foreach($this->gotoNtpServers as $key => $server){
      if($server == "default"){
        unset($this->gotoNtpServers[$key]);
      }
    }
  }

  function check()
  {
     /* Call common method to give check the hook */
    $message= plugin::check();

    if (!$this->acl_is_createable() && $this->dn == "new"){
      $message[]= msgPool::permCreate();
    }
  }

  function remove_from_parent()
  {
    /* Workstation startup is using gotoWorkstationTemplate too,
        if we remove this oc all other not manged attributes will cause errors */
    if(isset($this->attrs['gotoKernelParameters'])){
      $this->objectclasses = array();
    }

    /* Remove acc */
    parent::remove_from_parent();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->orig_dn);
    $ldap->modify($this->attrs);
    $this->handle_post_events("remove");
    new log("remove","ogroup/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
  }



  function execute()
  {
    /* Call parent execute */
    parent::execute();

    if(!$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","ogroup/".get_class($this),$this->dn);
    }


    /***************
      Handle requested action
     ***************/

    /* Watch for events */
    if ((isset($_POST['action'])) && ($this->acl_is_writeable("FAIstate")) && isset($this->mapActions[$_POST['saction']])) {

      /* Check if we have an DaemonEvent for this action */
      $action = $this->mapActions[$_POST['saction']];
      if (class_available("DaemonEvent")) {
        $events = DaemonEvent::get_event_types(SYSTEM_EVENT | HIDDEN_EVENT);
        $macaddresses = array();
        foreach ($this->members as $cn => $macAddress){
          $macaddresses[]= $macAddress;
        }

        if (isset($events[$action])) {
          $tmp = new DaemonEvent($this->config, $action);
          $tmp->add_targets($macaddresses);
          $tmp->set_type(TRIGGERED_EVENT);
          $o_queue = new gosaSupportDaemon();
          $o_queue->append($tmp);
          if ($o_queue->is_error()) {
            msg_dialog::display(_("Infrastructure service"),msgPool::siError($o_queue->get_error()),ERROR_DIALOG);
          }
        }
      } else {
        msg_dialog::display(_("Event error"),
            sprintf(_("Event '%s' is not available!"),$action),ERROR_DIALOG);
      }
    }


    /***************
      Add remove NTP server
     ***************/

    /* Add new ntp Server to our list */
    if((isset($_POST['addNtpServer'])) && (isset($_POST['gotoNtpServers']))){
      $this->gotoNtpServer[$_POST['gotoNtpServers']] = $_POST['gotoNtpServers'];
    }

    /* Delete selected NtpServer for list of used servers  */
    if((isset($_POST['delNtpServer'])) && (isset($_POST['gotoNtpServerSelected']))){
      foreach($_POST['gotoNtpServerSelected'] as $name){
        unset($this->gotoNtpServer[$name]);
      }
    }


    /***************
      Prepare smarty
     ***************/

    /* Set government mode */
    $smarty= get_smarty();

    if (isset($this->parent->by_name['terminalStartup'])){
      $smarty->assign("is_termgroup", "1");
    } else {
      $smarty->assign("is_termgroup", "0");
    }

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translated) {
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr,      $this->$attr);
    }

    /* Variables */
    foreach(array("gotoMode","gotoNtpServer") as $val){
      $smarty->assign($val."_select", $this->$val);
    }

    $smarty->assign("actions", array("halt" => _("Switch off"), "reboot" => _("Reboot"),
          "instant_update" => _("Instant update"),
          "update" => _("Scheduled update"),
          "wake" => _("Wake"),
          "reinstall" => _("Reinstall"),
          "memcheck" => _("Memory test"),
          "sysinfo"  => _("System analysis")));

    $smarty->assign("inheritTimeServer",$this->inheritTimeServer);
    $smarty->assign("modes", $this->modes);

    $tmp = array();
    foreach($this->gotoNtpServers as $server){
      if(!in_array($server,$this->gotoNtpServer)){
        $tmp[$server] = $server;
      }
    }

    $smarty->assign("gotoNtpServers",$tmp);

    $smarty->assign("nfsservers",     $this->nfsservers);
    $smarty->assign("swapservers",    $this->swapservers);

    $smarty->assign("syslogservers", $this->config->data['SERVERS']['SYSLOG']);
    $smarty->assign("gotoSyslogServer_select", $this->gotoSyslogServer);

    /* Variables */
    foreach(array("gotoTerminalPath", "gotoSwapServer") as $val){
      $smarty->assign($val."_select", $this->$val);
    }


    /* Show main page */
    $smarty->assign("members_inherit_from_group", $this->members_inherit_from_group);
    return ($smarty->fetch (get_template_path('termgroup.tpl', TRUE, dirname(__FILE__))));
  }


  function save_object()
  {
    parent::save_object();
    /* Set inherit mode */
    if(isset($_POST['workgeneric_posted'])){

      /* Handle the inherit to members button */
      $this->members_inherit_from_group = isset($_POST['members_inherit_from_group']);

      if(isset($_POST["inheritTimeServer"])){
        $this->inheritTimeServer = true;
      }else{
        $this->inheritTimeServer = false;
      }
    }
  }

  /* Save to LDAP */
  function save()
  {
    if (isset($this->parent->by_name['terminalStartup'])){
      $this->objectclasses= array("gotoTerminalTemplate");
    } else {
      $this->objectclasses= array("gotoWorkstationTemplate");
    }

    parent::save();

    /* Tell members to inherit all attributes if the flag is set  */
    //~ $this->inherit_attributes_to_members();

    /***************
      Prepare special vars
     ***************/

    /* Unset some special vars ... */
    foreach (array("gotoSyslogServer") as $val){
      if ($this->attrs[$val] == "default"){
        $this->attrs[$val]= array();
      }
    }

    /* Update ntp server settings */
    if($this->inheritTimeServer){
      $this->attrs['gotoNtpServer'] = "default";
    }else{
      /* Set ntpServers */
      $this->attrs['gotoNtpServer'] = array();
      foreach($this->gotoNtpServer as $server){
        $this->attrs['gotoNtpServer'][] = $server;
      }
    }


    /***************
      Write to ldap
     ***************/

    /* Write back to ldap */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs);

    if($this->initially_was_account){
      new log("modify","ogroup/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("create","ogroup/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if(!$this->didAction){
      $this->handle_post_events("modify");
    }
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()), LDAP_ERROR);
    }
  }


  static function plInfo()
  {
    return array(
      'plShortName'   => _('System'),
      'plDescription' => _('System group'),
      'plSelfModify'  => FALSE,
      'plPriority'    => 5,
      'plObjectType'  => array('ogroup-workstation', 'ogroup-terminal'),

      'plProvidedAcls'  => array(
        'gotoMode'          => _('Mode'),
        'gotoSyslogServer'  => _('Syslog server'),
        'FAIstate'          => _('Action flag'),
        'gotoNtpServer'     => _('Ntp server'),
        'gotoTerminalPath'  => _('Root server'),
        'gotoSwapServer'    => _('Swap server')
      )
    );
  }

  //~ function inherit_attributes_to_members()
  //~ {
    //~ if ($this->members_inherit_from_group) {
      //~ foreach ($this->member_dn as $dn) {
        //~ $member_obj = new worktabs($this->config, $this->config->data['TABS']['WORKSTATIONTABS'], $dn, "workstation");
        //~ $member_obj->getBaseObject()->set_everything_to_inherited();
        //~ $member_obj->save();
      //~ }
    //~ }
  //~ }

  function PrepareForCopyPaste($source)
  {
    /* Create used ntp server array */
    $this->gotoNtpServer= array();

    $source_o = new termgroup ($this->config, $source['dn']);
    foreach(array("gotoTerminalPath","gotoSwapServer","gotoSyslogServer","gotoMode") as $attr){
      $this->$attr = $source_o->$attr;
    }

    if(isset($source['gotoNtpServer'])){
      $this->inheritTimeServer = false;
      unset($source['gotoNtpServer']['count']);
      foreach($source['gotoNtpServer'] as $server){
        $this->gotoNtpServer[$server] = $server;
      }
    }

    /* Set inherit checkbox state */
    if(in_array("default",$this->gotoNtpServer)){
      $this->inheritTimeServer = true;
      $this->gotoNtpServer=array();
    }

    /* Create available ntp options */
    $this->gotoNtpServers = $this->config->data['SERVERS']['NTP'];
    foreach($this->gotoNtpServers as $key => $server){
      if($server == "default"){
        unset($this->gotoNtpServers[$key]);
      }
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
