<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class NoAddOrderedArrayAttribute extends OrderedArrayAttribute
{
  function renderButtons ()
  {
    return '';
  }

  public function htmlIds (): array
  {
    $id   = $this->getHtmlId();
    $ids  = [];
    if ($this->attribute !== FALSE) {
      $ids  = array_merge($ids, $this->attribute->htmlIds());
    }
    $nb_values = count($this->value);
    for ($i = 0; $i < $nb_values; ++$i) {
      if ($this->order) {
        if ($i > 0) {
          $ids[] = $id.'_up_'.$i;
        }
        if (($i + 1) < $nb_values) {
          $ids[] = $id.'_down_'.$i;
        }
      }
      $ids[] = $id.'_del_'.$i;
    }
    return $ids;
  }
}
