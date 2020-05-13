<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2019-2020 FusionDirectory

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

/*! \brief String LDAP attribute that contains an IP (v4 or v6) with netmask in CIDR notation
 *
 */
class CIDRAttribute extends StringAttribute
{
  protected $trim = TRUE;

  function validate ()
  {
    list($ip,$bits) = explode('/', $this->getValue(), 2);
    if (empty($ip) || empty($bits) || !tests::is_ip($ip) || !preg_match('/^[0-9]$/', $bits) || ($bits > 128)) {
      return msgPool::invalid($this->getLabel(), $this->getValue(), $this->example);
    }
  }
}
