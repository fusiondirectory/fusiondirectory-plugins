<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2013  FusionDirectory

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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannStructuresManagement extends management
{
  // Tab definition
  protected $tabClass     = "simpleTabs";
  protected $aclCategory  = "supannStructures";
  protected $aclPlugin    = "supannStructures";
  protected $objectName   = "supannStructures";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Supann structures"),
      "plDescription" => _("Supann structures management"),
      "plIcon"        => "plugins/supann/images/icon.png",
      "plSection"     => "admin",
      "plCategory"    => array("supannStructures"),

      "plProvidedAcls" => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

    $this->storagePoints = array(get_ou("supannStructuresRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("supannStructures-filter.xml", TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("supannStructures-list.xml", TRUE));
    $headpage->registerElementFilter("structureLabel",
                                     "supannStructuresManagement::filterStructureLabel");
    $headpage->registerElementFilter("filterProperties",
                                     "supannStructuresManagement::filterProperties");
    $headpage->setFilter($filter);

    parent::__construct($config, $ui, "supannStructures", $headpage);

    $this->registerAction("edit_etablissement", "editEntry");
    $this->registerAction("edit_entite", "editEntry");

    /*
    $this->registerAction("remove_etablissement", "removeEntry");
    $this->registerAction("remove_entite", "removeEntry");
    */
    $this->registerAction("removeConfirmed", "removeEntryConfirmed");

    $this->registerAction("organizationalUnit", "newEntry");
    $this->registerAction("organization", "newEntry");


    $this->registerAction("sendMessage", "sendMessage");
    $this->registerAction("saveEventDialog", "saveEventDialog");
    $this->registerAction("abortEventDialog", "closeDialogs");
  }

  // Inject user actions
  function detectPostActions()
  {
    $action = management::detectPostActions();
    if (isset($_POST['save_event_dialog'])) {
      $action['action'] = "saveEventDialog";
    }
    if (isset($_POST['abort_event_dialog'])) {
      $action['action'] = "abortEventDialog";
    }
    return $action;
  }

  static function filterStructureLabel($row, $dn, $pid, $base, $ou = NULL)
  {
    if ($ou == NULL) {
      return "&nbsp;";
    }
    $ou = $ou[0];
    if ($dn == $base) {
      $ou = ".";
    }
    $dn = LDAP::fix(func_get_arg(1));
    return  "<a href='?plug=".$_GET['plug'].
            "&amp;PID=$pid&amp;act=listing_edit_$row' title='$dn'>$ou</a>";
  }

  // Overridden new handler - We've different types of departments to create!
  function newEntry($action = "", $target = array(), $all = array(), $altTabClass = "",
                    $altTabType = "", $altAclCategory = "")
  {
    $types = $this->get_support_supannStructures();
    $type = preg_replace("/^new_/", "", $action);

    return management::newEntry($action, $target, $all, $this->tabClass,
                                $types[$type]['TAB'], $this->aclCategory);
  }


  function editEntry( $action = "", $target = array(), $all = array(), $altTabClass = "",
                      $altTabType = "", $altAclCategory = "")
  {
    $types = $this->get_support_supannStructures();
    $headpage = $this->getHeadpage();

    $type = $headpage->getType($target[0]);

    $str = management::editEntry( $action, $target, $all, $this->tabClass,
                                  $types[$type]['TAB'], $this->aclCategory);
    if (preg_match("/^edit_/", $action)) {
      $tab = preg_replace("/^edit_/", "", $action);
      if (isset($this->tabObject->by_object[$tab])) {
        $this->tabObject->current = $tab;
      } else {
        trigger_error("Unknown tab: ".$tab);
      }
    }
    if (!empty($str)) {
      return $str;
    }
  }

  function removeEntryConfirmed($action = "", $target = array(), $all = array(),
                                $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    $types    = $this->get_support_supannStructures();
    $headpage = $this->getHeadpage();

    $targets = array();
    foreach ($this->dns as $dn) {
      $type = $headpage->getType($dn);
      if (empty($type)) {
        continue;
      }
      $targets[$type][] = $dn;
    }

    foreach ($targets as $type => $dns) {
      $this->dns = $dns;
      management::removeEntryConfirmed($action, $target, $all, $this->tabClass,
                                  $types[$type]['TAB'], $this->aclCategory);
    }
  }

  /*! \brief  Returns information about all container types that GOsa con handle.
     @return Array   Informations about departments supported by GOsa.
   */
  public static function get_support_supannStructures()
  {
    $types = array();

    /* Locality object
     */
    $types['organization']['ACL']      = "etablissement";
    $types['organization']['CLASS']    = "etablissement";
    $types['organization']['TAB']      = "ETABLISSEMENTTABS";
    $types['organization']['ATTR']     = "o";
    $types['organization']['OC']       = "organization";
    $types['organization']['IMG']      = "plugins/supann/images/etablissement.png";
    $types['organization']['IMG_FULL'] = "plugins/supann/images/etablissement.png";
    $types['organization']['TITLE']    = _("etablissement");
    $types['organization']['TPL']      = "etablissement.tpl";

    $types['organizationalUnit']['ACL']       = "entite";
    $types['organizationalUnit']['CLASS']     = "entite";
    $types['organizationalUnit']['TAB']       = "ENTITETABS";
    $types['organizationalUnit']['ATTR']      = "ou";
    $types['organizationalUnit']['OC']        = "organizationalUnit";
    $types['organizationalUnit']['IMG']       = "plugins/supann/images/entite.png"; //plugins/departments/images/department.png";
    $types['organizationalUnit']['IMG_FULL']  = "plugins/supann/images/entite.png";  //:wplugins/departments/images/department.png";
    $types['organizationalUnit']['TITLE']     = _("entite");
    $types['organizationalUnit']['TPL']       = "entite.tpl";

    return $types;
  }
}

?>
