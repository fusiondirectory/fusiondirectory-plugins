<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class worktabs extends tabs
{

  public $was_activated = FALSE;

  function __construct($config, $data, $dn, $category, $copied_object = NULL)
  {
    parent::__construct($config, $data, $dn, $category, $copied_object);

    /* Add references/acls/snapshots */
    $this->addSpecialTabs();
  }


  function save_object($save_current = FALSE)
  {
    tabs::save_object($save_current);

    /* Assign current cn */
    $baseobject = $this->getBaseObject();
    $baseobject->netConfigDNS->cn = $baseobject->cn;
    foreach ($this->by_object as $key => $obj) {
      $this->by_object[$key]->cn = $baseobject->cn;
    }
  }


  function save()
  {
    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    $baseobject = $this->getBaseObject();
    $this->dn = "cn=".$baseobject->cn.",".get_ou('workstationRDN').$baseobject->base;

    if ($this->dn != $baseobject->orig_dn && $baseobject->orig_dn != "new") {

      /* Udpate acls */
      $baseobject->update_acls($baseobject->orig_dn,$this->dn);
      $baseobject->move($baseobject->orig_dn,$this->dn);
    }

    $baseobject->dn = $this->dn;

    foreach ($this->by_object as $key => $obj) {
      $this->by_object[$key]->dn = $this->dn;
      $this->by_object[$key]->cn = $baseobject->cn;
    }

    parent::save();

    if (isset($this->by_object['printgeneric'])) {
      if (($this->by_object['printgeneric']->is_account == FALSE) &&
          ($this->by_object['printgeneric']->initially_was_account == TRUE)) {
        $this->by_object['printgeneric']->remove_from_parent();
      }
    }
  }

}

?>
