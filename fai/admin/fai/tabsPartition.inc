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

class tabsPartition extends tabs
{
  var $base   = "";
  var $parent = "";

  function tabsPartition($config, $data, $dn, $category, $copied_object = NULL)
  {
    tabs::tabs($config, $data, $dn, $category, $copied_object);

    /* Add references/acls/snapshots */
    //$this->addSpecialTabs();
  }


  function save()
  {
    $baseobject = $this->by_object['faiPartitionTable'];

    $new_dn ='cn='.$baseobject->cn.",".get_ou('faiPartitionRDN').$this->parent->fai_release;

    if ($this->dn != $new_dn && $this->dn != "new") {

      /* if( new_dn is subtree of this->dn ) */
      $cnt1 = count(explode(",", $this->dn));
      $cnt2 = count(explode(",", $new_dn));
      if ((strstr($new_dn, $this->dn)) && ($cnt1 < $cnt2)) {
        msg_dialog::display(_("Error"), _("Moving the tree failed. Destination tree is subtree of source tree."), ERROR_DIALOG);
      } else {
        //$baseobject->recursive_move($this->dn, $new_dn);
      }
    }
    $this->dn = $new_dn;

    tabs::save();
  }

}

?>
