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

class argonautQueue extends simpleManagement
{
  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;

  protected $autoFilter       = FALSE;
  protected $autoActions      = FALSE;

  protected $skipCpHandler    = TRUE;
  protected $skipSnapHandler  = TRUE;

  var $read_only = FALSE;
  var $acl_base;
  protected $aclCategory = 'argonautQueue/';

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Deployment queue'),
      'plDescription' => _('Provide a mechanism to automatically activate systems'),
      'plIcon'        => 'geticon.php?context=applications&icon=argonaut&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 30,
      'plCategory'    => array('argonautQueue' => array('objectClass' => 'none', 'description' => _('Deployment queue'))),

      'plProvidedAcls' => array(
        'deploymentQueue' => _('Deployment queue')
      )
    );
  }

  function __construct()
  {
    global $config;
    $this->filterXMLPath  = get_template_path('deploy-filter.xml', TRUE, dirname(__FILE__));
    $this->listXMLPath    = get_template_path('deploy-list.xml', TRUE, dirname(__FILE__));

    parent::__construct();

    $this->o_queue  = new supportDaemon();
    $this->acl_base = $config->current['BASE'];
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();
    $this->registerAction('processNow', 'processNow');
    $this->registerAction('viewLogs',   'viewLogs');
    $this->registerAction('abort',      'abortEvent');
    $this->registerAction('import',     'importEvents');
    $this->registerAction('showError',  'showErrorEntry');

    $this->headpage->registerElementFilter('hostName',        'argonautQueue::filterHostName');
    $this->headpage->registerElementFilter('filterTask',      'argonautQueue::filterTask');
    $this->headpage->registerElementFilter('filterPeriod',    'argonautQueue::filterPeriod');
    $this->headpage->registerElementFilter('filterSchedule',  'argonautQueue::filterSchedule');
    $this->headpage->registerElementFilter('filterStatus',    'argonautQueue::filterStatus');
  }

  function importEvents()
  {
    $this->dialogObject = new argonautImportFile($this);
  }

  /*! \brief    Queue selected objects to be removed.
   *            Checks ACLs, Locks and ask for confirmation.
   */
  protected function removeEntryRequested($action, array $target, array $all)
  {
    if (!$this->acl_is_removeable()) {
      msg_dialog::display(_('Permission'), msgPool::permDelete(), ERROR_DIALOG);
      return;
    }
    $disallowed = array();
    $this->dns  = array();

    @DEBUG (DEBUG_SI, __LINE__, __FUNCTION__, __FILE__, $target, 'Entry removal requested!');

    // Check permissons for each target
    foreach ($target as $dn) {
      $tmp  = $this->headpage->getEntry($dn);
      $task = $tmp['EVENT'];

      /* Create a printable job name/description */
      $infos = argonautEventTypes::get_event_info($task['HEADERTAG']);
      if ($infos) {
        $j_name = $task['ID']." - ".$infos['name']."&nbsp;".$task['MACADDRESS'];
      } else {
        $j_name = $task['ID']." - ".$task['HEADERTAG']."&nbsp;".$task['MACADDRESS'];
      }

      /* Only remove WAITING or ERROR entries */
      if (in_array($task['STATUS'], array('waiting','error','processed')) ||
          ($task['STATUS'] == 'processing' && !preg_match('/install/', $task['HEADERTAG'])) ) {
        $this->dns[] = $dn;
      } else {
        $disallowed[] = $j_name;
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_('Remove'),
          sprintf(_('The following jobs couldn\'t be deleted, they have to be aborted: %s'),
            '<br/>'.msgPool::buildList($disallowed)), INFO_DIALOG);
    }

    // We've at least one entry to delete.
    if (count($this->dns)) {
      $objects = array();
      foreach ($this->dns as $dn) {
        $tmp  = $this->headpage->getEntry($dn);
        $task = $tmp['EVENT'];
        $infos = argonautEventTypes::get_event_info($task['HEADERTAG']);
        if ($infos) {
          $objects[] = array(
            'name'  => $infos['name'],
            'dn'    => $dn,
            'icon'  => $infos['img'],
            'type'  => 'task'
          );
        } else {
          $objects[] = array(
            'name'  => $task['HEADERTAG'],
            'dn'    => $dn,
            'icon'  => '',
            'type'  => 'task'
          );
        }
      }

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign('objects', $objects);
      $smarty->assign('multiple', TRUE);
      return $smarty->fetch(get_template_path('simple-remove.tpl'));
    }
  }

  function removeEntryConfirmed($action, array $target, array $all)
  {
    if ($this->acl_is_removeable()) {
      timezone::get_default_timezone();
      $ids = array();
      foreach ($this->dns as $dn) {
        $entry  = $this->headpage->getEntry($dn);
        $ids[]  = $entry['EVENT']['ID'];
      }
      $this->o_queue->remove_entries($ids);
    }
  }

  /*! \brief  Force queue job to be aborted.
   */
  function abortEvent($action, array $target)
  {
    $macs     = array();
    $taskids  = array();
    $headpage = $this->getHeadpage();
    foreach ($target as $id) {
      $tmp        = $headpage->getEntry($id);
      $macs[]     = $tmp['MACADDRESS'][0];
      $taskids[]  = $tmp['EVENT']['ID'];
    }

    $this->o_queue->append_call('wakeup', $macs, array());
    if ($this->o_queue->is_error()) {
      msg_dialog::display(_('Info'), sprintf(_('%s'), $this->o_queue->get_error()), INFO_DIALOG);
    }
    // Removing the task.
    $this->o_queue->remove_entries($taskids);
  }

  function processNow($action, array $target)
  {
    $this->execute_queue_entries($target);
  }

  function viewLogs($action, array $target)
  {
    if (count($target) == 1) {
      $entry        = $this->getHeadpage()->getEntry($target[0]);
      $attrsWrapper = new stdClass();
      $attrsWrapper->attrs = $entry['EVENT'];
      $this->dialogObject = new faiLogView(NULL, '', $attrsWrapper, $this);
    }
  }

  function showErrorEntry($action, array $target)
  {
    if (count($target) == 1) {
      $entry  = $this->headpage->getEntry($target[0]);
      $event  = $entry['EVENT'];
      msg_dialog::display(_('Error detail'), $event['ERROR']);
    }
  }

  function detectPostActions()
  {
    $action = parent::detectPostActions();
    if (isset($_POST['import_abort'])) {
      $action['action'] = 'cancel';
    }
    return $action;
  }

  protected function _getTabFooter()
  {
    if ($this->dialogObject instanceof faiLogView) {
      return  '<p class="plugbottom">'.
              ' <input type="submit" name="edit_cancel" value="'.msgPool::backButton().'"/>'.
              '</p>';
    } else {
      return parent::_getTabFooter();
    }
  }

  /*! \brief  Force queue job to be done as far as possible.
   *  @return Boolean TRUE in case of success, else FALSE.
   */
  private function execute_queue_entries($ids)
  {
    /* Only allow execution of paused or waiting entries */
    $update_ids = array();
    foreach ($ids as $id) {
      $tmp    = $this->headpage->getEntry($id);
      $entry  = $tmp['EVENT'];
      if (in_array($entry['STATUS'], array('paused','waiting'))) {
        $update_ids[] = $entry['ID'];
      }
    }

    /* Tell the daemon that we want to update some entries */
    if (count($update_ids)) {
      if (!$this->o_queue->process_entries_now($update_ids)) {
        msg_dialog::display(_('Error'), sprintf(_('Cannot update queue entries.')), ERROR_DIALOG);
        return FALSE;
      }
    }
    return TRUE;
  }

  private function acl_is_removeable()
  {
    if ($this->read_only) {
      return FALSE;
    }
    $ui = get_userinfo();
    return preg_match('/d/', $ui->get_permissions($this->acl_base, $this->aclCategory.get_class($this), '0'));
  }

  static function filterHostName($mac, $name = '')
  {
    if (isset($name[0]) && $name[0] != 'none') {
      return htmlentities($name[0].' ('.$mac[0].')', ENT_COMPAT, 'UTF-8');
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
    $infos = argonautEventTypes::get_event_info($tag);
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
    if ($stamp['0'] == '19700101000000') {
      return _('immediately');
    } else {
      return date('d.m.Y H:i:s', strtotime($stamp[0]));
    }
  }

  static function filterStatus($row, $status, $substatus)
  {
    $status     = $status[0];
    $substatus  = $substatus[0];

    if ($status == 'waiting') {
      $status = '<img class="center" src="geticon.php?context=status&icon=task-waiting&size=16" alt="clock"/>&nbsp;'._('Waiting');
    }
    if ($status == 'error') {
      $status = '<input class="center" type="image" src="geticon.php?context=status&icon=task-failure&size=16" title="'._('Show error').'" '.
                'name="listing_showError_'.$row.'" style="padding:1px"/>'._('Error');
    }
    if ($status == 'processed') {
      $status = '<img class="center" src="geticon.php?context=status&icon=task-complete&size=16" alt=""/>&nbsp;'._('Processed');
    }

    /* Special handling for all entries that have
       STATUS == "processing" && PROGRESS == NUMERIC
     */
    if ($status == 'processing' && $substatus) {
        $status = $substatus;
    } elseif ($status == 'processing') {
        $status = preg_replace('/ /', '&nbsp;', _('in progress'));
    }
    return $status;
  }
}
?>
