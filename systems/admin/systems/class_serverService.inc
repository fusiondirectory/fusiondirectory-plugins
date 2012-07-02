<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
define("SERVICE_STOPPED",   _("Stop"));
define("SERVICE_STARTED",   _("Start"));
define("SERVICE_RESTARTED", _("Restart"));

class ServerService extends management
{
  static public $actionStatus = array(
    "start"   => SERVICE_STARTED,
    "stop"    => SERVICE_STOPPED,
    "restart" => SERVICE_RESTARTED,
  );

  /* attribute list for save action */
  var $ignore_account   = TRUE;
  var $skipFooter       = TRUE;
  var $skipHeader       = TRUE;
  var $plugins          = array();
  var $pluign_names     = array();
  var $current          = "";
  var $dialog           = FALSE;
  var $read_only        = FALSE;
  var $backup           = NULL;
  var $acl;
  var $cn;
  var $parent;
  var $pl_notify        = FALSE;
  var $acl_category     = "";
  var $acl_base         = "";

  var $is_account;

  function __construct (&$config, $dn, $parent)
  {
    $this->config = $config;
    $this->parent = $parent;
    $this->ui     = get_userinfo();
    $this->dn     = $dn;
    if (isset($this->parent->attrs)) {
      $this->attrs = $this->parent->attrs;
    }

    /* Initialize acl base */
    $this->set_acl_base($this->dn);

    // Set storage points - We do not have any - We just create a fake list which lists all services
    $this->storagePoints = array("");

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("serverService-filter.xml", TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("serverService-list.xml", TRUE));
    $headpage->registerElementFilter("filterServiceStatus", "ServerService::filterServiceStatus");
    $headpage->setFilter($filter);


    // Initialize list of used and useable services.
    foreach ($config->data['TABS']['SERVERSERVICE'] as $plug) {
      if (class_available($plug['CLASS'])) {
        $name = $plug['CLASS'];

        $this->plugin_names[] = $name;
        $this->plugins[$name] = new $name($config, $dn, $this);

        /* Initialize kerberos key support */
        if (isset($this->plugins[$name]->krb_service_prefix) && !empty($this->plugins[$name]->krb_service_prefix)) {
          $this->plugins[$name]->krb_host_key = &$this->parent->kerberos_key_service;
        }

        /* Capture all service objectClases, necessary for acl handling */
        if (isset($this->plugins[$name]->objectclasses)) {
          foreach ($this->plugins[$name]->objectclasses as $oc) {
            $this->objectclasses[] = $oc;
          }
        }
      }
    }

    parent::__construct($config, $this->ui, "services", $headpage);
    $this->registerAction("new",                "newService");
    $this->registerAction("remove",             "removeService");
    $this->registerAction("newServiceSelected", "newServiceSelected");
    $this->registerAction("saveService",        "saveService");
    $this->registerAction("cancelService",      "cancelEdit");
    $this->registerAction("newServiceCancel",   "closeDialogs");
    $this->registerAction("restart",            "updateServiceStatus");
    $this->registerAction("stop",               "updateServiceStatus");
    $this->registerAction("start",              "updateServiceStatus");
  }


  /*! \brief    Filter extra POST and GET variables for this plugin.
   */
  function detectPostActions()
  {
    $action = management::detectPostActions();
    if (isset($_POST['SaveServiceAdd'])) {
      $action['action'] = "newServiceSelected";
    } elseif (isset($_POST['CancelServiceAdd'])) {
      $action['action'] = "newServiceCancel";
    } elseif (isset($_POST['SaveService'])) {
      $action['action'] = "saveService";
    } elseif (isset($_POST['CancelService'])) {
      $action['action'] = "cancelService";
    }
    return $action;
  }


  /*! \brief  Edit an existing service here.
   *          Somebody clicked on the paper and pencil icon.
   */
  function editEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    $this->dialog = TRUE;
    $s_entry = array_pop($target);
    if ($this->plugins[$s_entry]->acl_is_readable("")) {
      $this->backup       = get_object_vars($this->plugins[$s_entry]);
      $this->dialogObject = $this->plugins[$s_entry];
      $this->current      = $s_entry;
    }
  }


  /*! \brief  Editing an object was caneled.
   *          Close dialogs/tabs and remove locks.
   */
  protected function cancelEdit()
  {
    if ($this->backup == NULL && $this->current) {
      $this->plugins[$this->current] = new $this->current($this->config, $this->dn, $this);
      $this->plugins[$this->current]->set_acl_base($this->acl_base);
      $this->plugins[$this->current]->set_acl_category(preg_replace("/\/$/", "", $this->acl_category));

    } elseif (is_array($this->backup)) {
      foreach ($this->backup as $name => $value) {
        $this->plugins[$this->current]->$name = $value;
      }
    }
    $this->backup   = NULL;
    $this->current  = "";
    $this->closeDialogs();
  }


  /*! \brief  Let the user choose the new service to create.
   */
  function newService()
  {
    $this->dialog = TRUE;
    $this->dialogObject = new ServiceAddDialog($this->config, $this->dn, $this);
  }


  /*! \brief  Create the selected service.
   */
  function newServiceSelected()
  {
    $this->closeDialogs();
    $serv = $_POST['ServiceName'];

    $this->plugins[$serv]->is_account = TRUE;
    $this->dialogObject               = $this->plugins[$serv];
    $this->current                    = $serv;
    $this->dialog                     = TRUE;
  }


  /*! \brief  Save the currently edited service.
   */
  function saveService()
  {
    $this->dialogObject->save_object();
    $msgs = $this->dialogObject->check();
    if (count($msgs)) {
      foreach ($msgs as $msg) {
        msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
      }
    } else {
      $this->plugins[$this->current] = $this->dialogObject;
      $tmp  = get_object_vars($this->dialogObject);
      foreach ($tmp as $name => $value) {
        $this->plugins[$this->current]->$name = $value;
      }
      $this->closeDialogs();
      $this->backup = NULL;
    }
  }


  /*!\brief   Close all opened dialogs
   *          And reset "dialog open" flags to display bottom buttons again.
   */
  function closeDialogs()
  {
    management::closeDialogs();
    $this->dialog = FALSE;
  }


  /*! \brief    Remove the selected service(s)
   */
  protected function removeService($action = "", $target = array(), $all = array())
  {
    foreach ($target as $s_entry) {
      $new_obj = new $s_entry($this->config,$this->dn, $this);
      $new_obj->set_acl_base($this->acl_base);
      $new_obj->set_acl_category(preg_replace("/\/$/", "", $this->acl_category));
      $tmp = $new_obj->getListEntry();

      /* Check if we are allowed to remove this service
       */
      if ($tmp['AllowRemove']) {
        $str = $this->plugins[$s_entry]->allow_remove();
        if (empty($str)) {
          $this->plugins[$s_entry]              = $new_obj;
          $this->plugins[$s_entry]->is_account  = FALSE;
        } else {
          msg_dialog::display(_("Error"), $str, ERROR_DIALOG);
        }
      }
    }
  }


  /*! \brief    Returns the list of of services, active services.
   *            Used in the filter class for services.
   *             class_filterServerService.inc
   */
  static function getServiceList()
  {
    return session::get('ServerService');
  }


  /*! \brief    Returns an image for the service status.
   *            Used in the service list class.
   */
  static function filterServiceStatus($a, $b, $c, $d)
  {
    $img = "";
    if (isset($d['0'])) {
      switch ($d[0]) {
        case 'Restart':
          $img = ('images/status_restarting.png');
          break;
        case 'Stop':
          $img = ('images/lists/off.png');
          break;
        case 'Start':
          $img = ('images/lists/on.png');
          break;
      }
    }
    if (!empty($img)) {
      return '<img src=\''.$img.'\'>';
    }
    return "&nbsp;";
  }


  /*! \brief  Execute this plugin
   *          Handle actions/events, locking, snapshots, dialogs, tabs,...
   */
  function execute()
  {
    // Ensure that html posts and gets are kept even if we see a 'Entry islocked' dialog.
    $vars = array('/^act$/','/^listing/','/^PID$/','/^FILTER_PID$/');
    session::set('LOCK_VARS_TO_USE', $vars);

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandler("", array());
    if ($ret) {
      return $this->getHeader().$ret;
    }

    // Update filter
    $filter = $this->getFilter();
    if ($filter) {
      $filter->update();
      session::global_set(get_class($this)."_filter", $filter);
      session::set('autocomplete', $filter);
      if (!$filter->isValid()) {
        msg_dialog::display(_("Filter error"), _("The filter is incomplete!"), ERROR_DIALOG);
      }
    }


    // Handle actions (POSTs and GETs)
    $str = $this->handleActions($this->detectPostActions());
    if ($str) {
      return $this->getHeader().$str;
    }

    // Open single dialog objects
    if (is_object($this->dialogObject)) {
      if (method_exists($this->dialogObject, 'execute')) {

        /* There is currently a subdialog open, display this dialog */
        $add = "";
        if (isset($this->dialogObject->krb_host_key) && $this->dialogObject->krb_host_key instanceof krbHostKeys) {
          $add = $this->dialogObject->krb_host_key->save_object_by_prefix($this->dialogObject->krb_service_prefix);
          $add = $this->dialogObject->krb_host_key->execute_by_prefix($this->dialogObject->krb_service_prefix, TRUE);
        }

        $display = $add.$this->dialogObject->execute();
        $display .= $this->_getTabFooter();
        return $this->getHeader().$display;
      }
    }

    // Display tab object.
    if ($this->tabObject instanceOf tabs || $this->tabObject instanceOf multi_plug) {
      $display = $this->tabObject->execute();
      $display .= $this->_getTabFooter();
      return $this->getHeader().$display;
    }

    // Set current restore base for snapshot handling.
    if (is_object($this->snapHandler)) {
      $bases = array();
      foreach ($this->storagePoints as $sp) {
        $bases[] = $sp.$this->headpage->getBase();
      }

      // No bases specified? Try base
      if (!count($bases)) {
        $bases[] = $this->headpage->getBase();
      }

      $this->snapHandler->setSnapshotBases($bases);
    }

    // Display list
    session::set('ServerService', $this->plugins);
    return $this->renderList();
  }

  function updateServicesVars ($service)
  {
    foreach (array("cn","dn") as $var) {
      if (isset($this->$var)) {
        $this->plugins[$service]->$var = $this->$var;
      }
    }
  }


  /*! \brief    Updates the status for a list of services.
   */
  function updateServiceStatus($action, $target, $all)
  {
    /* Skip if this is a new server */
    if ($this->dn == "new") {
      msg_dialog::display(_("Information"), _("Cannot update service status until it has been saved!"), INFO_DIALOG);
      return;
    }

    /* Is this an existing action */
    if (!isset(self::$actionStatus[$action])) {
      msg_dialog::display(_("Error"), _("Unknown action $action"), ERROR_DIALOG);
      return;
    }

    /* Handle state changes for services */
    foreach ($target as $service) {
      $this->updateSingleServiceStatus($action, $service);
    }
  }


  /*! \brief    Updates the status of a service and
   *             calls an external hook if specified in fusiondirectory.conf
   */
  private function updateSingleServiceStatus($action, $service)
  {
    if ($this->plugins[$service]->is_account) {
      $this->updateServicesVars($service);

      $s_daemon = new supportDaemon();
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          sprintf(_("Could not get execute action %s on service %s."), $action, $service),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
      } else {
        $res = $s_daemon->manage_service($this->parent->by_object['servgeneric']->netConfigDNS->macAddress, $service, $action);

        if ($s_daemon->is_error()) {
          msg_dialog::display(
            sprintf(_("Could not get execute action %s on service %s."), $action, $service),
            msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
          );
        } elseif ($res) {
          $this->plugins[$service]->setStatus(self::$actionStatus[$action]);
        }
      }
    }
  }


  /*! \brief   Returns a list of all used services
   *            CLASSNAME => _($this->plugins[*]->DisplayName);
   */
  function getAllUsedServices()
  {
    $ret = array();
    foreach ($this->plugins as $name => $obj) {
      if ($obj->is_account) {
        if (isset($obj->DisplayName)) {
          $ret[$name] = $obj->DisplayName;
        } else {
          $ret[$name] = $name;
        }
      }
    }
    return $ret;
  }


  /*! \brief    Returns a list of all unused services.
   */
  function getAllUnusedServices()
  {
    $tmp = $this->getAllUsedServices();
    $pool_of_ocs = array();
    foreach ($tmp as $name => $value) {
      $pool_of_ocs[] = get_class($this->plugins[$name]);
      if (isset($this->plugins[$name]->conflicts)) {
        $pool_of_ocs = array_merge($pool_of_ocs, $this->plugins[$name]->conflicts);
      }
    }

    $ret = array();
    foreach ($this->plugins as $name => $obj) {
      if (!$obj->acl_is_createable()) {
        continue;
      }

      /* Skip all pluigns that will lead into conflicts */
      $conflicts = array();
      if (isset($obj->conflicts)) {
        $conflicts = $obj->conflicts;
      }
      $conflicts[] = get_class($obj);
      if (count(array_uintersect($conflicts, $pool_of_ocs, "strcasecmp"))) {
        continue;
      }

      if (isset($obj->DisplayName)) {
        $ret[$name] = $obj->DisplayName;
      } else {
        $ret[$name] = $name;
      }
    }
    return $ret;
  }


  /*! \brief    No checks here.
   */
  function check()
  {
    return array();
  }


  /*! \brief    Keep posted form values in opened dialogs
   */
  function save_object()
  {
    if ($this->dialog && is_object($this->dialogObject)) {
      if (method_exists($this->dialogObject, 'save_object')) {
        $this->dialogObject->save_object();
      }
    }
  }


  /*! \brief    Remove all active services
   */
  function remove_from_parent()
  {
    foreach ($this->plugins as $name => $obj) {
      $this->updateServicesVars($name);
      if ($this->plugins[$name]->initially_was_account) {
        if (isset($this->plugins[$name]->krb_host_key) && $this->plugins[$name]->krb_host_key instanceof krbHostKeys) {
          $this->plugins[$name]->krb_host_key->remove_from_parent_by_prefix($this->plugins[$name]->krb_service_prefix);
        }
        $this->plugins[$name]->remove_from_parent();
      }
    }
  }


  /*! \brief    Save all active services
   */
  function save()
  {
    foreach ($this->plugins as $name => $obj) {
      $this->updateServicesVars($name);

      if ($this->plugins[$name]->is_account) {
        $this->plugins[$name]->save();
      } elseif ($this->plugins[$name]->initially_was_account) {
        $this->plugins[$name]->remove_from_parent();
      }
    }
  }


  /*! \brief    Prepare active services to be copied.
   */
  function PrepareForCopyPaste($source)
  {
    foreach ($this->plugins as $name => $plugin) {
      $this->plugins[$name]->PrepareForCopyPaste($source);
    }
  }


  /*! \brief    Check if there is an active service which prevents
   *             complete service removal.
   */
  function allow_remove()
  {
    foreach ($this->plugins as $name => $obj) {
      $str = $obj->allow_remove();
      if (!empty($str)) {
        return $str;
      }
    }
  }


  /*! \brief    Forward plugin acls
   */
  function set_acl_base($base)
  {
    $this->acl_base = $base;
    foreach ($this->plugins as $name => $obj) {
      $this->plugins[$name]->set_acl_base($base);
    }
  }


  /*! \brief    Forward plugin acls
   */
  function set_acl_category($category)
  {
    $this->acl_category = $category;
    foreach ($this->plugins as $name => $obj) {
      $this->plugins[$name]->set_acl_category($category);
    }
  }

  function saveCopyDialog()
  {
  }


  function getCopyDialog()
  {
    return array("string" => "", "status" => "");
  }

}
?>
