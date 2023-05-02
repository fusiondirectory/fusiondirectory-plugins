<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019  FusionDirectory

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

class RecoveryCodesAttribute extends DialogOrderedArrayAttribute
{
  protected $order        = FALSE;
  protected $dialogClass  = NULL;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = FALSE;
  }

  /*! \brief Convert text value to displayable array value */
  protected function getAttributeArrayValue ($key, $value)
  {
    /* Show only date of generation */
    return [$value[0]];
  }

  function readValue ($value)
  {
    return [count($this->value), explode('|', $value, 2)];
  }

  function writeValue ($key, $value)
  {
    return implode('|', $value);
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      OrderedArrayAttribute::loadPostValue();
      OrderedArrayAttribute::applyPostValue();
      if (isset($_POST['add'.$this->getHtmlId().'_dialog'])) {
        $this->generateCode();
      }
    }
  }

  protected function generateCode ()
  {
    $len        = random_int(10, 15);
    /* All letters and numbers except 0, O and o to avoid confusion */
    $base       = 'ABCDEFGHKLMNPQRSTWXYZabcdefghjkmnpqrstwxyz123456789';
    $max        = strlen($base) - 1;
    $randomhash = '';
    while (strlen($randomhash) < $len + 1) {
      $randomhash .= $base[random_int(0, $max)];
    }
    $this->value[] = [date('c'), password_hash($randomhash, PASSWORD_DEFAULT)];
    msg_dialog::display(
      _('Recovery code'),
      htmlescape(sprintf(_('Here is your recovery code: %s'), $randomhash)).'<br><br>Print and hide it. Do not save it on your computer. You may only use it once.',
      INFO_DIALOG
    );
  }
}
