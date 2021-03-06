<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016 FusionDirectory

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

class SubmittingOPSIProductAttribute extends SelectAttribute
{
  function __construct ($label, $description, $ldapName, $required, $func, $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, [], '', NULL, $acl);
    $this->setSubmitForm($func);
  }

  function check ()
  {
  }

  function displayValue ($value): string
  {
    return $value;
  }
}

class ProductPropertyAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = "")
  {
    parent::__construct($description, $ldapName, [new StringAttribute('', '', 'fake')], '', '', $acl, $label);
  }

  function check ()
  {
  }

  function displayValue ($value): string
  {
    $array = json_decode($value);
    if (is_array($array) && (count($array) == 1)) {
      return $array[0];
    }
    return $value;
  }

  function readValues (string $value): array
  {
    throw new FusionDirectoryException('Should never be called');
  }

  function writeValues (array $values)
  {
    throw new FusionDirectoryException('Should never be called');
  }

  function inputValue ($value)
  {
    $values = json_decode($value);
    if ($this->attributes[0] instanceof SetAttribute) {
      return [$values];
    } else {
      return $values;
    }
  }

  /* We always return the LDAP value as the composite attribute has nothing else */
  function getValue ()
  {
    $values = $this->attributes[0]->computeLdapValue();
    if ($this->attributes[0] instanceof SetAttribute) {
      return json_encode($values);
    } else {
      return json_encode([$values]);
    }
  }
}

class opsiProductProperties extends simplePlugin
{
  var $properties = [];

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Product properties'),
      'plDescription' => _('Product properties'),
      'plSmallIcon'   => 'geticon.php?context=applications&icon=opsi&size=16',
      'plObjectClass' => ['opsiProductProperties'],
      'plObjectType'  => ['opsiProfile'],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Name'),
        'class' => ['fullwidth'],
        'attrs' => [
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Properties of the products of this profile'),
              'fdOpsiProductProperty',
              [
                new SubmittingOPSIProductAttribute(
                  '', '',
                  'productId', TRUE,
                  'product_changed'
                ),
                new SubmittingOPSIProductAttribute(
                  '', '',
                  'propertiesId', TRUE,
                  'property_changed'
                ),
                new ProductPropertyAttribute(
                  '', '', 'propertyValue'
                ),
              ],
              '|', '', ''
            ),
            // no order
            FALSE,
            [],
            // edit button
            TRUE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
  }

  public function update (): bool
  {
    parent::update();
    $this->updateProductList();
    return TRUE;
  }

  private function updateProductList ()
  {
    $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[0]->setChoices(
      $this->parent->getBaseObject()->getProductList()
    );
  }

  public function product_changed ()
  {
    $productid  = $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[0]->getValue();
    if (!isset($this->properties[$productid])) {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
        $error->display();
        return;
      }
      $properties = $s_daemon->append_call(
        'OPSI.get_product_properties',
        $this->parent->getBaseObject()->macAddress,
        ['args' => [
          [],
          ['productId' => $productid]
        ]]
      );
      if ($s_daemon->is_error()) {
        $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
        $error->display();
        return;
      }
      $this->properties[$productid] = [];
      foreach ($properties as $property) {
        $this->properties[$productid][$property['propertyId']] = $property;
      }
    }
    $choices_values = [];
    $choices_labels = [];
    foreach ($this->properties[$productid] as $property) {
      $choices_values[] = $property['propertyId'];
      $choices_labels[] = $property['propertyId'].' ('.join(',',
        array_map(
          function ($a)
          {
            if ($a === TRUE) {
              return 'true';
            }
            if ($a === FALSE) {
              return 'false';
            }
            return $a;
          },
          $property['defaultValues']
        )).')';
    }
    $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[1]->setChoices($choices_values, $choices_labels);
  }

  public function property_changed ()
  {
    $productid  = $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[0]->getValue();
    $propertyid = $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[1]->getValue();
    if (!isset($this->properties[$productid][$propertyid])) {
      /* When the product just changed, the property field contains an inconsistent value */
      return;
    }
    $property = $this->properties[$productid][$propertyid];
    $defaultValue = (isset($property['defaultValues'][0]) ? $property['defaultValues'][0] : '');
    if ($property['editable']) {
      $attr = new StringAttribute(
        '', $property['description'],
        $property['propertyId'], FALSE, $defaultValue
      );
    } else {
      $attr = new SelectAttribute(
        '', $property['description'],
        $property['propertyId'], FALSE,
        $property['possibleValues'], $defaultValue,
        array_map(
          function ($a)
          {
            if ($a === TRUE) {
              return 'true';
            }
            if ($a === FALSE) {
              return 'false';
            }
            return $a;
          },
          $property['possibleValues']
        )
      );
    }
    if ($property['multiValue']) {
      $attr = new SetAttribute($attr, $property['defaultValues']);
    }
    $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[2]->setAttributes([$attr]);
  }
}
