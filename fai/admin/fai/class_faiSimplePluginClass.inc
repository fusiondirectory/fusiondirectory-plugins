<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class faiSimplePluginClass extends simplePlugin
{
  public $base;

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    if ($dn == "new") {
      $ui = get_userinfo();
      $this->base = dn2base(session::global_is_set("CurrentMainBase")?"cn=dummy,".session::global_get("CurrentMainBase"):$ui->dn);
    } else {
      $this->base = dn2base($dn, get_ou(get_class($this).'RDN'));
    }
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message = parent::check();

    /* Ensure that we do not overwrite an allready existing entry */
    $new_dn   = $this->compute_dn();
    if ($this->dn != $new_dn) {
      $ldap     = $this->config->get_ldap_link();
      $ldap->cat($new_dn);
      if ($ldap->count() != 0) {
        $message[] = msgPool::duplicated(_("Name"));
      }
    }

    return $message;
  }

  function compute_dn()
  {
    return 'cn='.$this->cn.','.get_ou(get_class($this).'RDN').$this->base;
  }
}
?>
