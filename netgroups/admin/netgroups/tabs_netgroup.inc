<?php

/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * and  compatible with GOsa 2.6 (http://www.gosa-project.org)
 * Copyright (C) 2011 Alejandro Escanero Blanco (aescanero@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

class netgrouptabs extends tabs {

  function netgrouptabs($config, $data, $dn, $cat = "", $copied_object = NULL)
  {
    parent::tabs($config, $data, $dn, "netgroups", $copied_object);
    $this->addSpecialTabs();
  }

  function save_object($save_current = FALSE)
  {
    parent::save_object($save_current);

    /* Update reference, transfer variables */
    $baseobject = $this->getBaseObject();
    foreach ($this->by_object as $name => $obj) {
      /* Don't touch base object */
      if ($name != 'netgroup') {
        $obj->parent            = &$this;
        $obj->cn                = $baseobject->cn;
        $this->by_object[$name] = $obj;
      }
    }
  }

  function save()
  {
    $baseobject = $this->getBaseObject();

    /* Check for new 'dn', in order to propagate the
      'dn' to all plugins */
    $new_dn = 'cn=' . $baseobject->cn . ',' . get_ou("netgroupRDN") . $baseobject->base;

    /* Move netgroup? */
    if ($this->dn != $new_dn) {

      /* Write entry on new 'dn' */
      if ($this->dn != "new") {
        $baseobject->update_acls($this->dn, $new_dn);
        $baseobject->move($this->dn, $new_dn);
        $this->by_object['netgroup'] = $baseobject;
      }

      /* Happen to use the new one */
      $this->dn = $new_dn;
    }

    return parent::save();
  }
}
?>
