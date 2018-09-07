<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

class packageSelect extends simpleSelectManagement
{
  protected $objectTypes  = array();
  protected $autoFilter   = FALSE;

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;

  function __construct()
  {
    $this->filterXMLPath  = get_template_path('selectPackage-filter.xml', TRUE, dirname(__FILE__));
    $this->listXMLPath    = get_template_path('selectPackage-list.xml',   TRUE, dirname(__FILE__));

    if (!session::global_is_set(get_class($this).'_filter')) {
      $filter = new filterFAIcustoms($this->filterXMLPath);
      session::global_set(get_class($this).'_filter', $filter);
    }

    parent::__construct();
  }

  function save()
  {
    $act = $this->detectPostActions();
    $headpage = $this->getHeadpage();
    if (!isset($act['targets'])) {
      return array();
    }
    $ret = array();
    foreach ($act['targets'] as $dn) {
      $ret[] = $headpage->getEntry($dn);
    }
    return $ret;
  }
}
?>
