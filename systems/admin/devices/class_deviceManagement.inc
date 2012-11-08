<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class deviceManagement extends management
{
  var $plHeadline     = "Hotplug devices";
  var $plDescription  = "Hotplug device management";
  var $plIcon  = "plugins/systems/images/devices.png";

  var $app_release = ""; // The currently selected release while in release management mode!

  // Tab definition
  protected $tabClass = "devicetabs";
  protected $tabType = "DEVICETABS";
  protected $aclCategory = "devices";
  protected $aclPlugin   = "deviceGeneric";
  protected $objectName   = "device";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Hotplug devices"),
      "plDescription" => _("Hotplug device management"),
      "plIcon"        => "plugins/systems/images/devices.png",
      "plSection"     => "admin",
      "plCategory"    => array("devices"),

      "plProvidedAcls" => array()
    );
  }

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    $filter_xml = "device-filter.xml";
    $list_xml = "device-list.xml";
    $this->storagePoints= array(get_ou("deviceRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")){
      $filter= session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path($filter_xml, true));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path($list_xml, true));
    $headpage->setFilter($filter);

    // Add copy&paste and snapshot handler.
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->cpHandler = new CopyPasteHandler($this->config);
    }
    if($this->config->get_cfg_value("enableSnapshots") == "TRUE"){
      $this->snapHandler = new SnapshotHandler($this->config);
    }
    parent::__construct($config, $ui, "devices", $headpage);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
