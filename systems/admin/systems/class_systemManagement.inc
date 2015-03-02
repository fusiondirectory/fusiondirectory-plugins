<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class systemManagement extends simpleManagement
{
  /*
   * TODO
   * Fix snapshots if needed
   * */

  protected $objectTypes  = array('server','workstation','terminal','printer','component','winstation','phone','mobilePhone');

  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = TRUE;
  protected $multiSelect            = TRUE;

  protected $autoFilter       = TRUE;
  protected $autoActions      = TRUE;

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'ipHostNumber', 'macAddress', 'gotoMode', 'FAIstate', 'FAIclass');

  protected $skipTemplates = FALSE;

  protected $siActive = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Systems'),
      'plDescription' => _('Systems Management'),
      'plIcon'        => 'geticon.php?context=devices&icon=computer&size=48',
      'plSelfModify'  => FALSE,
      'plSection'     => array('systems' => array('name' => _('Systems'), 'priority' => 10)),
      'plPriority'    => 0,
      'plManages'     => array('server','workstation','terminal','printer','component','winstation','phone'),

      'plProvidedAcls'  => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->listXMLPath    = get_template_path('system-list.xml', TRUE, dirname(__FILE__));

    // Check if we are able to communicate with Argonaut server
    if (class_available("supportDaemon")) {
      $o = new supportDaemon();
      $this->siActive = $o->is_available() && class_available("DaemonEvent");
    }

    parent::__construct($config, $ui);
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();

    $this->headpage->registerElementFilter('systemRelease', 'systemManagement::systemRelease');
    $this->headpage->registerElementFilter('listServices',  'systemManagement::listServices');

    // Register Daemon Events
    if (class_available('DaemonEvent') && class_available('supportDaemon')) {
      $this->headpage->xmlData['actionmenu']['action'][2]['action'] = array();
      $this->headpage->xmlData['actionmenu']['action'][3]['action'] = array();
      $events = DaemonEvent::get_event_types();
      foreach ($events as $name => $data) {
        $this->registerAction('T_'.$name, 'handleEvent');
        $this->headpage->xmlData['actionmenu']['action'][2]['action'][] = array(
        'name' => 'T_'.$name,
        'type' => 'entry',
        'image' => $data['img'],
        'label' => $data['name'],
        );
        $this->registerAction('S_'.$name, 'handleEvent');
        $this->headpage->xmlData['actionmenu']['action'][3]['action'][] = array(
        'name' => 'S_'.$name,
        'type' => 'entry',
        'image' => $data['img'],
        'label' => $data['name'],
        );
      }
    } else {
      unset($this->headpage->xmlData['actionmenu']['action'][2]);
      unset($this->headpage->xmlData['actionmenu']['action'][3]);
      $this->headpage->xmlData['actionmenu']['action'] = array_values($this->headpage->xmlData['actionmenu']['action']);
    }
    $this->registerAction('saveEvent', 'saveEventDialog');
  }

  /*! \brief    Handle Argonaut events
   *            All schedules and triggered events are handled here.
   */
  function handleEvent($action = '', $target = array(), $all = array())
  {
    // Detect whether this event is scheduled or triggered.
    $triggered = TRUE;
    if (preg_match('/^S_/', $action)) {
      $triggered = FALSE;
    }

    // Detect triggered or scheduled actions
    $headpage = $this->getHeadpage();
    $event = preg_replace('/^[TS]_/', '', $action);

    echo "<h1>launching action $action</h1>\n";

    // Now send FAI/Argonaut events here.
    if (count($target) && $this->siActive) {
      $mac = array();

      // Collect target mac addresses
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
      if ($triggered && in_array($event, array('reinstall','update'))) {
        foreach ($mac as $key => $mac_address) {
          if ($o_queue->is_currently_installing($mac_address)) {
            unset($mac[$key]);
            new log('security', 'systems/'.get_class($this), '', array(), 'Skip adding "DaemonEvent::'.$type.'" for mac "'.$mac_address.'", there is already a job in progress.');
          }
        }
      }

      // Prepare event to be added
      if (count($mac) && isset($events[$event]) && $this->siActive) {
        $this->dialogObject = new DaemonEvent($this->config, $event);
        $this->dialogObject->add_targets($mac);

        if ($triggered) {
          $this->dialogObject->set_type(TRIGGERED_EVENT);
          $o_queue->append($this->dialogObject);
          if ($o_queue->is_error()) {
            msg_dialog::display(_('Service infrastructure'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
            $this->closeDialogs();
          } else {
            $this->closeDialogs();
          }
        } else {
          $this->dialogObject->set_type(SCHEDULED_EVENT);
        }
      }
    }
  }

  /*! \brief  Save event dialogs.
   *          And append the new Argonaut event.
   */
  function saveEventDialog()
  {
    if ($this->siActive) {
      $o_queue = new supportDaemon();
      $o_queue->append($this->dialogObject);
      if ($o_queue->is_error()) {
        msg_dialog::display(_('Service infrastructure'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
      } else {
        $this->closeDialogs();
      }
    }
  }
  /*! \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    $action = parent::detectPostActions();
    if (isset($_POST['abort_event_dialog']))  $action['action'] = 'cancel';
    if (isset($_POST['save_event_dialog']))   $action['action'] = 'saveEvent';
    return $action;
  }

  function handleSubAction ($all)
  {
    if (parent::handleSubAction()) {
      return TRUE;
    } elseif (preg_match('/^service_([a-zA-Z_]+)$/', $all['subaction'], $m)) {
      $service = $m[1];
      if (isset($this->tabObject->by_object['ServerService'])) {
        $this->tabObject->current = 'ServerService';
        $all = array('action'  => 'edit', 'targets' => array($service));
        $this->tabObject->by_object['ServerService']->editEntry($all['action'], $all['targets'], $all);
      } else {
        trigger_error("Unknown tab: ServerService");
      }
      return TRUE;
    }
    return FALSE;
  }

  static function systemRelease($a, $b, $c, $objectclasses = NULL, $class = NULL)
  {
    global $config;

    // No objectclasses set - go ahead
    if (!$objectclasses) {
      return '&nbsp;';
    }

    // Skip non fai objects
    if (!in_array_ics('FAIobject', $objectclasses)) {
      return '&nbsp;';
    }

    // If we've an own fai class, just use this
    if ($class && is_array($class)) {
      foreach (explode(' ', $class[0]) as $element) {
        if ($element[0] == ':') {
          return '&nbsp;'.htmlentities(mb_substr($element, 1), ENT_COMPAT, 'UTF-8');
        }
      }
    }

    // Load information if needed
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=gosaGroupOfNames)(FAIclass=*)(member='.$b.'))', array('FAIclass','cn'));
    while ($attrs = $ldap->fetch()) {
      $rel = htmlentities(preg_replace('/^.*:/', '', $attrs['FAIclass'][0]), ENT_COMPAT, 'UTF-8');
      $sys = htmlentities(sprintf(_('Inherited from %s'), $attrs['cn'][0]), ENT_COMPAT, 'UTF-8');
      $str = '&nbsp;<img class="center" src="plugins/ogroups/images/ogroup.png"
          title="'.$sys.'"
          alt="'.$sys.'"/>&nbsp;'.$rel;
      return $str;
    }

    return '&nbsp;';
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
            $services_imgs[] = '<input class="center" type="image" src="'.$infos['plIcon'].'" '.
                               'alt="'.$infos['plShortName'].'" title="'.$infos['plShortName'].'" '.
                               'name="listing_edit_service_'.$class.'_'.$row.'"/>';
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $infos['plShortName']." ($class)", "No icon for");
          }
        }
      }
    }

    $str = implode('', $services_imgs);
    if ($str == '') {
      $str = '&nbsp;';
    }
    return $str;
  }
}
?>
