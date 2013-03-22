<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org/)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2012-2013 FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class aliasManagement extends management
{
  // Tab definitions
  protected $tabClass     = "simpleTabs";
  protected $aclCategory  = "alias";
  protected $aclPlugin    = "alias";
  protected $objectName   = "alias";

  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return array(
      "plShortName"   => _("Aliases"),
      "plDescription" => _("Alias management"),
      "plIcon"        => "plugins/alias/images/alias.png",
      "plSelfModify"  => FALSE,
      "plSection"     => "admin",
      "plPriority"    => 30,
      "plCategory"    => array("alias" => array("description"  => _("Mail aliases"),
                                                "objectClass"  => array("mailAliasRedirection","mailAliasDistribution"))),

      "plProvidedAcls" => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

    $this->storagePoints = array(get_ou("aliasRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("alias-filter.xml", TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("alias-list.xml", TRUE));
    $headpage->registerElementFilter("filterDate", "aliasManagement::filterDate");
    $headpage->setFilter($filter);

    parent::__construct($config, $ui, "alias", $headpage);

    $this->registerAction("edit_alias", "editEntry");
    $this->registerAction("new_mailAliasRedirection", "newEntry");
    $this->registerAction("new_mailAliasDistribution", "newEntry");
    $this->registerAction("new_sympaAlias", "newEntry");
  }

  // Overridden new handler - We've different types of departments to create!
  function newEntry($action = "", $target = array(), $all = array(), $altTabClass = "",
                    $altTabType = "", $altAclCategory = "")
  {
    $type = preg_replace("/^new_/", "", $action);

    return management::newEntry($action, $target, $all, $this->tabClass,
                                strtoupper($type).'TABS', $this->aclCategory);
  }

  function editEntry( $action = "", $target = array(), $all = array(), $altTabClass = "",
                      $altTabType = "", $altAclCategory = "")
  {
    $type = $this->getHeadpage()->getType($target[0]);

    $str = management::editEntry( $action, $target, $all, $this->tabClass,
                                  strtoupper($type).'TABS', $this->aclCategory);
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
                                  strtoupper($type).'TABS', $this->aclCategory);
    }
  }

  /*! \brief  This method is used to queue and process copy&paste actions.
   *          Allows to copy, cut and paste mutliple entries at once.
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function copyPasteHandler($action = "", $target = array(), $all = array(),
      $altTabClass = "", $altTabType = "", $altAclCategory = "", $altAclPlugin = "")
  {
    // Add entries to queue
    if ($action == "copy" || $action == "cut") {
      $headpage = $this->getHeadpage();

      $targets = array();
      foreach ($target as $dn) {
        $type = $headpage->getType($dn);
        if (empty($type)) {
          continue;
        }
        $targets[$type][] = $dn;
      }

      foreach ($targets as $type => $dns) {
        return management::copyPasteHandler($action, $dns, $all, $this->tabClass,
                                    strtoupper($type).'TABS', $this->aclCategory);
      }
    } else {
      return management::copyPasteHandler($action, $target, $all);
    }
  }

  static function filterDate()
  {
    if (func_num_args() == 0) {
      return '&nbsp;';
    }
    $date = func_get_arg(0);
    if (!is_array($date)) {
      return '&nbsp;';
    }
    $dateObject = DateTime::createFromFormat('U', $date[0], new DateTimeZone('UTC'));
    if (is_object($dateObject)) {
      return $dateObject->format('d.m.Y');
    }
    return '&nbsp;';
  }
}
?>
