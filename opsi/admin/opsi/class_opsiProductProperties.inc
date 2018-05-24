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
    parent::__construct($label, $description, $ldapName, $required, array(), '', NULL, $acl);
    $this->setSubmitForm($func);
  }

  function check()
  {
  }

  function displayValue($value)
  {
    return $value;
  }
}

class ProductPropertyAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = "")
  {
    parent::__construct ($description, $ldapName, array(new StringAttribute('', '', 'fake')), '', '', $acl, $label);
  }

  function check()
  {
  }

  function displayValue($value)
  {
    $array = json_decode($value);
    if (is_array($array) && (count($array) == 1)) {
      return $array[0];
    }
    return $value;
  }

  function readValues($value)
  {
    trigger_error('Should never be called');
  }

  function writeValues(array $values)
  {
    trigger_error('Should never be called');
  }

  function inputValue ($value)
  {
    $values = json_decode($value);
    if ($this->attributes[0] instanceof SetAttribute) {
      return array($values);
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
      return json_encode(array($values));
    }
  }
}

class opsiProductProperties extends simplePlugin
{
  var $properties = array();

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Product properties'),
      'plDescription' => _('Product properties'),
      'plObjectClass' => array('opsiProductProperties'),
      'plObjectType'  => array('opsiProfile'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Name'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Properties of the products of this profile'),
              'fdOpsiProductProperty',
              array(
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
              ),
              '|', '', ''
            ),
            // no order
            FALSE,
            array(),
            // edit button
            TRUE
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['fdOpsiProductProperty']->setHeight('auto');
  }

  public function execute()
  {
    $this->updateProductList();
    return parent::execute();
  }

  private function updateProductList()
  {
    $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[0]->setChoices(
      $this->parent->getBaseObject()->getProductList()
    );
  }

  public function product_changed()
  {
    $productid  = $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[0]->getValue();
    if (!isset($this->properties[$productid])) {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        msg_dialog::display(
          _("Could not contact argonaut server"),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
        return;
      }
      $properties = $s_daemon->append_call(
        'OPSI.get_product_properties',
        $this->parent->getBaseObject()->macAddress,
        array('args' => array(
          array(),
          array('productId' => $productid)
        ))
      );
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          _('Failed to contact OPSI server'),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
        return;
      }
      $this->properties[$productid] = array();
      foreach ($properties as $property) {
        $this->properties[$productid][$property['propertyId']] = $property;
      }
    }
    $choices_values = array();
    $choices_labels = array();
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

  public function property_changed()
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
    $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[2]->setAttributes(array($attr));
  }
}
?>
