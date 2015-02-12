<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2015  FusionDirectory

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

class Mission
{
  private $title;
  private $objective;
  private $objectives;
  private $init_f;

  function __construct($title, $objectives, $init)
  {
    $this->title      = $title;
    $this->objective  = 0;
    $this->objectives = array();
    foreach ($objectives as $func => $label) {
      $this->objectives[] = array('func' => $func, 'label' => $label, 'done' => FALSE);
    }
    $this->init_f     = $init;
  }

  /* Starts the mission */
  public function init()
  {
    $func = $this->init_f;
    if (!empty($this->init_f)) {
      call_user_func($this->init_f);
    }
  }

  /* Returns true if mission complete */
  public function validate()
  {
    foreach ($this->objectives as &$obj) {
      if (call_user_func($obj['func'])) {
        $obj['done'] = TRUE;
      } else {
        return FALSE;
      }
    }
    unset($obj);
    return TRUE;
  }

  public function compute_infos()
  {
    return array(
      'title' => $this->title,
      'objectives' => $this->objectives
    );
  }
}

?>
