<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110 - 1301, USA.
*/

class csvimport extends simplePlugin
{
  protected $template_object;
  protected $cachedChoices;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('CSV import'),
      'plDescription' => _('Import of csv data into the ldap tree'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('ldapmanager'),
      'plPriority'    => 3,

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'import' => array(
        'name'  => _('Import CSV'),
        'attrs' => array(
          new SelectAttribute (
            _('Object type'), _('Type of objects you wish to import'),
            'type', TRUE,
            array(), '', NULL,
            'import_file'
          ),
          new SelectAttribute (
            _('Template'), _('Select a template to apply to imported entries'),
            'template_dn', TRUE,
            array(), '', NULL,
            'import_file'
          ),
          new FileAttribute (
            _('CSV file'), _('Import a CSV file into your LDAP'),
            'import_file', FALSE
          ),
          new SelectAttribute (
            _('Separator'), _('Character used as separator in the CSV file'),
            'separator', TRUE,
            array(',', ';'), '', NULL,
            'import_file'
          ),
          new SetAttribute(
            new StringAttribute(
              _('Fixed values'), _('Some fixed values that you might wanna use in the filling of the template.'),
              'fixed_values', FALSE,
              '',
              'import_file'
            )
          ),
          new ButtonAttribute (
            '', '',
            'import_submit',
            _('Import'),
            NULL, '',
            'import_file'
          )
        )
      ),
      'fields' => array(
        'name'  => _('Template filling'),
        'attrs' => array(
          /*new SetAttribute(
            new SelectAttribute(
              _('Additional fields'), _('Non-required template field that you still want to provide'),
              'additionals', FALSE
            )
          ),*/
          new CompositeAttribute (
            _('Select fields associations'),
            'fields',
            array(),
            '', '',
            'import_file',
            ''
          ),
          new ButtonAttribute (
            '', '',
            'import_submit_fields',
            _('Import'),
            NULL, '',
            'import_file'
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['type']->setSubmitForm('typeChanged');
    $choices  = array();
    $types    = objects::types();
    foreach ($types as $type) {
      if (in_array($type, departmentManagement::getDepartmentTypes())) {
        continue;
      }
      $infos = objects::infos($type);
      if ($infos['templateActive']) {
        $choices[$type] = $infos['name'];
      }
    }
    asort($choices);
    $this->attributesAccess['type']->setChoices(array_keys($choices), array_values($choices));
    /* People usually import users, so set it as default */
    $this->attributesAccess['type']->setDefaultValue('USER');
    $this->attributesAccess['type']->resetToDefault();
    $this->attributesAccess['import_submit_fields']->setDisabled(TRUE);
  }

  function typeChanged()
  {
    $templates = objects::getTemplates($this->type);
    $this->attributesAccess['template_dn']->setChoices(array_keys($templates), array_values($templates));
  }

  function handle_import_submit()
  {
    $this->csv_data = $this->parse_csv($this->import_file);

    $messages = $this->check();
    if (!empty($messages)) {
      msg_dialog::displayChecks($messages);
      return;
    }

    $this->template_object = new template($this->type, $this->template_dn);

    /* Add nonrequired field filling feature */
    $fields = array();
    if (isset($this->csv_data[0])) {
      foreach ($this->csv_data[0] as $key => $field_value) {
        $fields[$key] = $key.' - '.$field_value;
      }
    }
    $fields = array_merge($fields, $this->fixed_values);
    $tpl_attributes = $this->template_object->serialize();
    $attributesObjects = array();
    $this->cachedChoices = array();
    reset($fields);
    foreach ($tpl_attributes as $class => $class_infos) {
      foreach ($class_infos['attrs'] as $attr => $attr_infos) {
        if ($attr_infos['visible']) {
          $attributesObjects[] = $this->build_attribute($fields, $class, $class_infos, $attr, $attr_infos);
        }
      }
    }
    $this->attributesAccess['fields']->setAttributes($attributesObjects);
    $this->attributesAccess['import_submit_fields']->setDisabled(FALSE);
  }

  private function build_attribute(&$fields, $class, $class_infos, $attr, $attr_infos)
  {
    $choices  = $fields;
    $value    = NULL;
    $ldapName = 'template_'.$class.':'.$attr;
    if (isset ($attr_infos['choices'])) {
      $choices = array_merge($choices, array_values($attr_infos['choices']));
      $this->cachedChoices[$ldapName] = array_keys($attr_infos['choices']);
      if (isset($attr_infos['value']) && !empty($attr_infos['value'])) {
        $value = count($fields) + array_search($attr_infos['value'], $this->cachedChoices[$ldapName]);
      }
    }
    if ($value === NULL) {
      $value = key($fields);
      next($fields);
    }
    if (isset($attr_infos['attributes'])) {
      $subattrs = array();
      foreach ($attr_infos['attributes'] as $subattr => $subattr_infos) {
        if ($subattr_infos['visible']) {
          $subattribute = $this->build_attribute($fields, $class, $class_infos, $subattr, $subattr_infos);
        } else {
          $this->cachedChoices['template_'.$class.':'.$subattr] = array($attr_infos['value']);
          $subattribute = new HiddenAttribute('template_'.$class.':'.$subattr, FALSE, count($fields));
        }
        $subattrs[] = $subattribute;
      }
      return new CompositeAttribute(
        '', $ldapName,
        $subattrs,
        FALSE, FALSE
      );
    }
    if ($value === NULL) {
      $value = key($fields);
      next($fields);
    }
    if (in_array('SetAttribute', $attr_infos['type'])) {
      return new SetAttribute(
        new SelectAttribute(
          $class_infos['name'].' > '.$attr_infos['label'], '',
          $ldapName, TRUE,
          array_keys($choices), $value,
          array_values($choices),
          'import_file'
        )
      );
    } else {
      return
        new SelectAttribute(
          $class_infos['name'].' > '.$attr_infos['label'], '',
          $ldapName, TRUE,
          array_keys($choices), $value,
          array_values($choices),
          'import_file'
        );
    }
  }

  function handle_import_submit_fields()
  {
    $success = 0;
    foreach ($this->csv_data as $rownumber => $row) {
      $values = array();
      foreach ($this->attributesAccess['fields']->attributes as $attribute) {
        preg_match('/^template_([^:]+):(.*)$/', $attribute->getLdapName(), $m);
        $values[$m[1]][$m[2]] = $this->compute_attribute_value($attribute, $row);
      }
      $this->template_object->reset();
      $this->template_object->deserialize($values);
      $tabObject = $this->template_object->apply();
      $msgs = $tabObject->save();
      if (count($msgs)) {
        $msg = '<ul><li>'.implode('</li><li>', $msgs).'</li></ul>';
        msg_dialog::display(sprintf(_('Import failed for line %d'), $rownumber), $msg, ERROR_DIALOG);
        break;
      } else {
        $success++;
      }
    }
    if ($success > 0) {
      msg_dialog::display(_('Success'), sprintf(_('Successfully imported %d entries'), $success), INFO_DIALOG);
    }
  }

  private function compute_attribute_value($attribute, $row, $i = NULL)
  {
    if ($attribute instanceof CompositeAttribute) {
      $value = array();
      foreach ($attribute->attributes as $key => $subattribute) {
        $value[$key] = $this->compute_attribute_value($subattribute, $row);
      }
      return $value;
    }
    if ($i === NULL) {
      $i = $attribute->getValue();
    }
    if (is_array($i)) {
      $value = array();
      foreach ($i as $j) {
        $value[] = $this->compute_attribute_value($attribute, $row, $j);
      }
      return $value;
    } else {
      if ($i < count($row)) {
        return $row[$i];
      } elseif ($i < (count($row) + count($this->fixed_values))) {
        return $this->fixed_values[$i - count($row)];
      } else {
        return $this->cachedChoices[$attribute->getLdapName()][$i - count($row) - count($this->fixed_values)];
      }
    }
  }

  function parse_csv($str)
  {
    $lines  = preg_split("/\n/", $str);

    $anz    = 0;
    $rest   = 0;
    $data   = array();

    /* check column count */
    if (is_array($lines)) {
      foreach ($lines as $line) {
        /* continue if theres a comment */
        if (substr(trim($line), 0, 1) == "#") {
          continue;
        }

        $cells  = str_getcsv($line, $this->separator);

        if (count($cells) > $anz ) {
          $anz = count($cells);
        }
      }
    }

    /* At least one entry */
    if ($anz > 1) {

      /* Generate array with output info  */
      if (is_array($lines)) {
        foreach ($lines as $line) {
          $line_data = array();
          $rest = 0;
          $cnt  = 0;

          /* dont use comments or empty lines */
          if ((substr(trim($line), 0, 1) == "#") || (empty($line))) {
            continue;
          }

          /* get all elements  */
          $cells  = str_getcsv($line, $this->separator);

          /* attach all elements to data array */
          if (is_array($cells)) {
            foreach ($cells as $cell) {
              $cnt++;
              $line_data[] = trim($cell);
            }
          }

          /* cell count less than anz, attach some empty fields */
          if (($cnt < $anz) && ($cnt > 0)) {
            $rest = $anz - $cnt;
            for ($i = 0; $i < $rest; $i++) {
              $line_data[] = "";
            }
          }

          $data[] = $line_data;
        }
      }
    }

    return $data;
  }

  function save()
  {
  }
}
?>
