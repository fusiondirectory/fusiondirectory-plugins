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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*!
 * \brief Fake attribute class which is used to allow editing interfaces through templates or webservice
 */
class InterfacesAttribute extends DialogOrderedArrayAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $required = FALSE, $defaultValue = '', string $acl = '')
  {
    Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
    $this->edit_enabled = TRUE;
    $this->attribute    = FALSE;
  }

  function setParent (&$plugin)
  {
    $this->plugin = $plugin;
  }

  //~ function renderAttribute (array &$attributes, bool $readOnly, bool $readable, bool $writable)
  //~ {
  //~ }

  function renderFormInput (): string
  {
    $value = $this->getValue();
    if (is_array($value)) {
      $value = join(', ', $value);
    }
    return '{literal}'.htmlescape($value).'{/literal}';
  }

  function getValue ()
  {
    return $this->plugin->filter->getInterfacesAsArrays();
  }

  function setValue ($value)
  {
    return $this->plugin->setNetworkInterfaces($value);
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    /* FIXME Convert text value to displayable array value */
    die('Pure virtual method');
  }

  function addValue (string $dn, $attrs = NULL)
  {
    /* FIXME */
    if ($attrs !== NULL) {
      $this->value[] = $attrs;
      $this->reIndexValues();
    }
  }

  protected function handleEdit ($key)
  {
    /* FIXME */
    $this->editingValue = $this->value[$key];
    $this->delPostValue($key);
    $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->editingValue));
  }

  /* readValue/writeValue? */

  function computeLdapValue ()
  {
    throw new FusionDirectoryError('computeLdapValue should not be called on InterfacesAttribute');
  }

  protected function sortValues ()
  {
  }

  protected function reIndexValues ()
  {
  }

  function foreignKeyUpdate ($oldvalue, $newvalue, array $source)
  {
    throw new FusionDirectoryError('foreignKeyUpdate should not be called on InterfacesAttribute');
  }
}
