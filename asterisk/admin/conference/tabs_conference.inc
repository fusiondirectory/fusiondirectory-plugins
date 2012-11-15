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

class conferencetabs extends tabs
{
  var $base = "";

  function conferencetabs($config, $data, $dn, $category, $copied_object = NULL)
  {
    parent::tabs($config, $data, $dn, $category, $copied_object);
    $this->base = $this->by_object['conference']->base;

    /* Add references/acls/snapshots */
    $this->addSpecialTabs();
  }

  function check($ignore_account = FALSE)
  {
    return tabs::check(TRUE);
  }


  function save($ignore_account = FALSE)
  {
    $baseobject = $this->getBaseObject();
    $new_dn = 'cn='.$baseobject->cn.','.get_ou('phoneConferenceRDN').$baseobject->base;

    if (strtolower($this->dn) == strtolower($new_dn)) {
      $this->dn = $new_dn;
    }

    if ($this->dn != $new_dn && $this->dn != "new") {
      $baseobject->recursive_move($this->dn, $new_dn);
    }
    $this->dn = $new_dn;

    parent::save(TRUE);
  }

}

?>
