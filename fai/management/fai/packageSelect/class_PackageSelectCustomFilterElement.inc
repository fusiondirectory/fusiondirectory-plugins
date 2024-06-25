<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019  FusionDirectory

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

class PackageSelectCustomFilterElement extends FilterElement
{
  protected $customs;

  public function __construct (managementFilter $parent)
  {
    global $config;

    parent::__construct($parent);

    $this->customs = [];
    foreach ($this->parent->parent->getCustomReleases() as $custom) {
      $this->customs[$custom] = [
        'custom'  => $custom,
        'checked' => FALSE,
      ];
    }
  }

  public function update ()
  {
    foreach ($this->customs as $key => &$custom) {
      $custom['checked'] = isset($_POST['filter_custom_'.$key]);
    }
    unset($custom);
  }

  public function render (): string
  {
    if (empty($this->customs)) {
      return '';
    }
    $inputs = [];
    foreach ($this->customs as $key => $custom) {
      $inputs['filter_custom_'.$key] = [
        'name'    => sprintf(_('Custom %s'), $custom['custom']),
        'desc'    => sprintf(_('Custom %s'), $custom['custom']),
        'icon'    => NULL,
        'checked' => $custom['checked'],
      ];
    }
    $smarty = get_smarty();
    $smarty->assign('NAME',   _('Customs'));
    $smarty->assign('INPUTS', $inputs);
    return $smarty->fetch(get_template_path('management/filter-element.tpl'));
  }

  public function getFilters (string $type, array &$filters): bool
  {
    foreach ($this->customs as $custom) {
      if ($custom['checked']) {
        $filters[] = $custom['custom'];
      }
    }
    return FALSE;
  }
}
