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

class repositoryManagement extends management
{

  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;
  protected $parent     = NULL;

  function __construct($config, $parent)
  {
    $this->ui     = get_userinfo();
    $this->parent = $parent;

    // Set storage points - We do not have any - We just create a fake list which lists all services
    $this->storagePoints = array("");

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")){
      $filter= session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("services/repository/repository-filter.xml", true));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("services/repository/repository-list.xml", true));
    $headpage->registerElementFilter("filterSections", "repositoryManagement::filterSections");
    $headpage->setFilter($filter);

    parent::__construct($config, $this->ui, "repositories", $headpage);
    $this->registerAction('edit',    'forwardToParent');
    $this->registerAction('remove',  'forwardToParent');
    $this->registerAction('new',     'forwardToParent');
  }


  /*! \brief    Forwards list action to the parent management class 
   */
  function forwardToParent($action, $target, $all)
  {
    if(!empty($action)){
      $tmp = $action."Entry";
      $this->parent->$tmp($action,$target,$all);  
    }
  }


  /*! \brief    Makes section array printable for the overview list.
   */
  static function filterSections($a,$b)
  {
    if(!$b) return('&nbsp;');
    return(implode($b,', '));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
