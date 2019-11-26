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

class WebauthnRegistrationsAttribute extends OrderedArrayAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = '')
  {
    Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = FALSE;
    $this->attribute    = FALSE;
    $this->order        = FALSE;
    $this->setHeaders([
      _('Issuer'),
      _('Subject'),
      _('Signature count'),
      _('Domain'),
      '',
    ]);
  }

  function addRegistration (string $value)
  {
    $this->value[] = $value;
    $this->reIndexValues();
  }

  protected function getAttributeArrayValue ($key, $json)
  {
    $data = json_decode($json, TRUE);
    return [
      $data['certificateIssuer'],
      $data['certificateSubject'],
      ($data['signatureCounter'] ?? 0),
      $data['rpId'],
    ];
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons = $this->renderInputField('button', 'add'.$id, ['value' => '{msgPool type=addButton}', 'formnovalidate' => 'formnovalidate', 'class' => 'subattribute', 'onclick' => 'webauthnNewRegistration()']);
    return $buttons;
  }
}
