<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class SudoSystemsAttribute extends SystemsAttribute
{
  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = $this->renderInputField(
      'submit', 'add'.$id.'_ALL',
      [
        'value'           => 'ALL',
        'formnovalidate'  => 'formnovalidate'
      ]
    );
    return $buttons.parent::renderButtons();
  }

  public function htmlIds (): array
  {
    $id = $this->getHtmlId();
    $ids = parent::htmlIds();
    $ids[] = 'add'.$id.'_ALL';
    return $ids;
  }

  function loadPostValue ()
  {
    parent::loadPostValue();
    if ($this->isVisible()) {
      if (isset($_POST['add'.$this->getHtmlId().'_ALL'])) {
        $this->value    = ['ALL'];
        $this->displays = ['ALL'];
        $this->types    = [FALSE];
      }
    }
  }

  function addValue (string $dn, $entry = NULL)
  {
    if (isset($this->value[0]) && ($this->value[0] === 'ALL')) {
      $this->removeValue(0);
    }
    parent::addValue($dn, $entry);
  }

  protected function fillDisplayValue ($i)
  {
    if ($this->value[$i] === 'ALL') {
      $this->displays[$i] = 'ALL';
      $this->types[$i]    = FALSE;
    } else {
      parent::fillDisplayValue($i);
    }
  }
}
