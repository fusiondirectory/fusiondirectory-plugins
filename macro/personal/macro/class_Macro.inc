<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2026  FusionDirectory

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

class Macro extends simplePlugin
{
  static function plInfo (): array
  {
     return [
      'plShortName'     => _('Macro'),
      'plDescription'   => _('Macro settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=nextcloud&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=nextcloud&size=16',
      'plSelfModify'    => FALSE,
      'plObjectType'    => ['user'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo()),
     ];
  }

  /*!
  *  \brief The main function : information about attributes
  */
  static function getAttributesInfo (): array
  {
    global $config;

    $attrsList = $config->get_cfg_value("MacroAttributesList", []);
    $attrs = [];
    foreach ($attrsList as $attributes) {
      $attributeClass  = explode('|', explode(':', $attributes)[1])[0];
      $attributeName   = explode('|', explode(':', $attributes)[1])[1];
      $attributeLabel  = explode('|', explode(':', $attributes)[1])[2];
      if ($attributeClass == 'StringAttribute') {
        $object = new $attributeClass(
          $attributeLabel,
          _('Use as %ask_'.$attributeName.'%'),
          'ask_'.$attributeName,
          FALSE
        );
      }
      array_push($attrs, $object);
    }

    return  [
      'main' => [
        'name'  => _('Macro variables'),
        'attrs' => $attrs
      ]
    ];
  }

  protected function prepare_save (): array
  {
    $errors = parent::prepare_save();

    if (!empty($errors)) {
      return $errors;
    }

    if (!$this->is_template) {
      unset($this->attrs);
    }

    return $errors;
  }

  public function render (): string
  {
    $smarty = get_smarty();
    parent::render();
    if ($this->is_template) {
      return $smarty->fetch($this->templatePath);
    } else {
      // Display message if we are not in template mode
      return "&#9888; <b>Must be used in template only</b> &#9888;";
    }
  }
}
