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
  function __construct (string $label, string $description, string $ldapName, bool $required = FALSE, $defaultValue = [], string $acl = '')
  {
    \FusionDirectory\Core\SimplePlugin\Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
    $this->edit_enabled = TRUE;
    $this->attribute    = FALSE;
  }

  function setParent (&$plugin)
  {
    $this->plugin = $plugin;
  }

  function getValue ()
  {
    return $this->plugin->filter->getInterfacesAsArrays();
  }

  function setValue ($value)
  {
    $this->checkValue($value);
    return $this->plugin->setNetworkInterfaces($value);
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    return [
      $value['cn'][0] ?? '',
      $value['macAddress'][0] ?? '',
      implode(', ', ($value['ipHostNumber'] ?? [])),
    ];
  }

  function addValue (string $dn, $attrs = NULL)
  {
    throw new FusionDirectoryError('addValue should not be called on InterfacesAttribute');
  }

  protected function handleEdit ($key)
  {
    $this->plugin->editEntry(['targets' => [$this->getValue()[$key]['dn']], 'action' => 'edit', 'subaction' => NULL]);
  }

  function delPostValue ($key)
  {
    $this->plugin->removeRequested(['targets' => [$this->getValue()[$key]['dn']], 'action' => 'remove', 'subaction' => NULL]);
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      OrderedArrayAttribute::loadPostValue();
      if (isset($_POST['add'.$this->getHtmlId().'_dialog'])) {
        $this->plugin->newEntry(['targets' => [], 'action' => 'new', 'subaction' => 'NETWORKINTERFACE']);
      }
    }
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
