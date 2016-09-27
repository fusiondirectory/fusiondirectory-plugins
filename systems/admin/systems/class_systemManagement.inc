<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class systemManagement extends management
{
  // Tab definition
  protected $tabClass         = "";
  protected $tabType          = "";
  protected $aclCategory      = "";
  protected $aclPlugin        = "";
  protected $objectName       = "system";
  protected $objectInfo       = array();

  function __construct()
  {

    // Set storage points
    $typeDefs = $this->getObjectDefinitions();
    $sP = array();
    foreach ($typeDefs as $entry) {
      if (!empty($entry['ou'])) {
        $sP[] = $entry['ou'];
      }
    }
    $this->storagePoints = array_unique($sP);

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("system-filter.xml", TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("system-list.xml", TRUE));
    $headpage->registerElementFilter("systemRelease", "systemManagement::systemRelease");
    $headpage->registerElementFilter("listServices", "systemManagement::listServices");
    $headpage->setFilter($filter);

    // Register Daemon Events
    if (class_available('argonautAction') && class_available('supportDaemon')) {
      $this->headpage->xmlData['actionmenu']['action'][2]['action'] = array();
      $this->headpage->xmlData['actionmenu']['action'][3]['action'] = array();
      $events = argonautEventTypes::get_event_types();
      foreach ($events as $name => $data) {
        $this->registerAction("T_".$name, "handleEvent");
        $headpage->xmlData['actionmenu']['action'][2]['action'][] = array(
        'name' => 'T_'.$name,
        'type' => 'entry',
        'image' => $data['img'],
        'label' => $data['name'],
        );
        $this->registerAction("S_".$name, "handleEvent");
        $headpage->xmlData['actionmenu']['action'][3]['action'][] = array(
        'name' => 'S_'.$name,
        'type' => 'entry',
        'image' => $data['img'],
        'label' => $data['name'],
        );
      }
    } else {
      unset($headpage->xmlData['actionmenu']['action'][2]);
      unset($headpage->xmlData['actionmenu']['action'][3]);
      $headpage->xmlData['actionmenu']['action'] = array_values($headpage->xmlData['actionmenu']['action']);
    }
    $this->registerAction("saveEvent", "saveEventDialog");

    foreach (array_keys($typeDefs) as $type) {
      $this->registerAction("new_$type",         "newEntry");
    }

    parent::__construct("systems", $headpage);
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

      $tabClass[$dn]    = $tDefs[$type]['tabClass'];
      $tabType[$dn]     = $tDefs[$type]['tabDesc'];
      $aclCategory[$dn] = $tDefs[$type]['aclCategory'];
      $aclPlugin[$dn]   = $tDefs[$type]['aclClass'];
    }

    return parent::copyPasteHandler($action, $target, $all, $tabClass, $tabType, $aclCategory, $aclPlugin);
  }

  /*! \brief    Handle Argonaut events
   *            All schedules and triggered events are handled here.
   */
  function handleEvent($action = "", $target = array(), $all = array())
  {
    // Detect whether this event is scheduled or triggered.
    $triggered = TRUE;
    if (preg_match("/^S_/", $action)) {
      $triggered = FALSE;
    }

    // Detect triggered or scheduled actions
    $headpage = $this->getHeadpage();
    $event = preg_replace("/^[TS]_/", "", $action);

    // Now send FAI/Argonaut events here.
    $mac = array();

    // Collect target mac addresses
    $events   = argonautEventTypes::get_event_types();
    $o_queue  = new supportDaemon();
    foreach ($target as $dn) {
      $obj = $headpage->getEntry($dn);
      if (isset($obj['macAddress'][0])) {
        $mac[] = $obj['macAddress'][0];
      } else {
        msg_dialog::display(_('Action canceled'), sprintf(_('System %s has no mac address defined, cannot trigger action'), $dn), ERROR_DIALOG);
      }
    }

    if ((count($mac) == 0) && $triggered) {
      return;
    }

    /* Skip installation or update trigerred events,
     *  if this entry is currently processing.
     */
    if ($triggered && in_array($event, array("reinstall","update"))) {
      foreach ($mac as $key => $mac_address) {
        if ($o_queue->is_currently_installing($mac_address)) {
          msg_dialog::display(_('Action canceled'), sprintf(_('System %s is currently installing'), $dn), ERROR_DIALOG);
          unset($mac[$key]);
          logging::log('security', 'systems/'.get_class($this), '', array(), 'Skip adding "argonautAction::'.$type.'" for mac "'.$mac_address.'", there is already a job in progress.');
        }
      }
    }

    // Prepare event to be added
    if (isset($events[$event])) {
      $this->dialogObject = new argonautAction($event);
      $this->dialogObject->targets = $mac;

      if ($triggered) {
        $this->dialogObject->scheduled = FALSE;
        $o_queue->append($this->dialogObject);
        if ($o_queue->is_error()) {
          msg_dialog::display(_("Infrastructure service"), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
          $this->closeDialogs();
        } else {
          $this->closeDialogs();
        }
      } else {
        $this->dialogObject->scheduled = TRUE;
      }
    }
  }

  /*! \brief  Save event dialogs.
   *          And append the new Argonaut event.
   */
  function saveEventDialog()
  {
    $msgs = $this->dialogObject->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    }
    $o_queue = new supportDaemon();
    $o_queue->append($this->dialogObject);
    if ($o_queue->is_error()) {
      msg_dialog::display(_('Infrastructure service'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
      $this->closeDialogs();
    } else {
      $this->closeDialogs();
    }
  }

  /*! \brief    Queue selected objects to be removed.
   *            Checks ACLs, Locks and ask for confirmation.
   */
  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    global $ui;
    $disallowed = array();
    $this->dns  = array();

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removel requested!");

    // Check permissons for each target
    $tInfo    = $this->getObjectDefinitions();
    $headpage = $this->getHeadpage();
    foreach ($target as $dn) {
      $type = $headpage->getType($dn);
      if (!isset($tInfo[$type])) {
        trigger_error("Unknown object type received '".$type."' please update systemManagement::getObjectDefinitions()!");
      } else {
        $info = $tInfo[$type];
        $acl  = $ui->get_permissions($dn, $info['aclCategory']."/".$info['aclClass']);
        if (preg_match("/d/", $acl)) {
          $this->dns[] = $dn;
        } else {
          $disallowed[] = $dn;
        }
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_("Permission"), msgPool::permDelete($disallowed), INFO_DIALOG);
    }

    // We've at least one entry to delete.
    if (count($this->dns)) {
      // check locks
      if ($locks = get_locks($this->dns)) {
        return gen_locked_message($locks, $this->dns);
      }

      // Add locks
      $dns_names = array();
      foreach ($this->dns as $dn) {
        $dns_names[] = LDAP::fix($dn);
      }
      add_lock ($this->dns, $ui->dn);

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign("info", msgPool::deleteInfo($dns_names, _($this->objectName)));
      $smarty->assign("multiple", TRUE);
      return $smarty->fetch(get_template_path('remove.tpl', TRUE));
    }
  }


  /*! \brief  Object removal was confirmed, now remove the requested entries.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function removeEntryConfirmed($action = "", $target = array(), $all = array(),
      $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    global $config, $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removel confirmed!");

    // Check permissons for each target
    $tInfo = $this->getObjectDefinitions();
    $headpage = $this->getHeadpage();
    $disallowed = array();
    foreach ($this->dns as $dn) {
      $type = $headpage->getType($dn);
      if (!isset($tInfo[$type])) {
        trigger_error("Unknown object type received '".$type."' please update systemManagement::getObjectDefinitions()!");
      } else {

        $info = $tInfo[$type];
        $acl = $ui->get_permissions($dn, $info['aclCategory']."/".$info['aclClass']);
        if (preg_match("/d/", $acl)) {

          // Delete the object
          $this->dn = $dn;
          if ($info['tabClass'] == "phonetabs") {
             $this->tabObject = new $info['tabClass'](NULL, $config->data['TABS'][$info['tabDesc']], $dn,$type);
             $this->tabObject->set_acl_base($dn);
             $this->tabObject->by_object['phoneGeneric']->remove_from_parent ();
          } else {
            $this->tabObject = new $info['tabClass'](NULL, $config->data['TABS'][$info['tabDesc']],
                $this->dn, $info['aclCategory']);
            $this->tabObject->set_acl_base($this->dn);
            $this->tabObject->parent = $this;
            $this->tabObject->delete ();
          }

          // Remove the lock for the current object.
          del_lock($this->dn);

        } else {
          $disallowed[] = $dn;
          logging::log('security', 'system/'.get_class($this), $dn, array(), 'Tried to trick deletion.');
        }
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_("Permission"), msgPool::permDelete($disallowed), INFO_DIALOG);
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
      if (isset($all['subaction'])) {
        if (preg_match('/^tab_([a-zA-Z_]+)$/', $all['subaction'], $m)) {
          $tab = $m[1];
          if (isset($this->tabObject->by_object[$tab])) {
            $this->tabObject->current = $tab;
          } else {
            trigger_error('Unknown tab: '.$tab);
          }
        } elseif (preg_match('/^service_([a-zA-Z_]+)$/', $all['subaction'], $m)) {
          $service = $m[1];
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
  }


  /*! \brief  Edit the selected system type.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function newEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    $tInfo = $this->getObjectDefinitions();
    $info = preg_replace("/^new_/", "", $action);
    if (!isset($tInfo[$info])) {
      trigger_error("Unknown action type '".$action."' cant create a new system!");
    } else {
      return management::newEntry(
        $action, $target, $all,
        $tInfo[$info]['tabClass'], $tInfo[$info]['tabDesc'], $tInfo[$info]['aclCategory']
      );
    }
  }

  /*! \brief  Opens the snapshot creation dialog for the given target.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function createSnapshotDialog($action = "", $target = array(), $all = array())
  {
    global $config, $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Snaptshot creation initiated!");

    $pInfo    = $this->getObjectDefinitions();
    $headpage = $this->getHeadpage();
    if (count($target) == 1) {
      $this->dn = array_pop($target);
      if (empty($this->dn)) {
        return;
      }
      $type = $headpage->getType($this->dn);
      if (!isset($pInfo[$type])) {
        trigger_error('Unknown system type \''.$type.'\'!');
        return;
      }

      if ($ui->allow_snapshot_create($this->dn, $pInfo[$type]['aclCategory'])) {
        $this->dialogObject = new SnapshotCreateDialog($this->dn, $this, $pInfo[$type]['aclCategory']);
        $this->dialogObject->set_acl_base($this->dn);
      } else {
        msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to create a snapshot for %s.'), $this->dn),
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
  function restoreSnapshotDialog($action = "", $target = array(), $all = array())
  {
    global $ui;
    // Set current restore base for snapshot handling.
    $pInfo    = $this->getObjectDefinitions();

    // Set current restore base for snapshot handling.
    if (is_object($this->snapHandler)) {
      $bases = array();
      foreach ($this->storagePoints as $sp) {
        $bases[] = $sp.$this->headpage->getBase();
      }
    }

    // No bases specified? Try base
    if (!count($bases)) $bases[] = $this->headpage->getBase();


    // No target, open the restore removed object dialog.
    if (!count($target)) {
      $cats = array();
      foreach ($pInfo as $data) {
        $cats[] = $data['aclCategory'];
      }
      $cats = array_unique($cats);

      $this->dn = $this->headpage->getBase();
    } else {
      // Display the restore points for a given object.
      $this->dn = reset($target);
      $entry    = $this->headpage->getEntry($this->dn);
      $type     = $this->headpage->getType($this->dn);
      if (!isset($pInfo[$type])) {
        trigger_error('Unknown system type \''.$type.'\'!');
        return;
      }

      $cats = $pInfo[$type]['aclCategory'];
    }
    if (empty($this->dn)) {
      return;
    }

    if ($ui->allow_snapshot_restore($this->dn, $cats)) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'Snaptshot restoring initiated!');
      $this->snapHandler->setSnapshotBases($bases);
      $this->dialogObject = new SnapshotRestoreDialog($this->dn, $this, !count($target), $cats);
      $this->dialogObject->set_acl_base($this->dn);
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to restore a snapshot for %s.'), $this->dn),
          ERROR_DIALOG);
    }
  }

  /*! \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    $action = management::detectPostActions();
    if (isset($_POST['abort_event_dialog']))  $action['action'] = "cancel";
    if (isset($_POST['save_event_dialog']))  $action['action'] = "saveEvent";

    if (isset($_POST['new_goServer']))  $action['action'] = "new_goServer";
    if (isset($_POST['new_gotoWorkstation']))  $action['action'] = "new_gotoWorkstation";
    if (isset($_POST['new_gotoTerminal']))  $action['action'] = "new_gotoTerminal";
    if (isset($_POST['new_sambaSamAccount']))  $action['action'] = "new_sambaSamAccount";
    if (isset($_POST['new_fdPrinter']))  $action['action'] = "new_fdPrinter";
    if (isset($_POST['new_fdPhone']))  $action['action'] = "new_fdPhone";
    if (isset($_POST['new_ieee802Device']))  $action['action'] = "new_ieee802Device";

    return $action;
  }


  /*! \brief   Overridden render method of class management.
   *            this allows us to add a release selection box.
   */
  function renderList()
  {
    $headpage = $this->getHeadpage();
    $headpage->update();

    $tD     = $this->getObjectDefinitions();
    $smarty = get_smarty();
    foreach ($tD as $name => $obj) {
      $smarty->assign("USE_".$name, (empty($obj['tabClass']) || class_available($obj['tabClass'])));
    }

    $display = $headpage->render();
    return $this->getHeader().$display;
  }


  public function getObjectDefinitions()
  {
    $tabs = array(
      'goServer' => array(
        'ou'          => get_ou('serverRDN'),
        'plugClass'   => 'serverGeneric',
        'tabClass'    => 'servtabs',
        'tabDesc'     => 'SERVERTABS',
        'aclClass'    => 'serverGeneric',
        'sendEvents'  => TRUE,
        'aclCategory' => 'server'
      ),
      'gotoWorkstation' => array(
        'ou'          => get_ou('workstationRDN'),
        'plugClass'   => 'workstationGeneric',
        'tabClass'    => 'simpleTabs',
        'tabDesc'     => 'WORKSTATIONTABS',
        'aclClass'    => 'workstationGeneric',
        'sendEvents'  => TRUE,
        'aclCategory' => 'workstation'
      ),
      'gotoTerminal' => array(
        'ou'          => get_ou('terminalRDN'),
        'plugClass'   => 'terminalGeneric',
        'tabClass'    => 'simpleTabs',
        'sendEvents'  => TRUE,
        'tabDesc'     => 'TERMINALTABS',
        'aclClass'    => 'terminalGeneric',
        'aclCategory' => 'terminal'
      ),
      'fdPrinter' => array(
        'ou'          => get_ou('printerRDN'),
        'plugClass'   => 'printGeneric',
        'tabClass'    => 'simpleTabs',
        'tabDesc'     => 'PRINTERTABS',
        'aclClass'    => 'printGeneric',
        'sendEvents'  => FALSE,
        'aclCategory' => 'printer'
      ),
      'fdPhone' => array(
        'ou'          => get_ou('phoneRDN'),
        'plugClass'   => 'phoneGeneric',
        'tabClass'    => 'simpleTabs',
        'tabDesc'     => 'PHONETABS',
        'sendEvents'  => FALSE,
        'aclClass'    => 'phoneGeneric',
        'aclCategory' => 'phone'
      ),
      'sambaSamAccount' => array(
        'ou'          => get_winstations_ou(),
        'plugClass'   => 'winstationGeneric',
        'sendEvents'  => TRUE,
        'tabClass'    => 'simpleTabs',
        'tabDesc'     => 'WINSTATIONTABS',
        'aclClass'    => 'winstationGeneric',
        'aclCategory' => 'winstation'
      ),
      'ieee802Device' => array(
        'ou'          => get_ou('componentRDN'),
        'plugClass'   => 'componentGeneric',
        'sendEvents'  => FALSE,
        'tabClass'    => 'simpleTabs',
        'tabDesc'     => 'COMPONENTTABS',
        'aclClass'    => 'componentGeneric',
        'aclCategory' => 'component'
      ),
      'fdMobilePhone'   => array(
        'ou'          => get_ou('mobilePhoneRDN'),
        'plugClass'   => 'mobilePhoneGeneric',
        'tabClass'    => 'simpleTabs',
        'tabDesc'     => 'MOBILEPHONETABS',
        'aclClass'    => 'mobilePhoneGeneric',
        'sendEvents'  => FALSE,
        'aclCategory' => 'mobilePhone'
      ),
    );

    // Now map some special types
    $tabs['gotoWorkstation__IS_BUSY']     = &$tabs['gotoWorkstation'];
    $tabs['gotoWorkstation__IS_ERROR']    = &$tabs['gotoWorkstation'];
    $tabs['gotoWorkstation__IS_LOCKED']   = &$tabs['gotoWorkstation'];
    $tabs['gotoTerminal__IS_BUSY']        = &$tabs['gotoTerminal'];
    $tabs['gotoTerminal__IS_ERROR']       = &$tabs['gotoTerminal'];
    $tabs['gotoTerminal__IS_LOCKED']      = &$tabs['gotoTerminal'];
    $tabs['goServer__IS_BUSY']            = &$tabs['goServer'];
    $tabs['goServer__IS_ERROR']           = &$tabs['goServer'];
    $tabs['goServer__IS_LOCKED']          = &$tabs['goServer'];

    return $tabs;
  }


  static function systemRelease($a, $b, $c, $objectclasses = NULL, $class = NULL)
  {
    global $config;

    // No objectclasses set - go ahead
    if (!$objectclasses) {
      return "&nbsp;";
    }

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
    $ldap->search("(&(objectClass=gosaGroupOfNames)(FAIclass=*)(member=".$b."))", array('FAIclass','cn'));
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

          $services[$name] = new $name($dn);
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
                               "name='listing_edit_service_".$class."_$row' style='padding:1px'/>";
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

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Systems'),
      'plDescription' => _('Systems Management'),
      'plIcon'        => 'geticon.php?context=devices&icon=computer&size=48',
      'plSelfModify'  => FALSE,
      'plSection'     => array('systems' => array('name' => _('Systems'), 'priority' => 10)),
      'plPriority'    => 0,
      'plCategory'    => array('terminal','workstation','server','phone','printer','component','winstation'),
      'plManages'     => array('terminal','workstation','server','phone','printer','component','winstation'),

      'plProvidedAcls'  => array()
    );
  }
}
?>
