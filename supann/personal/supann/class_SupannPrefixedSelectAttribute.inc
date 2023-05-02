<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2010-2012 Antoine Gallavardin
  Copyright (C) 2013-2020 FusionDirectory

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

class SupannPrefixedSelectAttribute extends CompositeAttribute
{
  protected $prefixedChoices;

  function __construct (string $label, string $description, string $ldapName, bool $required, string $filename = '', string $acl = '')
  {
    $attributes = [
      new SelectAttribute('', '', $ldapName.'_prefix', $required),
      new SelectAttribute('', '', $ldapName.'_content', $required)
    ];
    parent::__construct($description, $ldapName, $attributes, '/^{(.*)}(.*)$/', '{%s}%s', $acl, $label);
    $this->setLinearRendering(TRUE);
    $this->attributes[0]->setSubmitForm(TRUE);
    if (!empty($filename)) {
      $this->setPrefixedChoices(supann::get_prefixed_choices_for($filename));
    }
    $this->setRequired($required);
  }

  protected function supannUpdateSelect ()
  {
    $prefix = $this->attributes[0]->getValue();
    if (!isset($this->prefixedChoices[$prefix])) {
      $this->prefixedChoices[$prefix] = [[], []];
    }
    $this->attributes[1]->setChoices($this->prefixedChoices[$prefix][0], $this->prefixedChoices[$prefix][1]);
    $this->attributes[1]->setRequired($prefix != '');
  }

  public function setPrefixedChoices (array $prefixedChoices)
  {
    $this->prefixedChoices = $prefixedChoices;
    if (!$this->isRequired() && !isset($this->prefixedChoices[''])) {
      $this->prefixedChoices[''] = [[''], [_('None')]];
    }
    $this->attributes[0]->setChoices(array_keys($this->prefixedChoices));
    $this->supannUpdateSelect();
  }

  function applyPostValue ()
  {
    parent::applyPostValue();
    $this->supannUpdateSelect();
  }

  function setValue ($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    $this->attributes[0]->setValue($values[0]);
    $this->supannUpdateSelect();
    $this->attributes[1]->setValue($values[1]);
  }

  function resetToDefault ()
  {
    $this->attributes[0]->resetToDefault();
    $this->supannUpdateSelect();
    $this->attributes[1]->resetToDefault();
  }

  function writeValues (array $values)
  {
    if ($values[0] == '') {
      return '';
    } else {
      return parent::writeValues($values);
    }
  }

  function displayValue ($values): string
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    $this->setValue($values);
    $v1 = $this->attributes[0]->displayValue($values[0]);
    $choices2 = $this->attributes[1]->getDisplayChoices();
    if (isset($choices2[$values[1]])) {
      $v2 = $choices2[$values[1]];
    } else {
      $v2 = $values[1];
    }
    return ($v1 == '' ? $v2 : $v1.': '.$v2);
  }
}
