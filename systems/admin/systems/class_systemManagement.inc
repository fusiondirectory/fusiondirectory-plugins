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

class systemManagement extends simpleManagement
{
  protected $objectTypes  = array('server','workstation','terminal','printer','component','phone','mobilePhone');

  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = TRUE;
  protected $multiSelect            = TRUE;

  protected $autoFilter       = TRUE;
  protected $autoActions      = TRUE;

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'ipHostNumber', 'macAddress', 'gotoMode', 'FAIstate', 'FAIclass');

  public static $skipTemplates = FALSE;

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
      'plManages'     => array('server','workstation','terminal','printer','component','phone','mobilePhone'),

      'plProvidedAcls'  => array()
    );
  }

  function __construct()
  {
    $this->listXMLPath    = get_template_path('system-list.xml', TRUE, dirname(__FILE__));

    // Check if we are able to communicate with Argonaut server
    if (class_available('supportDaemon') && class_available('argonautAction')) {
      $o = new supportDaemon();
      $this->siActive = $o->is_available();
    }

    parent::__construct();
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();

    $this->headpage->registerElementFilter('systemRelease', 'systemManagement::systemRelease');
    $this->headpage->registerElementFilter('listServices',  'systemManagement::listServices');

    // Register Daemon Events
    if ($this->siActive) {
      $this->headpage->xmlData['actionmenu']['action'][2]['action'] = array();
      $this->headpage->xmlData['actionmenu']['action'][3]['action'] = array();
      $events = argonautEventTypes::get_event_types();
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
      $this->registerAction('ping', 'pingSystems');
    } else {
      unset($this->headpage->xmlData['actionmenu']['action'][2]);
      unset($this->headpage->xmlData['actionmenu']['action'][3]);
      unset($this->headpage->xmlData['actionmenu']['action'][4]);
      $this->headpage->xmlData['actionmenu']['action'] = array_values($this->headpage->xmlData['actionmenu']['action']);
    }
    $this->registerAction('saveEvent', 'saveEventDialog');
  }

  function configureFilter ()
  {
    parent::configureFilter();
    /* We need all attributes to list services */
    $this->filter->query[0]['attribute'] = '*';
  }

  /*! \brief    Handle Argonaut events
   *            All schedules and triggered events are handled here.
   */
  function handleEvent($action, array $target)
  {
    if (!$this->siActive) {
      return;
    }

    // Detect whether this event is scheduled or triggered.
    $triggered = TRUE;
    if (preg_match('/^S_/', $action)) {
      $triggered = FALSE;
    }

    // Detect triggered or scheduled actions
    $headpage = $this->getHeadpage();
    $event = preg_replace('/^[TS]_/', '', $action);

    // Now send FAI/Argonaut events here.
    $mac = array();

    // Collect target mac addresses
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

    $o_queue  = new supportDaemon();

    /* Skip installation or update trigerred events,
     *  if this entry is currently processing.
     */
    if ($triggered && in_array($event, array('reinstall','update'))) {
      foreach ($mac as $key => $mac_address) {
        if ($o_queue->is_currently_installing($mac_address)) {
          msg_dialog::display(_('Action canceled'), sprintf(_('System %s is currently installing'), $dn), ERROR_DIALOG);
          unset($mac[$key]);
          logging::log('security', 'systems/'.get_class($this), '', array(), 'Skip adding "argonautAction::'.$event.'" for mac "'.$mac_address.'", there is already a job in progress.');
        }
      }
    }

    if ((count($mac) == 0) && $triggered) {
      return;
    }

    // Prepare event to be added
    $events   = argonautEventTypes::get_event_types();
    if (isset($events[$event])) {
      $this->dialogObject = new argonautAction($event, $mac, !$triggered);

      if ($triggered) {
        $res = $o_queue->append($this->dialogObject);
        if ($o_queue->is_error()) {
          msg_dialog::display(_('Infrastructure service'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
        } else {
          if (is_array($res) && count($res) > 1) {
            msg_dialog::display(_('Action triggered'), sprintf(_('Action called without error (results were "%s")'), implode(', ', $res)), INFO_DIALOG);
          } else {
            if (is_array($res)) {
              $res = $res[0];
            }
            msg_dialog::display(_('Action triggered'), sprintf(_('Action called without error (result was "%s")'), $res), INFO_DIALOG);
          }
        }
        $this->closeDialogs();
      }
    }
  }

  /*! \brief Ping those systems and show the result in the list
   */
  function pingSystems($action, array $target)
  {
    if (!$this->siActive) {
      return;
    }

    $headpage = $this->getHeadpage();
    $macs     = array();

    // Collect target mac addresses
    foreach ($target as $dn) {
      $obj = $headpage->getEntry($dn);
      if (isset($obj['macAddress'][0])) {
        $macs[$dn] = $obj['macAddress'][0];
      } else {
        msg_dialog::display(_('Action canceled'), sprintf(_('System %s has no mac address defined, cannot trigger action'), $dn), ERROR_DIALOG);
      }
    }

    if (count($macs) == 0) {
      return;
    }

    $o_queue  = new supportDaemon();

    $res = $o_queue->append_call('ping', array_values($macs), array('fullresult' => 1));
    if ($o_queue->is_error()) {
      msg_dialog::display(_('Infrastructure service'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
    } else {
      $dns = array_flip($macs);
      $msg = '';
      if (!empty($res['results'])) {
        $msg .= '<ul>'."\n";
        foreach ($res['results'] as $mac => $on) {
          $msg .= '<li style="list-style-type:'.($on ? 'disc' : 'circle').';">'.objects::link($dns[$mac], $this->getType($dns[$mac])).' - '.($on ? 'On' : 'Off').'</li>'."\n";
        }
        $msg .= "</ul>\n";
      }
      if (!empty($res['errors'])) {
        $msg .= '<ul style="list-style-type:square;">'."\n";
        foreach ($res['errors'] as $mac => $error) {
          $msg .= '<li>'.objects::link($dns[$mac], $this->getType($dns[$mac])).' - '.$error.'</li>'."\n";
        }
        $msg .= "</ul>\n";
      }
      msg_dialog::display(_('Ping results'), $msg, INFO_DIALOG);
    }
  }

  /*! \brief  Save event dialogs.
   *          And append the new Argonaut event.
   */
  function saveEventDialog()
  {
    $this->dialogObject->save_object();
    $msgs = $this->dialogObject->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    }
    if ($this->siActive) {
      $o_queue = new supportDaemon();
      $o_queue->append($this->dialogObject);
      if ($o_queue->is_error()) {
        msg_dialog::display(_('Infrastructure service'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
      }
      $this->closeDialogs();
    }
  }

  /*! \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    $action = parent::detectPostActions();
    if (isset($_POST['save_event_dialog'])) {
      $action['action'] = 'saveEvent';
    } elseif (isset($_POST['abort_event_dialog'])) {
      $action['action'] = 'cancel';
    }
    return $action;
  }

  function handleSubAction($all)
  {
    if (parent::handleSubAction($all)) {
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

  static function listServices($row, $dn, $attrs)
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
    $services_imgs = array();
    if (!empty($attrs)) {
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
