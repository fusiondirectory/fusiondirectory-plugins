<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2013  FusionDirectory

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

class BootKernelAttribute extends SelectAttribute
{
  private $cache = array();

  function setRelease($release)
  {
    if ($this->update_kernel_cache($release)) {
      $this->setChoices($this->cache[$release]);
      $this->setDisabled(FALSE);
    } else {
      $this->setChoices(array());
      $this->setValue($this->getInitialValue());
      $this->setDisabled(TRUE);
    }
  }

  function update_kernel_cache($release)
  {
    /* Get list of available kernel for this release
     */
    if (!isset($this->cache)) {
      $this->cache = array();
    }

    if (!isset($this->cache[$release])) {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        return FALSE;
      }
      $tmp = $s_daemon->FAI_get_kernels($release);
      if ($s_daemon->is_error()) {
        return FALSE;
      }
      $this->cache[$release] = $tmp;
      ksort($this->cache[$release]);
    }
    return TRUE;
  }
}
?>
