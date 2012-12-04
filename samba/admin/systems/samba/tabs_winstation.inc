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

class wintabs extends tabs
{
  function wintabs($config, $data, $dn, $category, $copied_object = NULL)
  {
    parent::tabs($config, $data, $dn, $category, $copied_object);

    /* Add references/acls/snapshots */
    $this->addSpecialTabs();
  }

  function save()
  {
    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */

    /* add $ to end of name if mising */
    $baseobject = $this->getBaseObject();
    if (!preg_match("/\\\$\$/", $baseobject->cn)) {
      $baseobject->cn .= "$";
    }

    /* Save this system with uid and NOT cn */
    $this->dn = "uid=$baseobject->uid,".get_winstations_ou().$baseobject->base;
    $baseobject->dn = $this->dn;

    if ($this->dn != $baseobject->orig_dn && $baseobject->orig_dn != "new") {

      /* Udpate acls */
      $baseobject->update_acls($baseobject->orig_dn, $this->dn);
      $baseobject->move($baseobject->orig_dn, $this->dn);
    }

    foreach ($this->by_object as $key => $obj) {
      $this->by_object[$key]->dn = $this->dn;
    }

    parent::save();
  }

}
?>
