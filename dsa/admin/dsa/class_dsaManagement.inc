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

class dsaManagement extends management
{
  // Tab definitions
  protected $tabClass     = "simpleTabs";
  protected $tabType      = "SIMPLESECURITYOBJECTTABS";
  protected $aclCategory  = "dsa";
  protected $aclPlugin    = "dsa";
  protected $objectName   = "dsa";

  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return array(
      "plShortName"   => _("DSA"),
      "plDescription" => _("DSA management"),
      "plIcon"        => "plugins/dsa/images/plugin.png",
      "plSelfModify"  => FALSE,
      "plSection"     => "admin",
      "plPriority"    => 30,
      "plCategory"    => array("dsa" => array("description"  => _("DSA"),
                                              "objectClass"  => array("simpleSecurityObject"))),

      "plProvidedAcls" => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

    $this->storagePoints = array(get_ou("dsaRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("dsa-filter.xml", TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("dsa-list.xml", TRUE));
    $headpage->registerElementFilter("filterProperties", "dsaManagement::filterProperties");
    $headpage->setFilter($filter);

    parent::__construct($config, $ui, "dsa", $headpage);
  }
}
?>
