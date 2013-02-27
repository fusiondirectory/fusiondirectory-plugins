<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013 FusionDirectory

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
  private $func;

  function __construct ($label, $description, $ldapName, $required = FALSE, $func, $acl = "")
  {
    $this->func = $func;
    parent::__construct($label, $description, $ldapName, $required, array(), "", NULL, $acl);
    $this->setSubmitForm(TRUE);
  }

  function check()
  {
  }

  function displayValue($value)
  {
    return $value;
  }

  function setValue($value)
  {
    $v = $this->getValue();
    parent::setValue($value);
    if (is_object($this->plugin) && ($v != $this->getValue())) {
      $func = $this->func;
      $this->plugin->$func();
    }
  }
}

class ProductPropertyAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = "")
  {
    parent::__construct ($description, $ldapName, array(), '/^(.*)$/', '%s', $acl, $label);
  }

  function check()
  {
  }

  function displayValue($value)
  {
    return $value;
  }
}

/* TODO
 * Try to avoid doing that many requests (maybe use some kind of cache?)
 * Remove from select the properties already set
 */

class opsiProductProperties extends simplePlugin
{
  var $objectclasses = array('opsiProductProperties');

  var $properties = array();

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Product properties'),
      'plDescription' => _('Product properties'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('opsiProfile'),
      'plObjectType'  => array('opsiProfile'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Name'),
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
            FALSE,  // no order
            array(),
            TRUE    // edit button
          )
        )
      )
    );
  }

  public function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }

  public function execute()
  {
    $this->updateProductList();
    return parent::execute();
  }

  private function updateProductList()
  {
    $localboot_attr = $this->parent->getBaseObject()->attributesAccess['fdOpsiLocalbootProduct'];
    $netboot_attr   = $this->parent->getBaseObject()->attributesAccess['fdOpsiNetbootProduct'];
    $products = $localboot_attr->getValue();
    $labels   =
      array_map(
        function ($a) use ($localboot_attr) {
          return $localboot_attr->displayValue($a);
        },
        $products
      );
    $netboot = $netboot_attr->getValue();
    if ($netboot != "") {
      array_unshift($products, $netboot);
      array_unshift($labels, $netboot_attr->displayValue($netboot));
    }
    $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[0]->setChoices($products, $labels);
  }

  public function product_changed()
  {
    $s_daemon = new supportDaemon();
    if (!$s_daemon->is_available()) {
      msg_dialog::display(
        _("Could not contact argonaut server"),
        msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
      );
      return;
    }
    $productid  = $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[0]->getValue();
    $properties = $s_daemon->append_call(
      'OPSI.get_product_properties',
      array($this->parent->getBaseObject()->macAddress),
      array('args' => array(
        //~ array('propertyId','possibleValues','multiValue','description','defaultValues','editable'),
        array(),
        array('productId' => $productid)
      ))
    );
    if ($s_daemon->is_error()) {
      msg_dialog::display(
        _('Could contact OPSI server'),
        msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
      );
      return;
    }
    $this->properties = array();
    foreach ($properties as $property) {
      $choices_values[] = $property['propertyId'];
      $choices_labels[] = $property['propertyId'].' ('.join(',',
        array_map(
          function ($a)
          {
            if ($a === TRUE) return "true";
            if ($a === FALSE) return "false";
            return $a;
          },
          $property['defaultValues']
        )).')';

      $this->properties[$property['propertyId']] = $property;
    }
    $this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[1]->setChoices($choices_values, $choices_labels);
  }

  public function property_changed()
  {
    $property = $this->properties[$this->attributesAccess['fdOpsiProductProperty']->attribute->attributes[1]->getValue()];
    if ($property['editable']) {
      $attr = new StringAttribute(
        '', $property['description'],
        $property['propertyId'], FALSE, $property['defaultValues'][0]
      );
    } else {
      $attr = new SelectAttribute(
        '', $property['description'],
        $property['propertyId'], FALSE,
        $property['possibleValues'], $property['defaultValues'][0],
        array_map(
          function ($a)
          {
            if ($a === TRUE) return "true";
            if ($a === FALSE) return "false";
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
