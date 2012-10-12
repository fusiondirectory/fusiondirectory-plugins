<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2012  FusionDirectory

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

class boardtabs extends tabs
{

  function __construct($config, $data, $dn = NULL, $category = "board", $hide_refs = FALSE, $hide_acls = FALSE)
  {
    parent::tabs($config, $data, $dn, $category, $hide_refs, $hide_acls);
  }

  function save_object($save_current = FALSE)
  {
  }

  function save($ignore_account = FALSE)
  {
  }
}

?>
