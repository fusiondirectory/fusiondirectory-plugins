<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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

class autofsManagement extends management
{
  // Tab definition
  protected $tabClass     = "";
  protected $tabType      = "";
  protected $aclCategory  = "autofs";
  protected $aclPlugin    = "autofs";
  protected $objectName   = "autofs";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Auto fs'),
      'plDescription' => _('Auto fs management'),
      'plIcon'        => 'plugins/autofs/images/icon.png',
      'plPriority'    => 24,
      'plSection'     => 'admin',
      'plManages'     => array('autofs'),

      'plProvidedAcls' => array()
    );
  }

  function __construct ($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

    $this->storagePoints = array(get_ou("autofsRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("autofs-filter.xml", TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("autofs-list.xml", TRUE));
    $headpage->setFilter($filter);

    $this->registerAction("new_nisObject",  "newEntry");
    $this->registerAction("new_nisMap",     "newEntry");

    parent::__construct($config, $ui, "autofs", $headpage);
  }

  /*! \brief  Edit the selected system type.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function newEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    $info = preg_replace("/^new_/", "", $action);
    return parent::newEntry($action, $target, $all,
        $info.'tabs', strtoupper($info.'tabs'), 'autofs');
  }

  /*! \brief  Edit the selected system type.
   */
  function editEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    if (count($target) == 1) {
      $headpage = $this->getHeadpage();
      $dn       = $target[0];
      $type     = $headpage->getType($dn);

      return parent::editEntry($action, $target, $all, $type.'tabs', strtoupper($type.'tabs'), 'autofs');
    }
  }

  /*! \brief    Queue selected objects to be removed.
   *            Checks ACLs, Locks and ask for confirmation.
   */
  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    $disallowed = array();
    $this->dns  = array();

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removal requested!");

    // Check permissons for each target
    $headpage = $this->getHeadpage();
    foreach ($target as $dn) {
      $type = $headpage->getType($dn);
      $acl  = $this->ui->get_permissions($dn, 'autofs/'.$type);
      if (preg_match("/d/", $acl)) {
        $this->dns[] = $dn;
      } else {
        $disallowed[] = $dn;
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_("Permission"), msgPool::permDelete($disallowed), INFO_DIALOG);
    }

    // We've at least one entry to delete.
    if (count($this->dns)) {

      // check locks
      if ($user = get_multiple_locks($this->dns)) {
        return gen_locked_message($user, $this->dns);
      }

      // Add locks
      $dns_names = array();
      foreach ($this->dns as $dn) {
        $dns_names[] = LDAP::fix($dn);
      }
      add_lock ($this->dns, $this->ui->dn);

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
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removal confirmed!");

    // Check permissons for each target
    $headpage   = $this->getHeadpage();
    $disallowed = array();
    foreach ($this->dns as $key => $dn) {
      $type = $headpage->getType($dn);
      $acl = $this->ui->get_permissions($dn, 'autofs/'.$type);
      if (preg_match("/d/", $acl)) {
        // Delete the object
        $this->dn = $dn;
        $tabclass = $type.'tabs';
        $this->tabObject = new $tabclass($this->config, $this->config->data['TABS'][strtoupper($type.'tabs')],
                                         $this->dn, 'autofs');
        $this->tabObject->set_acl_base($this->dn);
        $this->tabObject->parent = &$this;
        $this->tabObject->delete();

        // Remove the lock for the current object.
        del_lock($this->dn);
      } else {
        $disallowed[] = $dn;
        new log("security", "system/".get_class($this), $dn, array(), "Tried to trick deletion.");
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_("Permission"), msgPool::permDelete($disallowed), INFO_DIALOG);
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
  }

}
?>
