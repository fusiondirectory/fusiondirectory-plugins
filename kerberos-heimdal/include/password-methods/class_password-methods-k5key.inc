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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class passwordMethodk5key extends passwordMethod
{
  var $lockable = FALSE;

  function passwordMethodk5key($config)
  {
    
    $this->krbScript=$config->get_cfg_value("kerberosHashHook");

  }

  function is_available()
  {
    // check if userpasswd= {K5KEY}, overlay smbk5pwd is loaded, ldappasswd exist
    return true;
  }
  

  function generate_hash($pwd)
  {
    // set passwd trough exop operations ... php can't do this
    shell_exec($this->kerberosHashHook." '".$this->dn."' '".$pwd);
    unset($this->krbScript);
    
    return "{K5KEY}";       
  }

  
  function get_hash_name()
  {
    return ("k5key");
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
