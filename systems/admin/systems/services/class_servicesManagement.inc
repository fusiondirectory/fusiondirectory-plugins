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

class servicesManagement extends management implements simpleTab
{
  static public $actionStatus = [
    'start'   => 'running',
    'stop'    => 'stopped',
    'restart' => 'running',
    'status'  => '',
  ];

  protected $skipCpHandler     = TRUE;

  public static $skipSnapshots = TRUE;

  public static $skipTemplates = TRUE;

  protected $skipConfiguration = TRUE;

  /* Vars needed for simpleTabs */
  public $is_account      = TRUE;
  public $ignore_account  = TRUE;
  public $dn;
  public $is_template     = FALSE;
  public $parent          = NULL;
  public $acl_base        = '';
  public $acl_category    = '';

  protected $plugins      = [];
  protected $current      = '';
  protected $dialog       = FALSE;
  protected $backup       = NULL;

  var $read_only      = FALSE;
  var $acl;
  var $cn;

  /* Default columns */
  public static $columns = [
    ['Column',        ['attributes' => 'Status',  'label' => '!']],
    ['LinkColumn',    ['attributes' => 'Message', 'label' => 'Description']],
    ['ActionsColumn', ['label' => 'Actions']],
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Services'),
      'plDescription' => _('Server services'),
      'plIcon'        => '',
      'plObjectType'  => ['server'],
      'plPrority'     => 5,

      'plProvidedAcls'  => []
    ];
  }

  function __construct (string $dn, $object, $parent = NULL)
  {
    global $config;

    $this->dn     = $dn;
    if (isset($object->attrs)) {
      $this->attrs = $object->attrs;
    }
    $this->parent = $parent;

    /* Initialize acl base */
    $this->set_acl_base($this->dn);

    // Initialize list of used and useable services.
    foreach ($config->data['TABS']['SERVERSERVICE'] as $plug) {
      if (class_available($plug['CLASS'])) {
        $name = $plug['CLASS'];

        $this->plugins[$name] = new $name($dn, $this);
      }
    }

    uasort($this->plugins,
      function($a, $b)
      {
        return strcmp($a->DisplayName ?? get_class($a), $b->DisplayName ?? get_class($b));
      }
    );

    parent::__construct([]);
  }

  protected function setUpListing ()
  {
    /* Set baseMode to FALSE */
    $this->listing  = new managementListing($this, FALSE);
  }

  protected function setUpFilter ()
  {
    $this->filter   = new servicesManagementFilter($this);
  }

  protected function configureActions ()
  {
    $this->updateActionMenu();

    $this->registerAction(
      new Action(
        'remove', _('Remove'), 'geticon.php?context=actions&icon=edit-delete&size=16',
        '+', 'removeService',
        []
      )
    );

    $this->actions['remove']->setSeparator(TRUE);

    // Add export actions
    $exportMenu = [];
    foreach ($this->exporters as $action => $exporter) {
      $exportMenu[] = new Action(
        $action, $exporter['label'], $exporter['image'],
        '0', 'export'
      );
    }
    $this->registerAction(
      new SubMenuAction(
        'export', _('Export list'), 'geticon.php?context=actions&icon=document-export&size=16',
        $exportMenu
      )
    );

    $this->actions['export']->setSeparator(TRUE);

    $this->registerAction(
      new Action(
        'status', _('Get status'), 'geticon.php?context=actions&icon=view-refresh&size=16',
        '1', 'updateServiceStatus',
        [],
        FALSE, TRUE
      )
    );

    $this->registerAction(
      new Action(
        'start', _('Start'), 'geticon.php?context=actions&icon=task-start&size=16',
        '1', 'updateServiceStatus',
        [],
        FALSE, TRUE
      )
    );

    $this->registerAction(
      new Action(
        'stop', _('Stop'), 'geticon.php?context=actions&icon=task-stop&size=16',
        '1', 'updateServiceStatus',
        [],
        FALSE, TRUE
      )
    );

    $this->registerAction(
      new Action(
        'restart', _('Restart'), 'geticon.php?context=actions&icon=view-refresh&size=16',
        '1', 'updateServiceStatus',
        [],
        FALSE, TRUE
      )
    );

    $this->registerAction(
      new Action(
        'edit', _('Edit service'), 'geticon.php?context=actions&icon=document-edit&size=16',
        '1', 'editEntry',
        [],
        FALSE, TRUE
      )
    );

    $this->registerAction(new HiddenAction('saveService',   'saveService'));
    $this->registerAction(new HiddenAction('cancelService', 'cancelEdit'));
  }

  protected function updateActionMenu ()
  {
    $acl_category = (empty($this->acl_category) ? 'server' : $this->acl_category);
    $createMenu   = [];
    $available_services = $this->getAllUnusedServices();
    foreach ($available_services as $class => $label) {
      $infos = pluglist::pluginInfos($class);
      $createMenu[] = new Action(
        'new_'.$class, $label, ($infos['plIcon'] ?? 'geticon.php?context=actions&icon=document-new&size=16'),
        '0', 'newService',
        [$acl_category.'/'.$class.'/c']
      );
    }
    $this->registerAction(
      new SubMenuAction(
        'new', _('Create'), 'geticon.php?context=actions&icon=document-new&size=16',
        $createMenu
      )
    );
  }

  /*! \brief    Filter extra POST and GET variables for this plugin.
   */
  function detectPostActions (): array
  {
    $action = parent::detectPostActions();
    if (isset($_POST['SaveService'])) {
      $action['action'] = 'saveService';
    } elseif (isset($_POST['CancelService'])) {
      $action['action'] = 'cancelService';
    }
    return $action;
  }

  /*! \brief  Edit an existing service here.
   *          Somebody clicked on the paper and pencil icon.
   */
  function editEntry (array $action)
  {
    $s_entry = array_pop($action['targets']);
    if ($this->plugins[$s_entry]->acl_is_readable('')) {
      $this->backup       = get_object_vars($this->plugins[$s_entry]);
      $this->dialogObject = $this->plugins[$s_entry];
      $this->dialog       = TRUE;
      $this->current      = $s_entry;
    }
  }

  /*! \brief  Editing an object was caneled.
   *          Close dialogs/tabs and remove locks.
   */
  function cancelEdit ()
  {
    if (($this->backup == NULL) && $this->current) {
      $this->plugins[$this->current] = new $this->current($this->dn, $this);
      $this->plugins[$this->current]->set_acl_base($this->acl_base);
      $this->plugins[$this->current]->set_acl_category(preg_replace('/\/$/', '', $this->acl_category));
    } elseif (is_array($this->backup)) {
      foreach ($this->backup as $name => $value) {
        $this->plugins[$this->current]->$name = $value;
      }
    }
    $this->backup   = NULL;
    $this->current  = '';
    $this->closeDialogs();
  }

  /*! \brief  Let the user create a new service
   */
  function newService (array $action)
  {
    $this->closeDialogs();
    $serv = preg_replace('/^new_/', '', $action['action']);
    $this->plugins[$serv]->is_account = TRUE;
    $this->dialogObject               = $this->plugins[$serv];
    $this->current                    = $serv;
    $this->dialog                     = TRUE;
  }

  /*! \brief  Save the currently edited service.
   */
  function saveService (array $action)
  {
    $this->dialogObject->save_object();
    $msgs = $this->dialogObject->check();
    if (count($msgs)) {
      foreach ($msgs as $msg) {
        msg_dialog::display(_('Error'), $msg, ERROR_DIALOG);
      }
    } else {
      $this->plugins[$this->current] = $this->dialogObject;
      $this->closeDialogs();
      $this->backup = NULL;
      $this->updateActionMenu();
    }
  }

  /*!\brief   Close all opened dialogs
   *          And reset "dialog open" flags to display bottom buttons again.
   */
  function closeDialogs ()
  {
    parent::closeDialogs();
    $this->dialog = FALSE;
    set_object_info($this->dn);
  }

  /*! \brief    Remove the selected service(s)
   */
  function removeService (array $action)
  {
    foreach ($action['targets'] as $s_entry) {
      $new_obj = new $s_entry($this->dn, $this);
      $new_obj->set_acl_base($this->acl_base);
      $new_obj->set_acl_category(preg_replace('/\/$/', '', $this->acl_category));
      $tmp = $new_obj->getListEntry();

      /* Check if we are allowed to remove this service */
      if ($tmp['AllowRemove']) {
        $this->plugins[$s_entry]              = $new_obj;
        $this->plugins[$s_entry]->is_account  = FALSE;
      }
    }
  }

  function updateServicesVars ($service)
  {
    foreach (['cn','dn'] as $var) {
      if (isset($this->$var)) {
        $this->plugins[$service]->$var = $this->$var;
      }
    }
  }

  /*! \brief    Updates the status for a list of services.
   */
  function updateServiceStatus (array $action)
  {
    /* Skip if this is a new server */
    if ($this->dn == 'new') {
      msg_dialog::display(_('Information'), _('Cannot update service status until it has been saved!'), INFO_DIALOG);
      return;
    }

    /* Is this an existing action */
    if (!isset(static::$actionStatus[$action['action']])) {
      msg_dialog::display(_('Error'), sprintf(_('Unknown action "%s"'), $action['action']), ERROR_DIALOG);
      return;
    }

    /* Handle state changes for services */
    foreach ($action['targets'] as $service) {
      $this->updateSingleServiceStatus($action['action'], $service);
    }
  }

  /*! \brief    Updates the status of a service and
   *             calls an external hook if specified in fusiondirectory.conf
   */
  private function updateSingleServiceStatus (string $action, string $service)
  {
    if ($this->plugins[$service]->is_account) {
      $this->updateServicesVars($service);

      $s_daemon = new supportDaemon();
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          sprintf(_('Could not get execute action %s on service %s.'), $action, $service),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
      } else {
        $target = $this->parent->getBaseObject()->macAddress;
        if (empty($target)) {
          msg_dialog::display(
            sprintf(_('Could not get execute action %s on service %s.'), $action, $service),
            _('This server has no mac address.'), ERROR_DIALOG
          );
          return;
        }
        if (is_array($target)) {
          $target = $target[0];
        }
        if ($action == 'status') {
          $res = $s_daemon->append_call('Service.is_running', $target, ['args' => [$service]]);

          if ($s_daemon->is_error()) {
            msg_dialog::display(
              sprintf(_('Could not get execute action %s on service %s.'), $action, $service),
              msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
            );
          } else {
            $this->plugins[$service]->setStatus($res == 'yes' ? 'running' : 'stopped');
          }
        } else {
          $res = $s_daemon->append_call('Service.manage', $target, ['args' => [$service, $action]]);

          if ($s_daemon->is_error()) {
            msg_dialog::display(
              sprintf(_('Could not get execute action %s on service %s.'), $action, $service),
              msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
            );
          } elseif (preg_match('/^done/', $res)) {
            $this->plugins[$service]->setStatus(static::$actionStatus[$action]);
          }
        }
        return $res;
      }
    }
  }

  /*! \brief   Returns a list of all used services
   *            CLASSNAME => _($this->plugins[*]->DisplayName);
   */
  function getAllUsedServices (): array
  {
    $ret = [];
    foreach ($this->plugins as $name => $obj) {
      if ($obj->is_account) {
        $ret[$name] = ($obj->DisplayName ?? $name);
      }
    }
    return $ret;
  }

  /*! \brief    Returns a list of all unused services.
   */
  function getAllUnusedServices (): array
  {
    $tmp = $this->getAllUsedServices();
    $pool_of_ocs = [];
    foreach ($tmp as $name => $value) {
      $pool_of_ocs[] = get_class($this->plugins[$name]);
      if (isset($this->plugins[$name]->conflicts)) {
        $pool_of_ocs = array_merge($pool_of_ocs, $this->plugins[$name]->conflicts);
      }
    }

    $ret = [];
    foreach ($this->plugins as $name => $obj) {
      if (!$obj->acl_is_createable()) {
        continue;
      }

      /* Skip all pluigns that will lead into conflicts */
      $conflicts = ($obj->conflicts ?? []);
      $conflicts[] = get_class($obj);
      if (count(array_uintersect($conflicts, $pool_of_ocs, 'strcasecmp'))) {
        continue;
      }

      $ret[$name] = ($obj->DisplayName ?? $name);
    }
    return $ret;
  }

  /*! \brief    Returns the services list.
   *
   * Used in the filter class for services class_servicesManagementFilter.inc
   */
  public function getServiceList ()
  {
    return $this->plugins;
  }

  /*! \brief    No checks here.
   */
  public function check (): array
  {
    return [];
  }

  /*! \brief    Keep posted form values in opened dialogs
   */
  function save_object ()
  {
    // save_object of the dialog is called in management::execute
  }

  /*! \brief Remove all active services
   */
  public function remove (bool $fulldelete = FALSE): array
  {
    $errors = [];
    foreach ($this->plugins as $name => $obj) {
      $this->updateServicesVars($name);
      if ($this->plugins[$name]->initially_was_account) {
        $result = $this->plugins[$name]->remove($fulldelete);
        if (!empty($result)) {
          $errors = array_merge($errors, $result);
        }
      }
    }
    return $errors;
  }

  /*! \brief    Save all active services
   */
  public function save (): array
  {
    $errors = [];
    foreach ($this->plugins as $name => $obj) {
      $this->updateServicesVars($name);

      if ($this->plugins[$name]->is_account) {
        $result = $this->plugins[$name]->save();
      } elseif ($this->plugins[$name]->initially_was_account) {
        $result = $this->plugins[$name]->remove(FALSE);
      }
      if (!empty($result)) {
        $errors = array_merge($errors, $result);
      }
    }
    return $errors;
  }

  /*! \brief    Prepare active services to be copied.
   */
  public function resetCopyInfos ()
  {
    $this->dn = 'new';
    foreach ($this->plugins as $obj) {
      $obj->resetCopyInfos();
    }
  }

  /*! \brief    Forward plugin acls
   */
  public function set_acl_base (string $base)
  {
    $this->acl_base = $base;
    foreach ($this->plugins as $obj) {
      $obj->set_acl_base($base);
    }
  }

  /*! \brief    Forward plugin acls
   */
  public function set_acl_category (string $category)
  {
    $this->acl_category = $category;
    foreach ($this->plugins as $obj) {
      $obj->set_acl_category($category);
    }
  }

  public function setTemplate (bool $bool)
  {
    foreach ($this->plugins as &$plugin) {
      $plugin->setTemplate($bool);
    }
    unset($plugin);
  }

  public function getRequiredAttributes (): array
  {
    return [];
  }

  public function adapt_from_template (array $attrs, array $skip = [])
  {
    foreach ($this->plugins as $obj) {
      $obj->adapt_from_template($attrs, $skip);
    }
  }

  /*!
   * \brief Can we delete the object
   *
   * Only used on main tab
   *
   * \param string $base
   */
  public function acl_is_removeable (string $base = NULL): bool
  {
    foreach ($this->plugins as $name => $obj) {
      $this->updateServicesVars($name);
      if ($obj->initially_was_account && !$obj->acl_is_removeable($base)) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /*!
   * \brief Sets whether the opened objet has an edit button
   *
   * \param bool $needEditMode
   */
  public function setNeedEditMode (bool $needEditMode)
  {
  }

  /*!
   * \brief Is there a modal dialog opened
   */
  public function is_modal_dialog (): bool
  {
    return $this->dialogOpened();
  }

  /*!
   * \brief Returns TRUE if this attribute should be asked in the creation by template dialog
   */
  public function showInTemplate (string $attr, array $templateAttrs): bool
  {
    return FALSE;
  }

  /*!
   * \brief Deserialize values
   */
  public function deserializeValues (array $values, bool $checkAcl = TRUE)
  {
  }

  /*!
   * \brief Get the acl permissions for an attribute or the plugin itself
   */
  public function aclGetPermissions ($attribute = '0', string $base = NULL, bool $skipWrite = FALSE): string
  {
    return '';
  }

  /*!
   * \brief Merge in objectClasses needed by this tab
   *
   *  Used by prepare_save and template::apply
   */
  public function mergeObjectClasses (array $oc): array
  {
    foreach ($this->plugins as $plugin) {
      if ($plugin->is_account || $plugin->ignore_account) {
        $oc = $plugin->mergeObjectClasses($oc);
      }
    }
    return $oc;
  }

  /*! \brief    Returns an image for the service status.
   *            Used in the service list class.
   */
  static function filterServiceStatus ($a, $b, $c, $d)
  {
    $img = '';
    if (isset($d['0'])) {
      switch ($d[0]) {
        case 'stopped':
          $img = 'geticon.php?context=status&icon=task-stopped&size=16';
          break;
        case 'running':
          $img = 'geticon.php?context=status&icon=task-running&size=16';
          break;
      }
    }
    if (!empty($img)) {
      return '<img src="'.$img.'"/>';
    }
    return '&nbsp;';
  }
}
