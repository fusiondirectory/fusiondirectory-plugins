<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class debconfProfileManagement extends management
{
  // Tab definition
  protected $tabClass     = "debconfprofiletabs";
  protected $tabType      = "DEBCONFPROFILETABS";
  protected $aclCategory  = "debconfProfile";
  protected $aclPlugin    = "debconfProfile";
  protected $objectName   = "debconfProfile";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Debconf profiles"),
      "plDescription" => _("Debconf profile management"),
      "plIcon"        => "plugins/debconf/images/debconf.png",
      "plPriority"    => 10,
      "plSection"     => "admin",
      "plCategory"    => array("debconfProfile"),

      "plProvidedAcls" => array()
    );
  }

  function __construct ($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

    $this->storagePoints = array(get_ou("debconfRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("debconfProfile-filter.xml", TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("debconfProfile-list.xml", TRUE));
    $headpage->setFilter($filter);

    parent::__construct($config, $ui, "debconfProfile", $headpage);
  }
}
?>
