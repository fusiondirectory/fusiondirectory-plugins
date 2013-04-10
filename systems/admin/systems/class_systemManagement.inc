<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class systemManagement extends management
{
  // Tab definition
  protected $tabClass         = "";
  protected $tabType          = "";
  protected $aclCategory      = "";
  protected $aclPlugin        = "";
  protected $objectName       = "system";
  protected $objectInfo       = array();
  protected $activationQueue  = array();

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    // Set storage points
    $tD = $this->getObjectDefinitions();
    $sP = array();
    foreach($tD as $entry){
      if(!empty($entry['ou']))
        $sP[] = $entry['ou'];
    }
    $this->storagePoints = array_unique($sP);

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")){
     $filter= session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("system-filter.xml", true));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("system-list.xml", true));
    $headpage->registerElementFilter("systemRelease", "systemManagement::systemRelease");
    $headpage->registerElementFilter("listServices", "systemManagement::listServices");
    $headpage->setFilter($filter);

    // Register Daemon Events
    if (class_available("DaemonEvent") && class_available("supportDaemon")) {
      $events = DaemonEvent::get_event_types();
      foreach($events as $name => $data){
        $this->registerAction("T_".$name, "handleEvent");
        $this->registerAction("S_".$name, "handleEvent");
      }
      $this->registerAction("activateMultiple", "activateMultiple");
    }
    $this->registerAction("saveEvent","saveEventDialog");
    $this->registerAction("createISO","createISO");
    $this->registerAction("initiateISOcreation","initiateISOcreation");
    $this->registerAction("performIsoCreation","performIsoCreation");
    $this->registerAction("systemTypeChosen","systemTypeChosen");
    $this->registerAction("handleActivationQueue","handleActivationQueue");

    $this->registerAction("new_goServer",         "newEntry");
    $this->registerAction("new_gotoWorkstation",  "newEntry");
    $this->registerAction("new_gotoTerminal",     "newEntry");
    $this->registerAction("new_sambaSamAccount",  "newEntry");
    $this->registerAction("new_gotoPrinter",      "newEntry");
    $this->registerAction("new_goFonHardware",    "newEntry");
    $this->registerAction("new_ieee802Device",    "newEntry");
    $this->registerAction("new_fdMobilePhone",    "newEntry");

    foreach ($this->config->data['TABS']['SERVERSERVICE'] as $plug) {
      if (class_available($plug['CLASS'])) {
        $this->registerAction("edit_".$plug['CLASS'], "editEntry");
      }
    }

    // Check if we are able to communicate with Argonaut server
    if (class_available("supportDaemon")) {
      $o = new supportDaemon();
      $this->si_active = $o->is_available() && class_available("DaemonEvent");
    }

    parent::__construct($config, $ui, "systems", $headpage);
  }


  /*! \brief  This method is used to queue and process copy&paste actions.
   *          Allows to copy, cut and paste mutliple entries at once.
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function copyPasteHandler($action = "", $target = array(), $all = array(),
      $altTabClass = "", $altTabType = "", $altAclCategory = "", $altAclPlugin = "")
  {
    $tDefs    = $this->getObjectDefinitions();

    $tabClass     = array();
    $tabType      = array();
    $aclCategory  = array();
    $aclPlugin    = array();

    $headpage = $this->getHeadpage();

    foreach ($target as $dn) {
      $type   = $headpage->getType($dn);
      $entry  = $headpage->getEntry($dn);

      $tabClass[$dn]    = $tDefs[$type]['tabClass'];
      $tabType[$dn]     = $tDefs[$type]['tabDesc'];
      $aclCategory[$dn] = $tDefs[$type]['aclCategory'];
      $aclPlugin[$dn]   = $tDefs[$type]['aclClass'];
    }

    return parent::copyPasteHandler($action, $target, $all, $tabClass, $tabType, $aclCategory, $aclPlugin);
  }


  /*! \brief  The method gets called when somebody clicked the CD icon
   *           in the system listing.
   *          A confirmation will be shown to acknowledge the creation.
   */
  function createISO($action,$target)
  {
    if(count($target) == 1){
      $smarty = get_smarty();
      $this->dn= array_pop($target);
      set_object_info($this->dn);
      return ($smarty->fetch(get_template_path('gencd.tpl', TRUE)));

    }
  }


  /*! \brief  Once the user has confirmed the ISO creation in 'createISO',
   *           this method gets called.
   *          An iFrame is shown which then used 'performIsoCreation' as contents.
   */
  function initiateISOcreation()
  {
    $smarty = get_smarty();
    $smarty->assign("src", "?plug=".$_GET['plug']."&amp;PerformIsoCreation");
    return ($smarty->fetch(get_template_path('gencd_frame.tpl', TRUE)));
  }


  /*! \brief  ISO creation confirmed and iFrame is visible, now create the ISO
   *           and display the status to fill the iFrame.
   */
  function performIsoCreation()
  {
    $return_button   = "<form method='get' action='main.php' target='_parent'>
      <input type='submit' value='"._("Back")."'>
      <input type='hidden' name='plug' value='".$_GET['plug']."'/>
      </form>";

    $dsc   = array(0 => array("pipe", "r"), 1 => array("pipe", "w"), 2 => array("pipe", "w"));

    /* Get and check command */
    $command= $this->config->search("workstationGeneric", "SYSTEMISOHOOK", array('tabs'));
    if (check_command($command)) {
      @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execute");

      /* Print out html introduction */
      echo '  <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN">
        <html>
        <head>
        <title></title>
        <style type="text/css">@import url("themes/default/style.css");</style>
        <script language="javascript" src="include/focus.js" type="text/javascript"></script>
        </head>
        <body style="background: none; margin:4px;" id="body" >
        <pre>';

      /* Open process handle and check if it is a valid process */
      $process= proc_open($command." '".$this->dn."'", $dsc, $pipes);
      if (is_resource($process)) {
        fclose($pipes[0]);

        /* Print out returned lines && write JS to scroll down each line */
        while (!feof($pipes[1])){
          $cur_dat = fgets($pipes[1], 1024);
          echo $cur_dat;
          echo '<script language="javascript" type="text/javascript">scrollDown2();</script>' ;
          flush();
        }
      }

      /* Get error string && close streams */
      $buffer= stream_get_contents($pipes[2]);

      fclose($pipes[1]);
      fclose($pipes[2]);
      echo "</pre>";

      /* Check return code */
      $ret= proc_close($process);
      if ($ret != 0){
        echo "<h1 style='color:red'>"._("Creating the image failed. Please see the report below.")."</h1>";
        echo "<pre style='color:red'>$buffer</pre>";
      }
      echo $return_button."<br>";
    } else {
      $tmp= "<h1 style='color:red'>".sprintf(_("Command '%s', specified for ISO creation doesn't seem to exist."), $command)."</h1>";
      echo $tmp;
      echo $return_button."<br>";
    }

    /* Scroll down completly */
    echo '<script language="javascript" type="text/javascript">scrollDown2();</script>' ;
    echo '</body></html>';
    flush();
    exit;
  }


  /*! \brief    Handle Argonaut events
   *            All schedules and triggered events are handled here.
   */
  function handleEvent($action="",$target=array(),$all=array())
  {
    // Detect whether this event is scheduled or triggered.
    $triggered = TRUE;
    if (preg_match("/^S_/", $action)) {
      $triggered = FALSE;
    }

    // Detect triggere or scheduled actions
    $headpage = $this->getHeadpage();
    $event = preg_replace("/^[TS]_/", "", $action);
    if (preg_match("/^[TS]_/", $action)) {

    }

    // Now send remaining FAI/Argonaut events here.
    if (count($target) && $this->si_active) {
      $mac = array();

      // Collect target mac addresses
      $ldap     = $this->config->get_ldap_link();
      $tD       = $this->getObjectDefinitions();
      $events   = DaemonEvent::get_event_types();
      $o_queue  = new supportDaemon();
      foreach ($target as $dn) {
        $type = $headpage->getType($dn);
        if ($tD[$type]['sendEvents']) {
          $obj = $headpage->getEntry($dn);
          if (isset($obj['macAddress'][0])) {
            $mac[] = $obj['macAddress'][0];
          }
        }
      }

      /* Skip installation or update trigerred events,
       *  if this entry is currently processing.
       */
      if ($triggered && in_array($event, array("reinstall","update"))) {
        foreach ($mac as $key => $mac_address) {
          if ($o_queue->is_currently_installing($mac_address)) {
            unset($mac[$key]);
            new log("security", "systems/".get_class($this), "", array(), "Skip adding 'DaemonEvent::".$type."' for mac '".$mac_address."', there is already a job in progress.");
          }
        }
      }

      // Prepare event to be added
      if (count($mac) && isset($events[$event]) && $this->si_active) {
        $this->dialogObject = new DaemonEvent($this->config, $event);
        $this->dialogObject->add_targets($mac);

        if ($triggered) {
          $this->dialogObject->set_type(TRIGGERED_EVENT);
          $o_queue->append($this->dialogObject);
          if ($o_queue->is_error()) {
            msg_dialog::display(_("Service infrastructure"), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
          } else {
            $this->closeDialogs();
          }
        } else {
          $this->dialogObject->set_type(SCHEDULED_EVENT);
        }
      }
    }
  }


  /*! \brief  Close all dialogs and reset the activationQueue.
   */
  function cancelEdit()
  {
    management::cancelEdit();
    $this->activationQueue = array();
  }

  /*! \brief  Save event dialogs.
   *          And append the new Argonaut event.
   */
  function saveEventDialog()
  {
      if ($this->si_active) {
          $o_queue = new supportDaemon();
          $o_queue->append($this->dialogObject);
          if ($o_queue->is_error()) {
              msg_dialog::display(_("Service infrastructure"), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
          } else {
              $this->closeDialogs();
          }
      }
  }

  function saveSnapshot($action="",$target=array(),$all=array())
  {
    if (!is_object($this->dialogObject)) return;
    $this->dialogObject->save_object();
    $msgs = $this->dialogObject->check();
    if (count($msgs)) {
      foreach ($msgs as $msg) {
        msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
      }
    } else {
      $this->dn =  $this->dialogObject->dn;
      $dns      = array($this->dn);
      /* Check if other dns needs to be included in this snapshot */
      $entry = $this->getHeadpage()->getEntry($this->dn);
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      /* DHCP entry */
      $ldap->search ("(&(objectClass=dhcpHost)(cn=".$entry['cn'][0]."))", array("uid","cn","dn"));

      if ($attr = $ldap->fetch()) {
        $dns[] = $attr['dn'];
      }
      /* DNS entry */
      if (class_available('DNS')) {
        $dnsEntry = DNS::getDNSHostEntries($this->config, $entry['cn'][0]);
        if ($dnsEntry['exists']) {
          $dns[]  = "relativeDomainName=".$entry['cn'][0].",".DNS::getDNSZoneDN($this->config, $dnsEntry['zoneName']);
        }
      }
      $this->snapHandler->create_snapshot($dns, $this->dialogObject->CurrentDescription);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "Snaptshot created!");
      $this->closeDialogs();
    }
  }


  /*! \brief    Queue selected objects to be removed.
   *            Checks ACLs, Locks and ask for confirmation.
   */
  protected function removeEntryRequested($action="",$target=array(),$all=array())
  {
    $disallowed = array();
    $this->dns = array();

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$target,"Entry removel requested!");

    // Check permissons for each target
    $tInfo = $this->getObjectDefinitions();
    $headpage = $this->getHeadpage();
    foreach($target as $dn){
      $type = $headpage->getType($dn);
      if(!isset($tInfo[$type])){
        trigger_error("Unknown object type received '".$type."' please update systemManagement::getObjectDefinitions()!");
      }else{
        $info = $tInfo[$type];
        $acl = $this->ui->get_permissions($dn, $info['aclCategory']."/".$info['aclClass']);
        if(preg_match("/d/",$acl)){
          $this->dns[] = $dn;
        }else{
          $disallowed[] = $dn;
        }
      }
    }
    if(count($disallowed)){
      msg_dialog::display(_("Permission"),msgPool::permDelete($disallowed),INFO_DIALOG);
    }

    // We've at least one entry to delete.
    if(count($this->dns)){

      // check locks
      if ($user= get_multiple_locks($this->dns)){
        return(gen_locked_message($user,$this->dns));
      }

      // Add locks
      $dns_names = array();
      foreach($this->dns as $dn){
        $dns_names[] =LDAP::fix($dn);
      }
      add_lock ($this->dns, $this->ui->dn);

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign("info", msgPool::deleteInfo($dns_names,_($this->objectName)));
      $smarty->assign("multiple", true);
      return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
    }
  }


  /*! \brief  Object removal was confirmed, now remove the requested entries.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function removeEntryConfirmed($action="",$target=array(),$all=array(),
      $altTabClass="",$altTabType="",$altAclCategory="")
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$target,"Entry removel confirmed!");

    // Check permissons for each target
    $tInfo = $this->getObjectDefinitions();
    $headpage = $this->getHeadpage();
    $disallowed = array();
    foreach($this->dns as $key => $dn){
      $type = $headpage->getType($dn);
      if(!isset($tInfo[$type])){
        trigger_error("Unknown object type received '".$type."' please update systemManagement::getObjectDefinitions()!");
      }else{

        $info = $tInfo[$type];
        $acl = $this->ui->get_permissions($dn, $info['aclCategory']."/".$info['aclClass']);
        if(preg_match("/d/",$acl)){

          // Delete the object
          $this->dn = $dn;
          if($info['tabClass'] == "phonetabs"){
             $this->tabObject= new $info['tabClass']($this->config, $this->config->data['TABS'][$info['tabDesc']], $dn,$type);
             $this->tabObject->set_acl_base($dn);
             $this->tabObject->by_object['phoneGeneric']->remove_from_parent ();
          }else{
            $this->tabObject= new $info['tabClass']($this->config,$this->config->data['TABS'][$info['tabDesc']],
                $this->dn, $info['aclCategory']);
            $this->tabObject->set_acl_base($this->dn);
            $this->tabObject->parent = &$this;
            $this->tabObject->delete ();
          }

          // Remove the lock for the current object.
          del_lock($this->dn);

        }else{
          $disallowed[] = $dn;
          new log("security","system/".get_class($this),$dn,array(),"Tried to trick deletion.");
        }
      }
    }
    if(count($disallowed)){
      msg_dialog::display(_("Permission"),msgPool::permDelete($disallowed),INFO_DIALOG);
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
  }


  /*! \brief  Edit the selected system type. */
  function editEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    if (count($target) == 1) {
      $tInfo    = $this->getObjectDefinitions();
      $headpage = $this->getHeadpage();
      $dn       = $target[0];
      $type     = $headpage->getType($dn);
      $tData    = $tInfo[$type];

      $str = management::editEntry($action, $target, $all, $tData['tabClass'], $tData['tabDesc'], $tData['aclCategory']);
      if ($str) {
        return $str;
      }
      if (preg_match("/^edit_/", $action)) {
        $service = preg_replace("/^edit_/", "", $action);
        if (isset($this->tabObject->by_object['ServerService'])) {
          $this->tabObject->current = 'ServerService';
          $all = array('action'  => 'edit', 'targets' => array($service));
          $this->tabObject->by_object['ServerService']->editEntry($all['action'], $all['targets'], $all);
        } else {
          trigger_error("Unknown tab: ServerService");
        }
      }
    }
  }


  /*! \brief  Edit the selected system type.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function newEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    $tInfo = $this->getObjectDefinitions();
    $info = preg_replace("/^new_/","",$action);
    if(!isset($tInfo[$info])){
      trigger_error("Unknown action type '".$action."' cant create a new system!");
    }else{
      return(management::newEntry($action,$target,$all,
          $tInfo[$info]['tabClass'],
          $tInfo[$info]['tabDesc'],
          $tInfo[$info]['aclCategory']));
    }
  }


  /*! \brief  Activates all selcted 'NewDevices' at once.
   *          Enqueues the selected Devices in the activation queue.
   */
  function activateMultiple($action,$target)
  {
    $headpage = $this->getHeadpage();
    foreach($target as $dn) {
      if($headpage->getType($dn) == "FAKE_OC_NewDevice"){
        $this->activationQueue[$dn] = array();
      }
    }
    if(count($this->activationQueue)){
      $this->dialogObject = new SelectDeviceType($this->config, array_keys($this->activationQueue));
      $this->skipFooter = TRUE;
    }
  }


  /*! \brief  The system selection dialog was closed.
   *          We will now queue the given entry to be activated.
   */
  function systemTypeChosen()
  {
    // Detect the systems target type
    $tInfo = $this->getObjectDefinitions();
    $selected_group = "none";
    if(isset($_POST['ObjectGroup'])){
      $selected_group = $_POST['ObjectGroup'];
    }
    $selected_system = $_POST['SystemType'];
    $tmp = array();
    foreach($this->activationQueue as $dn => $data){
      $tmp[$dn]['OG'] = $selected_group;
      $tmp[$dn]['SS'] = $selected_system;
    }
    $this->closeDialogs();
    $this->activationQueue = $tmp;
    return($this->handleActivationQueue());
  }


  /*! \brief  Activate queued goto systems.
   */
  function handleActivationQueue()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
        'Entries left: '.count($this->activationQueue), "<b>Handling system activation queue!</b>");

    if(!count($this->activationQueue)) return("");

    $ldap     = $this->config->get_ldap_link();
    $pInfo    = $this->getObjectDefinitions();
    $ui       = get_userinfo();
    $headpage = $this->getHeadpage();
    $ldap->cd($this->config->current['BASE']);

    // Walk through systems to activate
    while(count($this->activationQueue)){

      // Get next entry
      reset($this->activationQueue);
      $dn = key($this->activationQueue);
      $data= $this->activationQueue[$dn];

      // Validate the given system type.
      if(!isset($data['SS'])) continue;
      $sysType = $data['SS'];
      if(!isset($pInfo[$sysType])){
        trigger_error('Unknown type \''.$sysType.'\'!');
        continue;
      }
      $type = $pInfo[$sysType];

      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
          $dn, "<b>Try to activate:</b>");

      // Get target type definition
      $plugClass    = $type["plugClass"];
      $tabClass     = $type["tabClass"];
      $aclCategory  = $type["aclCategory"];
      $tabDesc      = $type["tabDesc"];

      if(!class_available($tabClass)){
        msg_dialog::display(_("Error"), msgPool::class_not_found($tabclass), ERROR_DIALOG);
        unset($this->activationQueue[$dn]);
        continue;
      }else{

        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
            $sysType, "<b>System type:</b>");

        // Load permissions for selected 'dn' and check if we're allowed to create this 'dn'
        $this->dn = $dn;
        $acls   = $ui->get_permissions($this->dn,$aclCategory."/".$plugClass);

        // Check permissions
        if(!preg_match("/c/",$acls)){
          unset($this->activationQueue[$dn]);
          msg_dialog::display(_("Error"), msgPool::permCreate(), ERROR_DIALOG);

          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
              $acls, "<b>Insufficient permissions!</b>");
          continue;
        }else{

          // Open object an preset some values like the objects base
          del_lock($dn);
          management::editEntry('editEntry',array($dn),array(),$tabClass,$tabDesc, $aclCategory);
          $this->displayApplyBtn = FALSE;
          $this->tabObject->set_acl_base($headpage->getBase());

          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
              $data['OG'], "<b>Selected ogroup:</b>");

          if($data['OG'] != "none"){
            $this->tabObject->base = preg_replace("/^[^,]+,".preg_quote(get_ou('ogroupRDN'), '/')."/i", "", $data['OG']);
            $this->tabObject->by_object[$plugClass]->baseSelector->setBase($this->tabObject->base);
          } else {
            $this->tabObject->by_object[$plugClass]->baseSelector->setBase($headpage->getBase());
            $this->tabObject->base = $headpage->getBase();
          }

          // Queue entry to be activated, when it is saved.
          if($data['OG'] != "none"){

            // Set gotoMode to active if there was an ogroup selected.
            $found = false;
            foreach(array("workstationGeneric"=>"active","serverGeneric"=>"active","terminalGeneric"=>"active") as $tab => $value){
              if(isset($this->tabObject->by_object[$tab]->gotoMode)) {
                $found = true;
                $this->tabObject->by_object[$tab]->gotoMode = $value;
                @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
                    $tab."->gotoMode = {$value}", "<b>Setting gotoMode to: </b>");
              }
            }
            if(!$found){
              msg_dialog::display(_("Internal error"), _("Cannot set mode to 'active'!"), ERROR_DIALOG);
            }

            // Update object group membership
            $og = new ogroup($this->config,$data['OG']);
            if($og){
              $og->AddDelMembership($this->tabObject->dn);
              $og->save();
              @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
                  $og->dn, "<b>Adding system to ogroup</b>");
            }

            // Set default system specific attributes
            foreach (array("workstationGeneric", "terminalGeneric") as $cls){
              if (isset($this->tabObject->by_object[$cls])){
                $this->tabObject->by_object[$cls]->set_everything_to_inherited();
                @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
                    $og->dn, "<b>Calling {$cls}->set_everything_to_inherited()</b>");
              }
            }

            // Enable activation
            foreach (array("serverGeneric", "workstationGeneric", "terminalGeneric") as $cls){
              if (isset($this->tabObject->by_object[$cls])){
                $this->tabObject->by_object[$cls]->auto_activate= TRUE;
                @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
                    $cls, "<b>Setting auto_activate=TRUE for</b>");
              }
            }

            // Enable sending of LDAP events
            if (isset($this->tabObject->by_object["workstartup"])){
              $this->tabObject->by_object["workstartup"]->gotoLdap_inherit= TRUE;
              @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
                  "", "<b>Setting workstartup->gotoLdap_inherit=TRUE</b>");
            }
          }

          // Try to inherit everythin from the selected object group and then save
          //  the entry, normally this should work without any problems.
          // But if there is any, then display the dialogs.
          if($data['OG'] != "none"){
            $str = $this->saveChanges();

            // There was a problem, skip activation here and allow to fix the problems..
            if(is_object($this->tabObject)){
              @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
                  "", "<b>Automatic saving failed, let the user fix the issues now.</b>");
              return;
            }
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
                "", "<b>System activated!</b>");
          }else{
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
                "", "<b>Open dialogs now</b>");
            return;
          }
        }
      }
    }
  }


  /*! \brief  Save object modifications here and any dialogs too.
   *          After a successfull update of the object data, close
   *           the dialogs.
   */
  protected function saveChanges()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
        get_class($this->tabObject).": ".$this->tabObject->dn, "<b>Save</b>");

    // Try to save changes here.
    $str = management::saveChanges();
    if($this->tabObject) return("");

    // Activate system if required..
    if(isset($this->activationQueue[$this->last_dn])){
      $dn = $this->last_tabObject->dn;
      $this->activate_new_device($dn);
      unset($this->activationQueue[$this->last_dn]);
    }

    if(isset($this->last_tabObject->was_activated) && $this->last_tabObject->was_activated) {
      $this->activate_new_device($this->last_tabObject->dn);
    }

    // Avoid using values from an older input dialog
    $_POST = array();
    $this->handleActivationQueue();
  }


  /*! \brief  Save object modifications here and any dialogs too.
   *          Keep dialogs opened.
   */
  protected function applyChanges()
  {
    $str = management::applyChanges();
    if($str) return($str);

    if(isset($this->tabObject->was_activated) && $this->tabObject->was_activated) {
      $this->activate_new_device($this->tabObject->dn);
    }
  }


  /*! \brief  Sets FAIstate to "install" for "New Devices".
    This function is some kind of "Post handler" for activated systems,
    it is called directly after the object (workstabs,servtabs) gets saved.
    @param  String  $dn   The dn of the newly activated object.
    @return Boolean TRUE if activated else FALSE
   */
  function activate_new_device($dn)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
        $dn, "<b>Activating system:</b>");
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->cat($dn);
    if($ldap->count()){
      $attrs = $ldap->fetch();
      if(count(array_intersect(array('goServer','gotoWorkstation'), $attrs['objectClass']))){
        $ocs = $attrs['objectClass'];
        unset($ocs['count']);
        $new_attrs = array();
        if(!in_array("FAIobject",$ocs)){
          $ocs[] = "FAIobject";
          $new_attrs['objectClass'] = $ocs;
        }
        $new_attrs['FAIstate'] = "install";
        $ldap->cd($dn);
        $ldap->modify($new_attrs);
        if (!$ldap->success()){
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn,
                LDAP_MOD, "activate_new_device($dn)"));
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
              $dn, "<b>Failed!</b>");
        }else{
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
              $dn, "<b>Success</b>");
          return(TRUE);
        }
      }else{
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
            $dn, "<b>FAIstate not set to install, this is only done for gotoWorkstation/goServer!</b>");
      }
    }
    return(FALSE);
  }


  /*! \brief  Opens the snapshot creation dialog for the given target.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function createSnapshotDialog($action="",$target=array(),$all=array())
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$target,"Snaptshot creation initiated!");

    $pInfo    = $this->getObjectDefinitions();
    $headpage = $this->getHeadpage();
    foreach($target as $dn){

      $entry = $headpage->getEntry($dn);
      $type = $headpage->getType($dn);
      if(!isset($pInfo[$type])) {
        trigger_error('Unknown system type \''.$type.'\'!');
        return;
      }

      if(!empty($dn) && $this->ui->allow_snapshot_create($dn,$pInfo[$type]['aclCategory'])){
        $this->dialogObject = new SnapShotDialog($this->config,$dn,$this);
        $this->dialogObject->aclCategories = array($pInfo[$type]['aclCategory']);
        $this->dialogObject->parent = &$this;

      }else{
        msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to create a snapshot for %s."),$dn),
            ERROR_DIALOG);
      }
    }
  }


  /*! \brief  Displays the "Restore snapshot dialog" for a given target.
   *          If no target is specified, open the restore removed object
   *           dialog.
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function restoreSnapshotDialog($action="",$target=array(),$all=array())
  {
    // Set current restore base for snapshot handling.
    $headpage = $this->getHeadpage();
    $pInfo    = $this->getObjectDefinitions();
    if(is_object($this->snapHandler)){
      $bases = array();
      foreach($this->storagePoints as $sp){
        $bases[] = $sp.$headpage->getBase();
      }
    }

    // No bases specified? Try base
    if(!count($bases)) $bases[] = $this->headpage->getBase();

    // No target, open the restore removed object dialog.
    if(!count($target)){

      $cats = array();
      foreach($pInfo as $data){
        $cats[] = $data['aclCategory'];
      }
      $cats = array_unique($cats);

      $entry = $headpage->getBase();
      if(!empty($entry) && $this->ui->allow_snapshot_restore($entry,$cats)){
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$entry,"Snaptshot restoring initiated!");
        $this->dialogObject = new SnapShotDialog($this->config,$entry,$this);
        $this->dialogObject->set_snapshot_bases($bases);
        $this->dialogObject->display_all_removed_objects = true;
        $this->dialogObject->display_restore_dialog = true;
        $this->dialogObject->parent = &$this;
      }else{
        msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to restore a snapshot for %s."),$entry),
            ERROR_DIALOG);
      }
    }else{

      // Display the restore points for a given object.
      $dn = array_pop($target);
      $entry = $headpage->getEntry($dn);
      $type = $headpage->getType($dn);
      if(!isset($pInfo[$type])) {
        trigger_error('Unknown system type \''.$type.'\'!');
        return;
      }

      if(!empty($dn) && $this->ui->allow_snapshot_create($dn,$pInfo[$type]['aclCategory'])){
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$dn,"Snaptshot restoring initiated!");
        $this->dialogObject = new SnapShotDialog($this->config,$dn,$this);
        $this->dialogObject->set_snapshot_bases($bases);
        $this->dialogObject->display_restore_dialog = true;
        $this->dialogObject->parent = &$this;
      }else{
        msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to restore a snapshot for %s."),$dn),
            ERROR_DIALOG);
      }
    }
  }


  /*! \brief  Restores a snapshot object.
   *          The dn of the snapshot entry has to be given as ['target'] parameter.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function restoreSnapshot($action="",$target=array(),$all=array())
  {
    $dn       = array_pop($target);
    $this->snapHandler->restore_snapshot($dn);
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$dn,"Snaptshot restored!");
    $this->closeDialogs();
  }


  /*! \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    $action= management::detectPostActions();
    if(isset($_POST['abort_event_dialog']))  $action['action'] = "cancel";
    if(isset($_POST['save_event_dialog']))  $action['action'] = "saveEvent";
    if(isset($_POST['cd_create']))  $action['action'] = "initiateISOcreation";
    if(isset($_GET['PerformIsoCreation']))  $action['action'] = "performIsoCreation";
    if(isset($_POST['SystemTypeAborted']))  $action['action'] = "cancel";

    if(isset($_POST['new_goServer']))  $action['action'] = "new_goServer";
    if(isset($_POST['new_gotoWorkstation']))  $action['action'] = "new_gotoWorkstation";
    if(isset($_POST['new_gotoTerminal']))  $action['action'] = "new_gotoTerminal";
    if(isset($_POST['new_sambaSamAccount']))  $action['action'] = "new_sambaSamAccount";
    if(isset($_POST['new_gotoPrinter']))  $action['action'] = "new_gotoPrinter";
    if(isset($_POST['new_goFonHardware']))  $action['action'] = "new_goFonHardware";
    if(isset($_POST['new_ieee802Device']))  $action['action'] = "new_ieee802Device";

    if(!is_object($this->tabObject) && !is_object($this->dialogObject)){
      if(count($this->activationQueue)) $action['action'] = "handleActivationQueue";
    }

    if(isset($_POST['systemTypeChosen']))  $action['action'] = "systemTypeChosen";

    return($action);
  }


  /*! \brief   Overridden render method of class management.
   *            this allows us to add a release selection box.
   */
  function renderList()
  {
    $headpage = $this->getHeadpage();
    $headpage->update();

    $tD = $this->getObjectDefinitions();
    $smarty = get_smarty();
    foreach($tD as $name => $obj){
      $smarty->assign("USE_".$name, (empty($obj['tabClass']) || class_available($obj['tabClass'])));
    }

    $display = $headpage->render();
    return($this->getHeader().$display);
  }


  public function getObjectDefinitions()
  {
    $tabs = array(
        "goServer" => array(
          "ou"          => get_ou('serverRDN'),
          "plugClass"   => "serverGeneric",
          "tabClass"    => "servtabs",
          "tabDesc"     => "SERVERTABS",
          "aclClass"    => "serverGeneric",
          "sendEvents"  => TRUE,
          "aclCategory" => "server"),

        "gotoWorkstation" => array(
          "ou"          => get_ou('workstationRDN'),
          "plugClass"   => "workstationGeneric",
          "tabClass"    => "worktabs",
          "tabDesc"     => "WORKSTATIONTABS",
          "aclClass"    => "workstationGeneric",
          "sendEvents"  => TRUE,
          "aclCategory" => "workstation"),

        "gotoTerminal" => array(
            "ou"          => get_ou('terminalRDN'),
            "plugClass"   => "terminalGeneric",
            "tabClass"    => "termtabs",
            "sendEvents"  => TRUE,
            "tabDesc"     => "TERMINALTABS",
            "aclClass"    => "terminalGeneric",
            "aclCategory" => "terminal"),

        "gotoPrinter" => array(
            "ou"          => get_ou('printerRDN'),
            "plugClass"   => "printgeneric",
            "tabClass"    => "printtabs",
            "tabDesc"     => "PRINTERTABS",
            "aclClass"    => "printgeneric",
            "sendEvents"  => FALSE,
            "aclCategory" => "printer"),

        "goFonHardware" => array(
            "ou"          => get_ou('phoneRDN'),
            "plugClass"   => "phoneGeneric",
            "tabClass"    => "phonetabs",
            "tabDesc"     => "PHONETABS",
            "sendEvents"  => FALSE,
            "aclClass"    => "phoneGeneric",
            "aclCategory" => "phone"),

        "sambaSamAccount" => array(
            "ou"          => get_winstations_ou(),
            "plugClass"   => "winstationGeneric",
            "sendEvents"  => TRUE,
            "tabClass"    => "wintabs",
            "tabDesc"     => "WINSTATIONTABS",
            "aclClass"    => "winstationGeneric",
            "aclCategory" => "winstation"),

        "ieee802Device" => array(
            "ou"          => get_ou('componentRDN'),
            "plugClass"   => "componentGeneric",
            "sendEvents"  => FALSE,
            "tabClass"    => "componenttabs",
            "tabDesc"     => "COMPONENTTABS",
            "aclClass"    => "componentGeneric",
            "aclCategory" => "component"),

        "fdMobilePhone"   => array(
            "ou"          => get_ou('mobilePhoneRDN'),
            "plugClass"   => "mobilePhoneGeneric",
            "tabClass"    => "mobiletabs",
            "tabDesc"     => "MOBILEPHONETABS",
            "aclClass"    => "mobilePhoneGeneric",
            "sendEvents"  => FALSE,
            "aclCategory" => "mobilePhone"),
        );

    // Now map some special types
    $tabs['FAKE_OC_NewWorkstation'] = &$tabs['gotoWorkstation'];
    $tabs['FAKE_OC_NewTerminal'] = &$tabs['gotoTerminal'];
    $tabs['FAKE_OC_NewServer'] = &$tabs['gotoWorkstation'];
    $tabs['gotoWorkstation__IS_BUSY'] = &$tabs['gotoWorkstation'];
    $tabs['gotoWorkstation__IS_ERROR'] = &$tabs['gotoWorkstation'];
    $tabs['gotoWorkstation__IS_LOCKED'] = &$tabs['gotoWorkstation'];
    $tabs['gotoTerminal__IS_BUSY'] = &$tabs['gotoTerminal'];
    $tabs['gotoTerminal__IS_ERROR'] = &$tabs['gotoTerminal'];
    $tabs['gotoTerminal__IS_LOCKED'] = &$tabs['gotoTerminal'];
    $tabs['FAKE_OC_TerminalTemplate'] = &$tabs['gotoTerminal'];
    $tabs['FAKE_OC_WorkstationTemplate'] = &$tabs['gotoTerminal'];
    $tabs['goServer__IS_BUSY'] = &$tabs['goServer'];
    $tabs['goServer__IS_ERROR'] = &$tabs['goServer'];
    $tabs['goServer__IS_LOCKED'] = &$tabs['goServer'];

    return $tabs;
  }


  static function systemRelease($a,$b,$c,$objectclasses= null,$class= null)
  {
    global $config;

    // No objectclasses set - go ahead
    if(!$objectclasses) return("&nbsp;");

    // Skip non fai objects
    if (!in_array_ics("FAIobject", $objectclasses)) {
      return "&nbsp;";
    }

    // If we've an own fai class, just use this
    if ($class && is_array($class)) {
      foreach (explode(' ', $class[0]) as $element) {
        if ($element[0] == ":") {
          return "&nbsp;".htmlentities(mb_substr($element, 1), ENT_COMPAT, 'UTF-8');
        }
      }
    }

    // Load information if needed
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search("(&(objectClass=gosaGroupOfNames)(FAIclass=*)(member=".$b."))",array('FAIclass','cn'));
    while ($attrs = $ldap->fetch()) {
      $rel = htmlentities(preg_replace("/^.*:/", "", $attrs['FAIclass'][0]), ENT_COMPAT, 'UTF-8');
      $sys = htmlentities(sprintf(_("Inherited from %s"), $attrs['cn'][0]), ENT_COMPAT, 'UTF-8');
      $str = "&nbsp;<img class='center' src='plugins/ogroups/images/ogroup.png'
          title='".$sys."'
          alt='".$sys."' >&nbsp;".$rel;
      return $str;
    }

    return "&nbsp;";
  }

  static function listServices($row, $dn)
  {
    global $config;
    static $services = array();
    if (empty($services)) {
      foreach ($config->data['TABS']['SERVERSERVICE'] as $plug) {
        if (class_available($plug['CLASS'])) {
          $name = $plug['CLASS'];

          $services[$name] = new $name($config, $dn);
        }
      }
    }

    // Load information if needed
    $ldap = $config->get_ldap_link();
    $ldap->cat($dn);
    $services_imgs = array();
    if ($attrs = $ldap->fetch()) {
      foreach ($services as $class => $service) {
        if ($service->is_this_account($attrs)) {
          $infos = pluglist::pluginInfos($class);
          if (isset($infos['plIcon'])) {
            $services_imgs[] = "<input class='center' type='image' src='".$infos['plIcon']."' ".
                               "alt='".$infos['plShortName']."' title='".$infos['plShortName']."' ".
                               "name='listing_edit_".$class."_$row' style='padding:1px'/>";
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $infos['plShortName']." ($class)", "No icon for");
          }
        }
      }
    }

    $str = implode("", $services_imgs);
    if ($str == "") {
      $str = "&nbsp;";
    }
    return $str;
  }

  /*! \brief  !! Incoming dummy acls, required to defined acls for incoming objects
   */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Systems"),
      "plDescription" => _("Systems Management"),
      "plIcon"        => "plugins/systems/images/icon.png",
      "plSelfModify"  => FALSE,
      "plPriority"    => 20,
      "plSection"     => "admin",
      "plCategory"    => array("terminal","workstation","server","phone","printer","component","winstation"),

      "plProvidedAcls"  => array()
    );
  }
}
?>
