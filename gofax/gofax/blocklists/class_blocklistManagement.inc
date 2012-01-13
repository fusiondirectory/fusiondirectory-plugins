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

class blocklist extends management
{
  var $plHeadline     = "Blocklists";
  var $plDescription  = "Blocklist management";
  var $plIcon  = "plugins/gofax/images/blocklists.png";

  // Tab definition 
  protected $tabClass = "faxblocktabs";
  protected $tabType = "FAXBLOCKTABS";
  protected $aclCategory = "gofaxlist";
  protected $aclPlugin   = "blocklistGeneric";
  protected $objectName   = "blocklist";

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    $this->storagePoints = array(get_ou("faxBlocklistRDN"));

      $filter = new filter(get_template_path("blocklist-filter.xml", true));
      $filter->setObjectStorage($this->storagePoints);
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("blocklist-list.xml", true));
    $headpage->setFilter($filter);

    // Add copy&paste and snapshot handler.
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->cpHandler = new CopyPasteHandler($this->config);
    }
    if($this->config->get_cfg_value("enableSnapshots") == "true"){
      $this->snapHandler = new SnapshotHandler($this->config);
    }
    parent::__construct($config, $ui, "blocklists", $headpage);
  }
} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
