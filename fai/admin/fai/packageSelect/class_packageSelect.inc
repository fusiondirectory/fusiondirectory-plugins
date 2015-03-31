<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class packageSelect extends management
{

  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;

  function __construct($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

    $filter = new filterFAIcustoms(get_template_path("selectPackage-filter.xml", TRUE, dirname(__FILE__)));
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("selectPackage-list.xml", TRUE, dirname(__FILE__)));
    $headpage->setFilter($filter);
    parent::__construct($config, $ui, "FAI-packages", $headpage);
  }

  function save()
  {
    $act = $this->detectPostActions();
    $headpage = $this->getHeadpage();
    if (!isset($act['targets'])) return(array());
    $ret = array();
    foreach($act['targets'] as $dn) {
      $ret[] = $headpage->getEntry($dn);
    }
    return $ret;
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
