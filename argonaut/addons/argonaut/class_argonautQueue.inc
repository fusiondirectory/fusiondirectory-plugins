<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
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

class argonautQueue extends management
{
  protected $skipCpHandler     = TRUE;

  public static $skipSnapshots = TRUE;

  public static $skipTemplates = TRUE;

  public $o_queue;

  public $siActive = TRUE;

  /* Default columns */
  public static $columns = [
    ['ArgonautQueueColumn', ['attributes' => 'MACADDRESS',  'label' => 'Target']],
    ['ArgonautQueueColumn', ['attributes' => 'HEADERTAG',   'label' => 'Task']],
    ['ArgonautQueueColumn', ['attributes' => 'PERIODIC',    'label' => 'Period']],
    ['ArgonautQueueColumn', ['attributes' => 'TIMESTAMP',   'label' => 'Schedule']],
    ['ArgonautQueueColumn', ['attributes' => 'STATUS',      'label' => 'Status']],
    ['ActionsColumn',       ['label' => 'Actions']],
  ];

  static function plInfo ()
  {
    return [
      'plShortName'   => _('Deployment queue'),
      'plDescription' => _('Provide a mechanism to automatically activate systems'),
      'plIcon'        => 'geticon.php?context=applications&icon=argonaut&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 30,
      'plCategory'    => ['argonautQueue' => ['objectClass' => 'none', 'description' => _('Deployment queue')]],

      'plProvidedAcls' => [
        'deploymentQueue' => _('Deployment queue')
      ]
    ];
  }

  function __construct ()
  {
    parent::__construct([]);

    $this->o_queue = new supportDaemon();
  }

  protected function setUpListing ()
  {
    /* Set baseMode to FALSE */
    $this->listing  = new managementListing($this, FALSE);
  }

  protected function setUpFilter ()
  {
    $this->filter   = new argonautQueueFilter($this);
  }

  protected function configureActions ()
  {
    $this->registerAction(new HiddenAction('cancel',          'cancelEdit'));
    $this->registerAction(new HiddenAction('cancelDelete',    'cancelEdit'));
    $this->registerAction(new HiddenAction('removeConfirmed', 'removeConfirmed'));
    $this->registerAction(new HiddenAction('configure',       'configureDialog'));

    $this->registerAction(
      new Action(
        'import', _('Import'), 'geticon.php?context=actions&icon=document-import&size=16',
        '0', 'importEvents',
        ['argonautQueue/argonautQueue/c']
      )
    );

    $this->registerAction(
      new Action(
        'remove', _('Remove'), 'geticon.php?context=actions&icon=edit-delete&size=16',
        '+', 'removeRequested',
        ['argonautQueue/argonautQueue/d']
      )
    );

    $this->actions['remove']->setSeparator(TRUE);

    $this->registerAction(
      new Action(
        'abort', _('Abort'), 'geticon.php?context=actions&icon=process-stop&size=16',
        '+', 'abortEvent',
        ['argonautQueue/argonautQueue/deploymentQueue:w']
      )
    );

    $this->actions['abort']->setSeparator(TRUE);

    $this->registerAction(
      new Action(
        'processNow', _('Process now'), 'geticon.php?context=types&icon=action&size=16',
        '+', 'processNow',
        ['argonautQueue/argonautQueue/deploymentQueue:w']
      )
    );

    $this->registerAction(
      new Action(
        'retry', _('Retry'), 'geticon.php?context=actions&icon=view-refresh&size=16',
        '1', 'retryAction',
        ['argonautQueue/argonautQueue/deploymentQueue:w'],
        FALSE, TRUE
      )
    );

    $this->registerAction(
      new Action(
        'showError', '', '',
        '1', 'showErrorEntry',
        [],
        FALSE, FALSE
      )
    );

    $this->actions['processNow']->setEnableFunction([$this, 'enableProcessNow']);
    $this->actions['abort']->setEnableFunction([$this, 'enableAbort']);
    $this->actions['retry']->setEnableFunction([$this, 'enableRetry']);
    $this->actions['remove']->setEnableFunction([$this, 'enableRemove']);
  }

  function enableProcessNow ($action, ListingEntry $entry = NULL): bool
  {
    if ($entry === NULL) {
      return TRUE;
    } else {
      /* If PAUSED or WAITING add execution action */
      return in_array($entry['STATUS'], ['paused','waiting']);
    }
  }

  function enableAbort ($action, ListingEntry $entry = NULL): bool
  {
    if ($entry === NULL) {
      return TRUE;
    } else {
      /* If PROCESSING add abort action */
      return (in_array($entry['STATUS'], ['processing']) && preg_match('/install/', $entry['HEADERTAG']));
    }
  }

  function enableRetry ($action, ListingEntry $entry = NULL): bool
  {
    if ($entry === NULL) {
      return TRUE;
    } else {
      /* If ERROR add retry action */
      return in_array($entry['STATUS'], ['error']);
    }
  }

  function enableRemove ($action, ListingEntry $entry = NULL): bool
  {
    if ($entry === NULL) {
      return TRUE;
    } else {
      return (
        in_array($entry['STATUS'], ['waiting','error','processed']) ||
        (in_array($entry['STATUS'], ['processing']) && !preg_match('/install/', $entry['HEADERTAG']))
      );
    }
  }

  function importEvents (array $action)
  {
    $this->dialogObject = new argonautImportFile($this);
  }

  /*! \brief    Queue selected objects to be removed.
   *            Checks ACLs, Locks and ask for confirmation.
   */
  protected function removeEntryRequested (array $action)
  {
    $nodelete         = [];
    $disallowed       = [];
    $this->currentDns = [];

    @DEBUG(DEBUG_SI, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Entry removal requested!');

    // Check permissons for each target
    foreach ($action['targets'] as $dn) {
      $task = $this->listing->getEntry($dn);

      /* Create a printable job name/description */
      $infos = argonautEventTypes::get_event_info($task['HEADERTAG']);
      if ($infos) {
        $j_name = $task['ID']." - ".$infos['name']."&nbsp;".$task['MACADDRESS'];
      } else {
        $j_name = $task['ID']." - ".$task['HEADERTAG']."&nbsp;".$task['MACADDRESS'];
      }

      if ($this->acl_is_removeable($task['TARGETDN'])) {
        /* Only remove WAITING or ERROR entries */
        if (in_array($task['STATUS'], ['waiting','error','processed']) ||
            ($task['STATUS'] == 'processing' && !preg_match('/install/', $task['HEADERTAG']))) {
          $this->currentDns[] = $dn;
        } else {
          $nodelete[] = $j_name;
        }
      } else {
        $disallowed[] = $dn;
      }
    }
    if (count($nodelete)) {
      msg_dialog::display(_('Remove'),
          sprintf(_('The following jobs couldn\'t be deleted, they have to be aborted: %s'),
            '<br/>'.msgPool::buildList($nodelete)), INFO_DIALOG);
    }
    if (count($disallowed)) {
      msg_dialog::display(_('Permission'), msgPool::permDelete($disallowed), INFO_DIALOG);
    }

    // We've at least one entry to delete.
    if (count($this->currentDns)) {
      $objects = [];
      foreach ($this->currentDns as $dn) {
        $task = $this->listing->getEntry($dn);
        $infos = argonautEventTypes::get_event_info($task['HEADERTAG']);
        if ($infos) {
          $objects[] = [
            'name'  => $infos['name'],
            'dn'    => $dn,
            'icon'  => $infos['img'],
            'type'  => 'task'
          ];
        } else {
          $objects[] = [
            'name'  => $task['HEADERTAG'],
            'dn'    => $dn,
            'icon'  => '',
            'type'  => 'task'
          ];
        }
      }

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign('objects', $objects);
      $smarty->assign('multiple', TRUE);
      return $smarty->fetch(get_template_path('simple-remove.tpl'));
    }
  }

  function removeConfirmed (array $action)
  {
    timezone::get_default_timezone();
    $ids = [];
    foreach ($this->currentDns as $dn) {
      $entry  = $this->listing->getEntry($dn);
      if ($this->acl_is_removeable($entry['TARGETDN'])) {
        $ids[]  = $entry['ID'];
      }
    }
    $this->o_queue->remove_entries($ids);
  }

  /*! \brief  Force queue job to be aborted.
   */
  function abortEvent (array $action)
  {
    $macs     = [];
    $taskids  = [];
    foreach ($action['targets'] as $id) {
      $tmp        = $this->listing->getEntry($id);
      $macs[]     = $tmp['MACADDRESS'];
      $taskids[]  = $tmp['ID'];
    }

    $this->o_queue->append_call('wakeup', $macs, []);
    if ($this->o_queue->is_error()) {
      msg_dialog::display(_('Info'), sprintf(_('%s'), $this->o_queue->get_error()), INFO_DIALOG);
    }
    // Removing the task.
    $this->o_queue->remove_entries($taskids);
  }

  function processNow (array $action)
  {
    $this->execute_queue_entries($action['targets']);
  }

  function retryAction (array $action)
  {
    $entry = $this->listing->getEntry($action['targets'][0]);
    $this->dialogObject = new argonautAction($this, $entry['HEADERTAG'], [$entry['MACADDRESS']], FALSE);
  }

  function showErrorEntry (array $action)
  {
    if (count($action['targets']) == 1) {
      $entry  = $this->listing->getEntry($action['targets'][0]);
      msg_dialog::display(_('Error detail'), $entry['ERROR']);
    }
  }

  protected function _getTabFooter ()
  {
    if ($this->dialogObject instanceof faiLogView) {
      return  '<p class="plugbottom">'.
              ' <input type="submit" formnovalidate="formnovalidate" name="edit_cancel" value="'.msgPool::backButton().'"/>'.
              '</p>';
    } else {
      return parent::_getTabFooter();
    }
  }

  /*! \brief  Force queue job to be done as far as possible.
   *  @return Boolean TRUE in case of success, else FALSE.
   */
  private function execute_queue_entries (array $ids): bool
  {
    /* Only allow execution of paused or waiting entries */
    $update_ids = [];
    foreach ($ids as $id) {
      $entry  = $this->listing->getEntry($id);
      if (in_array($entry['STATUS'], ['paused','waiting'])) {
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

  private function acl_is_removeable (string $dn): bool
  {
    $ui = get_userinfo();
    return (strpos($ui->get_permissions($dn, 'argonautQueue/argonautQueue', '0'), 'd') !== FALSE);
  }
}
