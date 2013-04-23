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

class phoneSelect extends management
{
  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;
  protected $USE_phones;
  protected $USE_mobiles;
  public $mobileDialog;

  function __construct($config, $ui, $mobileDialog)
  {
    $this->config = $config;
    $this->ui     = $ui;

    $this->mobileDialog = $mobileDialog;
    $this->USE_phones   = !$mobileDialog;
    $this->USE_mobiles  = class_available('mobilePhoneGeneric');

    $this->storagePoints = array(get_ou("phoneRDN"), get_ou("mobilePhoneRDN"));

    $filter = new filter(get_template_path("phoneSelect-filter.xml", TRUE, dirname(__FILE__)));
    $filter->setObjectStorage($this->storagePoints);

    $filter->elementValues['PHONE']       = $this->USE_phones;
    $filter->elementValues['MOBILEPHONE'] = $this->mobileDialog;
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("phoneSelect-list.xml", TRUE, dirname(__FILE__)));
    $headpage->setFilter($filter);
    parent::__construct($config, $ui, "phones", $headpage);
  }

  function execute()
  {
    $smarty = get_smarty();
    $smarty->assign('USE_phones', $this->USE_phones);
    $smarty->assign('USE_mobiles', $this->USE_mobiles);
    return parent::execute();
  }
}
?>
