<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019  FusionDirectory

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

class packageSelectManagement extends selectManagement
{
  /* Create faiRepositoryPackage object type */
  static function plInfo (): array
  {
    return [
      'plShortName'   => 'Package selection',
      'plDescription' => 'Package selection for FAI package class',
      'plObjectType'  => ['faiRepositoryPackage' => [
        'name'        => _('Package'),
        'aclCategory' => 'fai',
        'icon'        => 'geticon.php?context=applications&icon=fai-packages&size=16',
      ]],
    ];
  }

  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'PACKAGE',  'label' => 'Package']],
    ['LinkColumn',       ['attributes' => 'VERSION',  'label' => 'Version']],
  ];

  /* Disable and hide configuration system */
  protected $skipConfiguration = TRUE;

  protected $parentPlugin;

  function __construct (
    $parent,
    $objectTypes        = FALSE,
    bool $multiSelect   = TRUE,
    array $neededAttrs  = [],
    array $blackList    = [],
    array $whiteList    = [],
    array $filterElementDefinitions = [])
  {
    $this->parentPlugin = $parent;

    parent::__construct($objectTypes, $multiSelect, $neededAttrs, $blackList, $whiteList,  $filterElementDefinitions);
  }

  protected function setUpListing ()
  {
    /* Set baseMode to FALSE */
    $this->listing  = new managementListing($this, FALSE, $this->multiSelect);
  }

  protected function setUpFilter (array $filterElementDefinitions)
  {
    $this->filter   = new packageSelectManagementFilter($this, TRUE, $filterElementDefinitions);
  }

  public function getCustomReleases (): array
  {
    return serviceRepository::getCustomReleases($this->parentPlugin->FAIdebianRelease);
  }

  public function getRelease (): string
  {
    return $this->parentPlugin->FAIdebianRelease;
  }
}
