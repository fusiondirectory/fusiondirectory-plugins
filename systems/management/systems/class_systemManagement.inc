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

  public $siActive = FALSE;

  public static $columns = [
    ['ObjectTypeColumn',     []],
    ['LinkColumn',           ['attributes' => 'nameAttr',     'label' => 'Name']],
    ['IpColumn',             ['attributes' => 'ipHostNumber', 'label' => 'IP']],
    ['Column',               ['attributes' => 'macAddress',   'label' => 'MAC']],
    ['LinkColumn',           ['attributes' => 'description',  'label' => 'Description']],
    ['SystemServicesColumn', ['label' => 'Services']],
    ['SystemReleaseColumn',  ['label' => 'Release']],
    ['ActionsColumn',        ['label' => 'Actions']],
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Systems'),
      'plTitle'       => _('Systems Management'),
      'plDescription' => _('Manage servers, computers, printers, phones and other devices'),
      'plIcon'        => 'geticon.php?context=devices&icon=computer&size=48',
      'plSection'     => ['systems' => ['name' => _('Systems'), 'priority' => 3]],
      'plPriority'    => 0,
      'plManages'     => ['server','workstation','terminal','printer','component','phone','mobilePhone'],
    ];
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
      $triggerActions   = [];
      $scheduleActions  = [];
      $events = argonautEventTypes::get_event_types();
      foreach ($events as $name => $infos) {
        $triggerActions[] = new Action(
          'trigger_'.$name, $infos['name'], $infos['img'],
          '*', 'handleEvent',
          ['argonautQueue/argonautAction/c']
        );
        $scheduleActions[] = new Action(
          'schedule_'.$name, $infos['name'], $infos['img'],
          '*', 'handleEvent',
          ['argonautQueue/argonautAction/c']
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

      /* Add ping action */
      $this->registerAction(
        new Action(
          'ping', _('Ping'), 'geticon.php?context=actions&icon=task-start&size=16',
          '+', 'pingSystems',
          ['argonautQueue/argonautAction/c']
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
    $mac = [];

    // Collect target mac addresses
    foreach ($action['targets'] as $dn) {
      $obj = $this->listing->getEntry($dn);
      if (isset($obj['macAddress'][0])) {
        $mac[] = $obj['macAddress'][0];
      } else {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('System %s has no mac address defined, cannot trigger action'),
            $dn
          ))
        );
        $error->display();
      }
    }

    if ((count($action['targets']) > 0) && (count($mac) == 0)) {
      return;
    }

    $o_queue  = new supportDaemon();

    /* Skip installation or update trigerred events,
     *  if this entry is currently processing.
     */
    if ($triggered && in_array($event, ['reinstall','update'])) {
      foreach ($mac as $key => $mac_address) {
        if ($o_queue->is_currently_installing($mac_address)) {
          $error = new FusionDirectoryError(
            htmlescape(sprintf(
              _('System %s is currently installing'),
              $mac_address
            ))
          );
          $error->display();
          unset($mac[$key]);
          logging::log('security', 'systems/'.get_class($this), '', [], 'Skip adding "argonautAction::'.$event.'" for mac "'.$mac_address.'", there is already a job in progress.');
        }
      }
    }

    if ((count($action['targets']) > 0) && (count($mac) == 0)) {
      return;
    }

    // Prepare event to be added
    $events   = argonautEventTypes::get_event_types();
    if (isset($events[$event])) {
      $this->dialogObject = new argonautAction($this, $event, $mac, !$triggered);

      if ($triggered && (count($mac) > 0)) {
        $res = $o_queue->append($this->dialogObject);
        if ($o_queue->is_error()) {
          $error = new FusionDirectoryError(msgPool::siError($o_queue->get_error()));
          $error->display();
        } else {
          if (is_array($res) && (count($res) > 1)) {
            msg_dialog::display(_('Action triggered'), htmlescape(sprintf(_('Action called without error (results were "%s")'), implode(', ', $res))), INFO_DIALOG);
          } else {
            if (is_array($res)) {
              $res = $res[0];
            }
            msg_dialog::display(_('Action triggered'), htmlescape(sprintf(_('Action called without error (result was "%s")'), $res)), INFO_DIALOG);
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

    $macs = [];

    // Collect target mac addresses
    foreach ($action['targets'] as $dn) {
      $obj = $this->listing->getEntry($dn);
      if (isset($obj['macAddress'][0])) {
        $macs[$dn] = $obj['macAddress'][0];
      } else {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('System %s has no mac address defined, cannot trigger action'),
            $dn
          ))
        );
        $error->display();
      }
    }

    if (count($macs) == 0) {
      return;
    }

    $o_queue  = new supportDaemon();

    $res = $o_queue->append_call('ping', array_values($macs), ['fullresult' => 1]);
    if ($o_queue->is_error()) {
      $error = new FusionDirectoryError(msgPool::siError($o_queue->get_error()));
      $error->display();
    } else {
      $dns = array_flip($macs);
      $html = '';
      if (!empty($res['results'])) {
        $html .= '<ul>'."\n";
        foreach ($res['results'] as $mac => $on) {
          $html .= '<li style="list-style-type:'.($on ? 'disc' : 'circle').';">'.objects::link($dns[$mac], $this->listing->getEntry($dns[$mac])->type).' - '.($on ? 'On' : 'Off').'</li>'."\n";
        }
        $html .= "</ul>\n";
      }
      if (!empty($res['errors'])) {
        $html .= '<ul style="list-style-type:square;">'."\n";
        foreach ($res['errors'] as $mac => $error) {
          $html .= '<li>'.objects::link($dns[$mac], $this->listing->getEntry($dns[$mac])->type).' - '.htmlescape($error).'</li>'."\n";
        }
        $html .= "</ul>\n";
      }
      msg_dialog::display(_('Ping results'), $html, INFO_DIALOG);
    }
  }

  function handleSubAction (array $action): bool
  {
    if (parent::handleSubAction($action)) {
      return TRUE;
    } elseif (preg_match('/^service_([a-zA-Z_]+)$/', $action['subaction'], $m)) {
      $service = $m[1];
      if (isset($this->tabObject->by_object['servicesManagement'])) {
        $this->tabObject->current = 'servicesManagement';
        $all = ['action'  => 'edit', 'targets' => [$service]];
        $this->tabObject->by_object['servicesManagement']->editEntry($all);
      } else {
        trigger_error('Unknown tab: servicesManagement');
      }
      return TRUE;
    }
    return FALSE;
  }
}
