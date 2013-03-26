<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class gotomasses extends management
{
  var $current        = FALSE;
  var $dialog         = FALSE;
  var $ids_to_remove  = array();

  var $events           = array();
  var $event_tags       = array();
  var $recently_removed = array();

  var $read_only = FALSE;
  var $acl_base;
  var $acl_category;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Deployment queue'),
      'plDescription' => _('Provide a mechanism to automatically activate systems'),
      'plIcon'        => 'plugins/argonaut/images/icon.png',
      'plSelfModify'  => FALSE,
      'plSection'     => 'admin',
      'plPriority'    => 10,
      'plCategory'    => array('gotomasses' => array('objectClass' => 'none', 'description' => _('Deployment queue'))),

      'plProvidedAcls' => array('Comment' => _('Description'))
    );
  }

  function __construct(&$config, $ui)
  {
    /* Include config object */
    $this->config   = &$config;
    $this->o_queue  = new supportDaemon();
    $this->events   = DaemonEvent::get_event_types();
    $this->acl_base = $config->current['BASE'];
    $this->acl_category = 'gotomasses/';

    /* Get tags that will be used in queue searches */
    $this->event_tags = array('none');
    foreach ($this->events as $action => $evt) {
      $this->event_tags[] = $action;
    }

    // Build filter
    if (session::global_is_set(get_class($this).'_filter')) {
      $filter = session::global_get(get_class($this).'_filter');
    } else {
      $filter = new filter(get_template_path('deploy-filter.xml', TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path('deploy-list.xml', TRUE));
    $headpage->registerElementFilter('hostName',        'gotomasses::filterHostName');
    $headpage->registerElementFilter('filterTask',      'gotomasses::filterTask');
    $headpage->registerElementFilter('filterPeriod',    'gotomasses::filterPeriod');
    $headpage->registerElementFilter('filterSchedule',  'gotomasses::filterSchedule');
    $headpage->registerElementFilter('filterStatus',    'gotomasses::filterStatus');
    $headpage->setFilter($filter);

    parent::__construct($config, $ui, 'Events', $headpage);

    $this->registerAction('processNow',       'processNow');
    $this->registerAction('viewLogs',         'viewLogs');
    $this->registerAction('abort',            'abortEvent');
    $this->registerAction('saveEventDialog',  'saveEventDialog');
    $this->registerAction('import',           'importEvents');

    $this->registerAction('showError', 'showErrorEntry');

    foreach ($this->events as $name => $infos) {
      $this->registerAction('new_'.$name, 'newEntry');
      $this->headpage->xmlData['actionmenu']['action'][0]['action'][] = array(
        'name' => 'new_'.$name,
        'type' => 'entry',
        'image' => $infos['img'],
        'label' => $infos['name'],
      );
    }
  }

  function newEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    $action = preg_replace("/^new_/","",$action);
    if ($this->acl_is_writeable("")) {
      if (isset($this->events[$action])) {
        $this->dialogObject = new DaemonEvent($this->config, $action);
        $this->dialogObject->set_timestamp_now();
      }
    }
  }

  function importEvents()
  {
    $this->dialogObject = new goto_import_file($this->config, $this);
  }

  static function filterHostName($mac, $name = "")
  {
    if (isset($name[0]) && $name[0] != "none") {
        // TODO ajouter lien machine.
      return htmlentities($name[0]." (".$mac[0].")", ENT_COMPAT, 'UTF-8');
    }
    return htmlentities($mac[0], ENT_COMPAT, 'UTF-8');
  }

  static function filterTask($tag, $progress)
  {
    $tag      = $tag[0];
    $progress = $progress[0];
    $str      = $tag;

    /* Check if this event exists as Daemon class
     * In this case, display a more accurate entry.
     */
    $infos = DaemonEvent::get_event_info($tag);
    if ($infos) {
      $str = $infos['name'];

      if (strlen($str) > 20) {
        $str = substr($str, 0, 18)."...";
      }

      $str = htmlentities($str, ENT_COMPAT, 'UTF-8');

      if (isset($infos['listimg']) && !empty($infos['listimg'])) {
        $str = $infos['listimg']."&nbsp;".$str;
      }
    }
    if ($progress) {
        $str .= "&nbsp;(".htmlentities($progress, ENT_COMPAT, 'UTF-8')."%)";
    }
    return $str;
  }

  static function filterPeriod($periodic = array())
  {
    $period = "&nbsp;-";
    if (isset($periodic[0]) && !preg_match("/none/i", $periodic[0])) {
      $tmp = explode("_", $periodic[0]);
      if (count($tmp) == 2) {
        $period = htmlentities($tmp[0], ENT_COMPAT, 'UTF-8')."&nbsp;".htmlentities(_($tmp[1]), ENT_COMPAT, 'UTF-8');
      }
    }
    return $period;
  }

  static function filterSchedule($stamp)
  {
    if ($stamp['0'] == "19700101000000") {
      return _("immediately");
    } else {
      return date("d.m.Y H:i:s", strtotime($stamp[0]));
    }
  }

  static function filterStatus($row, $status, $mac, $headertag, $error, $substatus)
  {
    $status     = $status[0];
    $mac        = $mac[0];
    $headertag  = $headertag[0];
    $error      = $error[0];
    $substatus  = $substatus[0];

    if ($status == "waiting") {
      $status = "<img class='center' src='plugins/systems/images/clock.png' alt=''>&nbsp;"._("Waiting");
    }
    if ($status == "error") {
      $status = "<input class='center' type='image' src='images/false.png' title='"._("Show error")."' ".
               "name='listing_showError_$row' style='padding:1px'>"._("Error");
    }
    if ($status == "processed") {
      $status = "<img class='center' src='images/true.png' alt=''>&nbsp;"._("Processed");
    }

    /* Special handling for all entries that have
       STATUS == "processing" && PROGRESS == NUMERIC
     */
    if ($status == "processing" && $substatus) {
        $status = $substatus;
    } else if ($status == "processing") {
        $status = preg_replace('/ /', '&nbsp;', _("in progress"));
    }
    return $status;
  }

  function editEntry($action = "", $target = array(), $all = array(), $altTabClass = "",
                      $altTabType = "", $altAclCategory = "")
  {
    if (count($target) == 1) {
      $headpage = $this->getHeadpage();
      $entry = $headpage->getEntry($target[0]);
      $event = $entry['EVENT'];
      if ($event['STATUS'] == "waiting" && isset($this->events[$event['HEADERTAG']])) {
        $this->dialogObject = new DaemonEvent($this->config, $event['HEADERTAG'], $event);
      }
    }
  }

  function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    if (!$this->acl_is_removeable()) {
      msg_dialog::display(_("Permission"), msgPool::permDelete(), ERROR_DIALOG);
    } else {

      $deleteable_jobs = array();
      $not_deleteable_jobs = array();
      $headpage = $this->getHeadpage();

      foreach ($target as $dn) {

        $tmp = $headpage->getEntry($dn);
        $task = $tmp['EVENT'];

        /* Create a printable job name/description */
        $infos = DaemonEvent::get_event_info($task['HEADERTAG']);
        if ($infos) {
          $j_name   = $task['ID']." - ".$infos['name']."&nbsp;".$task['MACADDRESS'];
        } else {
          $j_name = $task['ID']." - ".$task['HEADERTAG']."&nbsp;".$task['MACADDRESS'];
        }

        /* Only remove WAITING or ERROR entries */
        if (in_array($task['STATUS'], array("waiting","error","processed")) ||
            ($task['STATUS'] == "processing" && !preg_match("/install/", $task['HEADERTAG'])) ) {
          $this->ids_to_remove[]  = $task['ID'];
          $deleteable_jobs[]      = $j_name;
        } else {
          $not_deleteable_jobs[] = $j_name;
        }
      }
      if (count($not_deleteable_jobs)) {
        msg_dialog::display(_("Remove"),
            sprintf(_("The following jobs couldn't be deleted, they have to be aborted: %s"),
              "<br>".msgPool::buildList($not_deleteable_jobs)), INFO_DIALOG);
      }

      if (count($this->ids_to_remove)) {
        $smarty = get_smarty();
        $smarty->assign("multiple", TRUE);
        $smarty->assign("info", msgPool::deleteInfo($deleteable_jobs));
        return $smarty->fetch(get_template_path('remove.tpl', TRUE));
      }
    }
  }

  function removeEntryConfirmed($action = "", $target = array(), $all = array(),
                                $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    if ($this->acl_is_removeable("")) {
      timezone::get_default_timezone();
      $this->o_queue->remove_entries($this->ids_to_remove);
      $this->save();
    }
  }


  /*! \brief  Force queue job to be aborted.
   */
  function abortEvent($action = "", $target = array(), $all = array())
  {
    /* Detect if the ids we got are valid and
     *  check if the status allows pausing.
     */
    $macs = array();
    $taskids = array();
    $headpage = $this->getHeadpage();
    foreach ($target as $id) {
      $tmp = $headpage->getEntry($id);
      $macs[] = $tmp['MACADDRESS'][0];
      $taskids[] = $tmp['EVENT']['ID'];
    }

    if (class_available("DaemonEvent")) { // Sending a wakeup in order to flag it as localboot
      $tmp = new DaemonEvent($this->config, 'wakeup');
      $tmp->add_targets($macs);
      $tmp->set_type(TRIGGERED_EVENT);
      $this->recently_removed = $macs;
      $o_queue->append($tmp);
      if ($o_queue->is_error()) {
        msg_dialog::display(_("Info"), sprintf(_("%s"), $this->o_queue->get_error()), INFO_DIALOG);
      }
    } else {
      msg_dialog::display(_("Error"),
          sprintf(_("Required class '%s' cannot be found: job not aborted!"),
            "DaemonEvent"), ERROR_DIALOG);
    }

    $this->o_queue->remove_entries($taskids); // Removing the task.
  }

  function processNow($action = "", $target = array(), $all = array())
  {
    $this->execute_queue_entries($target);
  }

  function viewLogs($action = "", $target = array(), $all = array())
  {
    if (count($target) == 1) {
      $id       = $target[0];
      $type     = FALSE;
      $headpage = $this->getHeadpage();
      $tmp      = $headpage->getEntry($id);
      $entry    = $tmp['EVENT'];
      $this->dialogObject = new gotoLogView($this->config, "", $entry, $this);
    }
  }

  function showErrorEntry($action = "", $target = array(), $all = array())
  {
    if (count($target) == 1) {
      $id       = $target[0];
      $type     = FALSE;
      $headpage = $this->getHeadpage();
      $tmp      = $headpage->getEntry($id);
      $entry    = $tmp['EVENT'];
      msg_dialog::display(_("Error detail"), $entry['ERROR']);
    }
  }

  function saveEventDialog()
  {
    if (is_object($this->dialogObject)) {
      $this->dialogObject->save_object();
      $this->o_queue->append($this->dialogObject);
      if ($this->o_queue->is_error()) {
        msg_dialog::display(_("Service infrastructure"), msgPool::siError($this->o_queue->get_error()), ERROR_DIALOG);
      } else {
        $this->current = -1;
      }
    }
    $this->closeDialogs();
  }

  function detectPostActions()
  {
    $action = management::detectPostActions();
    if (isset($_POST['save_event_dialog'])) $action['action'] = "saveEventDialog";
    if (isset($_POST['abort_event_dialog'])) $action['action'] = "cancel";
    if (isset($_POST['delete_multiple_confirm'])) $action['action'] = "removeConfirmed";
    if (isset($_POST['delete_cancel'])) $action['action'] = "cancel";
    if (isset($_POST['import_abort'])) $action['action'] = "cancel";
    return $action;
  }


  function closeDialogs()
  {
    $this->current = FALSE;
    management::closeDialogs();
  }


  /*! \brief  Force queue job to be done as far as possible.
   *  @return Boolean TRUE in case of success, else FALSE.
   */
  private function execute_queue_entries($ids)
  {
    /* Only allow execution of paused or waiting entries
     */
    $update_ids = array();
    $headpage   = $this->getHeadpage();
    foreach ($ids as $id) {
      $tmp    = $headpage->getEntry($id);
      $entry  = $tmp['EVENT'];
      if (in_array($entry['STATUS'], array("paused","waiting"))) {
        $update_ids[] = $entry['ID'];
      }
    }

    /* Tell the daemon that we want to update some entries
     */
    if (count($update_ids)) {
      if (!$this->o_queue->process_entries_now($update_ids)) {
        msg_dialog::display(_("Error"), sprintf(_("Cannot update queue entries.")), ERROR_DIALOG);
        return FALSE;
      }
    }
    return TRUE;
  }

  function save_object()
  {
  }

  function save()
  {
  }


  function set_acl_base($base)
  {
    $this->acl_base = $base;
  }


  function set_acl_category($category)
  {
    $this->acl_category = "$category/";
  }


  function acl_is_writeable($attribute, $skip_write = FALSE)
  {
    if ($this->read_only) {
      return FALSE;
    }
    $ui = get_userinfo();
    return preg_match('/w/', $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute, $skip_write));
  }


  function acl_is_readable($attribute)
  {
    $ui = get_userinfo();
    return preg_match('/r/', $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute));
  }

  private function _acl_is_ ($mask, $base)
  {
    if ($this->read_only) {
      return FALSE;
    }
    $ui = get_userinfo();
    if ($base == "") {
      $base = $this->acl_base;
    }
    return preg_match($mask, $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  function acl_is_createable($base = "")
  {
    return $this->_acl_is_('/c/', $base);
  }

  function acl_is_removeable($base = "")
  {
    return $this->_acl_is_('/d/', $base);
  }

  function acl_is_moveable($base = "")
  {
    return $this->_acl_is_('/m/', $base);
  }


  function getacl($attribute, $skip_write = FALSE)
  {
    $ui = get_userinfo();
    $skip_write |= $this->read_only;
    return $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute, $skip_write);
  }
}
?>
