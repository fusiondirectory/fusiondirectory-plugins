<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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

/*!
 * \brief Filter on registration state
 */
class RegistrationStateFilterElement extends FilterElement
{
  protected $states;

  public function __construct (managementFilter $parent)
  {
    global $config;

    parent::__construct($parent);

    $states = RegistrationStateAttribute::getStates();
    foreach ($states as $key => list($label, $icon)) {
      $this->states[$key] = [
        'filter'  => '(fdRegistrationState='.$key.')',
        'label'   => $label,
        'icon'    => $icon,
        'checked' => FALSE,
      ];
    }

    $this->states['filled']['checked'] = TRUE;
  }

  public function update ()
  {
    foreach ($this->states as $key => &$infos) {
      $infos['checked'] = isset($_POST['filter_state_'.$key]);
    }
    unset($infos);
  }

  public function render ()
  {
    $inputs = [];
    foreach ($this->states as $key => $infos) {
      $inputs['filter_state_'.$key] = [
        'name'    => $infos['label'],
        'desc'    => $infos['label'].' '.$infos['filter'],
        'icon'    => (isset($infos['icon']) ? $infos['icon'] : NULL),
        'checked' => $infos['checked'],
      ];
    }
    $smarty = get_smarty();
    $smarty->assign('NAME',   _('Statuses'));
    $smarty->assign('INPUTS', $inputs);
    return $smarty->fetch(get_template_path('management/filter-element.tpl'));
  }

  public function getFilters ($type, array &$filters)
  {
    if (strtoupper($type) == 'REGISTRATION') {
      $stateFilters = [];
      foreach ($this->states as $infos) {
        if ($infos['checked']) {
          $stateFilters[] = $infos['filter'];
        }
      }
      if (!empty($stateFilters)) {
        /* & makes no sense for exclusive states, so we force | and return only one filter */
        $filters[] = '(|'.implode($stateFilters).')';
      }
    }
    return FALSE;
  }
}
