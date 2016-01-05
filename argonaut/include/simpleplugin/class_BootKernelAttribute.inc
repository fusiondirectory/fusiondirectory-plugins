<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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
  function setRelease($release)
  {
    $choices = $this->get_kernel_choices($release);
    if ($choices !== FALSE) {
      $this->setChoices($choices);
      $this->setDisabled(FALSE);
    } else {
      $this->setChoices(array());
      $this->setValue($this->getInitialValue());
      $this->setDisabled(TRUE);
    }
  }

  function get_kernel_choices($release)
  {
    /* Get list of available kernel for this release */
    if (session::global_is_set('bootkernel_cache_'.$release)) {
      return session::global_get('bootkernel_cache_'.$release);
    } else {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        return FALSE;
      }
      $tmp = $s_daemon->FAI_get_kernels($release);
      if ($s_daemon->is_error()) {
        return FALSE;
      }
      ksort($tmp);
      session::global_set('bootkernel_cache_'.$release, $tmp);
      return $tmp;
    }
  }
}
?>
