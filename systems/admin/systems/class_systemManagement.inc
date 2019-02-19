<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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
  public static $skipTemplates = FALSE;

  protected $siActive = FALSE;

  public static $columns = array(
    array('ObjectTypeColumn',     array()),
    array('LinkColumn',           array('attributes' => 'nameAttr',                   'label' => 'Name')),
    array('IpColumn',             array('attributes' => array('ipHostNumber' => '*'), 'label' => 'IP')),
    array('Column',               array('attributes' => array('macAddress' => '*'),   'label' => 'Mac')),
    array('LinkColumn',           array('attributes' => 'description',                'label' => 'Description')),
    array('SystemServicesColumn', array('label' => 'Services')),
    array('SystemReleaseColumn',  array('label' => 'Release')),
    array('ActionsColumn',        array('label' => 'Actions')),
  );

  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Systems'),
      'plTitle'       => _('Systems Management'),
      'plDescription' => _('Manage servers, computers, printers, phones and other devices'),
      'plIcon'        => 'geticon.php?context=devices&icon=computer&size=48',
      'plSection'     => array('systems' => array('name' => _('Systems'), 'priority' => 10)),
      'plPriority'    => 0,
      'plManages'     => array('server','workstation','terminal','printer','component','phone','mobilePhone'),
    );
  }

  function __construct ()
  {
    // Check if we are able to communicate with Argonaut server
    if (class_available('supportDaemon') && class_available('argonautAction')) {
      $o = new supportDaemon();
      $this->siActive = $o->is_available();
    }

    parent::__construct();
  }

  protected function configureActions ()
  {
    parent::configureActions();

    if ($this->siActive) {
      $triggerActions   = array();
      $scheduleActions  = array();
      $events = argonautEventTypes::get_event_types();
      foreach ($events as $name => $infos) {
        $triggerActions[] = new Action(
          'trigger_'.$name, $infos['name'], $infos['img'],
          '*', 'handleEvent'
        );
        $scheduleActions[] = new Action(
          'schedule_'.$name, $infos['name'], $infos['img'],
          '*', 'handleEvent'
        );
      }
      $this->registerAction(
        new SubMenuAction(
          'trigger', _('Trigger action'), 'geticon.php?context=types&icon=action&size=16',
          $triggerActions
        )
      );
      $this->registerAction(
        new SubMenuAction(
          'schedule', _('Schedule action'), 'geticon.php?context=actions&icon=task-schedule&size=16',
          $scheduleActions
        )
      );
      $this->actions['trigger']->setSeparator(TRUE);
      $this->registerAction(new HiddenAction('saveEvent', 'saveEventDialog'));

      /* Add ping action */
      $this->registerAction(
        new Action(
          'ping', _('Ping'), 'geticon.php?context=actions&icon=task-start&size=16',
          '+', 'pingSystems'
        )
      );
    }
  }

  /*! \brief    Handle Argonaut events
   *            All schedules and triggered events are handled here.
   */
  function handleEvent (array $action)
  {
    if (!$this->siActive) {
      return;
    }

    // Detect whether this event is scheduled or triggered.
    $triggered  = ($action['action'] == 'trigger');
    $event      = $action['subaction'];

    // Now send FAI/Argonaut events here.
    $mac = array();

    // Collect target mac addresses
    foreach ($action['targets'] as $dn) {
      $obj = $this->listing->getEntry($dn);
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
          if (is_array($res) && (count($res) > 1)) {
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
  function pingSystems (array $action)
  {
    if (!$this->siActive) {
      return;
    }

    $macs = array();

    // Collect target mac addresses
    foreach ($action['targets'] as $dn) {
      $obj = $this->listing->getEntry($dn);
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
          $msg .= '<li style="list-style-type:'.($on ? 'disc' : 'circle').';">'.objects::link($dns[$mac], $this->listing->getEntry($dns[$mac])->type).' - '.($on ? 'On' : 'Off').'</li>'."\n";
        }
        $msg .= "</ul>\n";
      }
      if (!empty($res['errors'])) {
        $msg .= '<ul style="list-style-type:square;">'."\n";
        foreach ($res['errors'] as $mac => $error) {
          $msg .= '<li>'.objects::link($dns[$mac], $this->listing->getEntry($dns[$mac])->type).' - '.$error.'</li>'."\n";
        }
        $msg .= "</ul>\n";
      }
      msg_dialog::display(_('Ping results'), $msg, INFO_DIALOG);
    }
  }

  /*! \brief  Save event dialogs.
   *          And append the new Argonaut event.
   */
  function saveEventDialog ()
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
  function detectPostActions (): array
  {
    $action = parent::detectPostActions();
    if (isset($_POST['save_event_dialog'])) {
      $action['action'] = 'saveEvent';
    } elseif (isset($_POST['abort_event_dialog'])) {
      $action['action'] = 'cancel';
    }
    return $action;
  }

  function handleSubAction (array $action): bool
  {
    if (parent::handleSubAction($action)) {
      return TRUE;
    } elseif (preg_match('/^service_([a-zA-Z_]+)$/', $action['subaction'], $m)) {
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
}
