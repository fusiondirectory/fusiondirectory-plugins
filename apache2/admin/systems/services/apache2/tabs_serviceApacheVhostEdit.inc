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

class apachevhosttabs extends tabs
{
  var $Release = "";

  function save()
  {
    $baseobject = $this->getBaseObject();
    $new_dn = "apacheServerName=".$baseobject->apacheServerName.",".session::get('serviceApacheVhostEditDN');

    /* Move group? */
    if ($this->dn != $new_dn) {

      /* Write entry on new 'dn' */
      if ($this->dn != "new") {
        $baseobject->move($this->dn, $new_dn);
        $this->by_object['serviceApacheVhostEdit'] = $baseobject;
      }

      /* Happen to use the new one */
      $this->dn = $new_dn;
    }

    return parent::save();
  }

}
?>
