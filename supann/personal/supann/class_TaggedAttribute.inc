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

class TaggedAttribute extends SetAttribute
{
  function __construct (\FusionDirectory\Core\SimplePlugin\Attribute $attribute, array $values = [], bool $valueUnicity = TRUE)
  {
    parent::__construct($attribute, $values, $valueUnicity);
  }

  protected function loadAttrValue (array $attrs)
  {
    $this->value  = [];
    $pattern      = '/^'.preg_quote($this->getLdapName(), '/').';(.+)$/';
    foreach ($attrs as $key => $attr) {
      if ($key == $this->getLdapName()) {
        for ($j = 0; $j < $attr['count']; $j++) {
          $this->value[] = ';'.$attr[$j];
        }
      } elseif (preg_match($pattern, $key, $m)) {
        for ($j = 0; $j < $attr['count']; $j++) {
          $this->value[] = $m[1].';'.$attr[$j];
        }
      }
    }
    $this->sortValues();
  }

  function computeLdapValue ()
  {
    throw new FusionDirectoryException('nope');
  }

  /*! \brief Fill LDAP value in the attrs array
   */
  function fillLdapValue (array &$attrs)
  {
    if ($this->isInLdap()) {
      $initialValues  = $this->getInitialValue();
      foreach ($initialValues as $value) {
        list($tag, $tagvalue) = explode(';', $value, 2);
        if (empty($tag)) {
          $attrs[$this->getLdapName()] = [];
        } else {
          $attrs[$this->getLdapName().';'.$tag] = [];
        }
      }
      if (!empty($this->value)) {
        foreach ($this->value as $value) {
          list($tag, $tagvalue) = explode(';', $value, 2);
          if (empty($tag)) {
            $attrs[$this->getLdapName()][] = $tagvalue;
          } else {
            $attrs[$this->getLdapName().';'.$tag][] = $tagvalue;
          }
        }
      }
    }
  }
}
