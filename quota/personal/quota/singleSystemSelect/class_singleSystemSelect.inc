<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2011-2013  FusionDirectory
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

class singleSystemSelect extends management
{

  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;
    $this->storagePoints = array(get_ou("sambaMachineAccountRDN"),"ou=systems,");

    $filter = new filter(get_template_path("singleSystem-filter.xml", true, dirname(__FILE__)));
    $filter->setObjectStorage($this->storagePoints);

    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("singleSystem-list.xml", true, dirname(__FILE__)));
    $headpage->setFilter($filter);
    parent::__construct($config, $ui, "systems", $headpage);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
