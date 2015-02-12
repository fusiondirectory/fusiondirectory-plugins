<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2015  FusionDirectory

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

class filterFAIcustoms extends filter {

  var $customs = array();

  function filterFAIcustoms($filename)
  {
    $release = session::get('packageSelect_Release');
    $this->customs = $this->getCustomList($release);

    parent::filter($filename);
  }

  function load($filename)
  {
    $res = parent::load($filename);

    foreach ($this->customs as $custom) {
      $this->elements[$custom] = array(
        'type'    => 'checkbox',
        'tag'     => $custom,
        'default' => 'false',
        'set'     => array(),
        'unset'   => array()
      );
      $this->elementValues[$custom] = '';
    }
    return $res;
  }

  function render()
  {
    $smarty = get_smarty();

    $customs = array();
    foreach ($this->customs as $custom) {
      $customs[$custom] = $this->getCheckbox($this->elements[$custom]);
    }
    $smarty->assign("customs",$customs);

    return filter::render();
  }

  function getCustomList ($release)
  {
    $q = new supportDaemon();
    return $q->FAI_get_customs($release);
  }
}
