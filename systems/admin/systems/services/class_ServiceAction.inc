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

/*!
 * \brief ServiceAction class
 */
class ServiceAction extends Action
{
  function renderColumnIcons (ListingEntry $entry): string
  {
    if (!$this->inline) {
      return '';
    }

    if (!empty($this->validTypes) && !($entry->isTemplate() && in_array('TEMPLATE', $this->validTypes)) && !in_array($entry->type, $this->validTypes)) {
      return '';
    }

    // Skip the entry completely if there's no permission to execute it
    if (!$this->hasPermission($entry)) {
      return '<img src="images/empty.png" alt=" " class="optional"/>';
    }

    if (!$this->isEnabledFor($entry)) {
      return '<img src="images/empty.png" alt=" " class="optional"/>';
    }

    // Render
    return '<input type="image" src="'.htmlentities($this->icon, ENT_COMPAT, 'UTF-8').'"'.
            ' title="'.$this->label.'" alt="'.$this->label.'" name="listing_'.$this->name.'_'.$entry->row.'"/>';
  }

  function hasPermission (ListingEntry $entry = NULL): bool
  {
    global $ui;

    if ($entry === NULL) {
      $class = NULL;
    } else {
      $class = $entry->getServiceName();
    }

    /*
     * Services are classes in server object type
     */
    $infos    = objects::infos('server');
    $category = $infos['aclCategory'];
    foreach ($this->acl as $acl) {
      $checkAcl = '';
      if (empty($acl['category'])) {
        $acl['category'] = $category;
      }
      if (empty($acl['class'])) {
        $acl['class'] = $class;
      }
      if ($entry === NULL) {
        $checkAcl = '';
        $dn       = $this->parent->listing->getBase();
        foreach (array_keys($this->parent->getAllUsedServices()) as $class) {
          $checkAcl .= $ui->get_permissions($dn, $acl['category'].'/'.$class, $acl['attribute']);
        }
      } else {
        $checkAcl = $ui->get_permissions($entry->dn, $acl['category'].'/'.$acl['class'], $acl['attribute']);
      }

      // Check rights
      foreach ($acl['acl'] as $part) {
        if (strpos($checkAcl, $part) === FALSE) {
          return FALSE;
        }
      }
    }

    return TRUE;
  }
}
