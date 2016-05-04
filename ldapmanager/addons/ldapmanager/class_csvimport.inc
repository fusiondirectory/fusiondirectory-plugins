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

      'plProvidedAcls'  => array()
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
            'type', TRUE
          ),
          new SelectAttribute (
            _('Template'), _('Select a template to apply to imported entries'),
            'template_dn', TRUE
          ),
          new FileAttribute (
            _('CSV file'), _('Import a CSV file into your LDAP'),
            'import_file', FALSE
          ),
          new SelectAttribute (
            _('Separator'), _('Character used as separator in the CSV file'),
            'separator', TRUE,
            array(',', ';')
          ),
          new SetAttribute(
            new StringAttribute(
              _('Fixed values'), _('Some fixed values that you might wanna use in the filling of the template.'),
              'fixed_values', FALSE
            )
          ),
          new ButtonAttribute (
            '', '',
            'import_submit',
            _('Import')
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
            '', '', '',
            ''
          ),
          new ButtonAttribute (
            '', '',
            'import_submit_fields',
            _('Import')
          )
        )
      )
    );
  }

  function __construct ($unused, $dn = NULL, $object = NULL)
  {
    parent::__construct($unused, $dn, $object);
    $this->attributesAccess['type']->setSubmitForm('typeChanged');
    $choices  = array();
    $outputs  = array();
    $types    = objects::types();
    foreach ($types as $type) {
      if (in_array($type, departmentManagement::getDepartmentTypes())) {
        continue;
      }
      $infos = objects::infos($type);
      if (isset($infos['management'])) {
        $choices[] = $type;
        $outputs[] = $infos['name'];
      }
    }
    $this->attributesAccess['type']->setChoices($choices, $outputs);
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
    /*
     * Add nonrequired field filling feature
     * */
    $this->template_object = new template($this->type, $this->template_dn);
    $attributes = array();
    $fields = array();
    foreach ($this->csv_data[0] as $key => $field_value) {
      $fields[$key] = $key.' - '.$field_value;
    }
    $fields = array_merge($fields, $this->fixed_values);
    $tpl_attributes = $this->template_object->serialize();
    $attributesObjects = array();
    $this->cachedChoices = array();
    reset($fields);
    foreach ($tpl_attributes as $class => $class_infos) {
      foreach ($class_infos['attrs'] as $attr => $attr_infos) {
        $choices  = $fields;
        $value    = NULL;
        $ldapName = 'template_'.$class.':'.$attr_infos['id'];
        if (isset ($attr_infos['choices'])) {
          $choices = array_merge($choices, array_values($attr_infos['choices']));
          $this->cachedChoices[$ldapName] = array_keys($attr_infos['choices']);
          if (isset($attr_infos['value']) && !empty($attr_infos['value'])) {
            $value = count($fields) + count($this->fixed_values) + array_search($attr_infos['value'], $this->cachedChoices[$ldapName]);
          }
        }
        if ($value === NULL) {
          $value = key($fields);
          next($fields);
        }
        if (in_array('SetAttribute', $attr_infos['type'])) {
          $attributesObjects[] = new SetAttribute(
            new SelectAttribute(
              $class_infos['name'].' > '.$attr_infos['label'], '',
              $ldapName, TRUE,
              array_keys($choices), $value,
              array_values($choices)
            )
          );
        } else {
          $attributesObjects[] =
            new SelectAttribute(
              $class_infos['name'].' > '.$attr_infos['label'], '',
              $ldapName, TRUE,
              array_keys($choices), $value,
              array_values($choices)
            );
        }
      }
    }
    $this->attributesAccess['fields']->setAttributes($attributesObjects);
    $this->attributesAccess['import_submit_fields']->setDisabled(FALSE);
  }

  function handle_import_submit_fields()
  {
    $success = 0;
    foreach ($this->csv_data as $rownumber => $row) {
      $values = array();
      foreach ($this->attributesAccess['fields']->attributes as $attribute) {
        preg_match('/^template_([^:]+):(.*)$/', $attribute->getLdapName(), $m);
        $i = $attribute->getValue();
        if (is_array($i)) {
          foreach ($i as $j) {
            $values[$m[1]][$m[2]] = array();
            if ($j < count($row)) {
              $values[$m[1]][$m[2]][] = $row[$j];
            } elseif ($j < (count($row) + count($this->fixed_values))) {
              $values[$m[1]][$m[2]][] = $this->fixed_values[$j - count($row)];
            } else {
              $values[$m[1]][$m[2]][] = $this->cachedChoices[$attribute->getLdapName()][$j - count($row) - count($this->fixed_values)];
            }
          }
        } else {
          if ($i < count($row)) {
            $values[$m[1]][$m[2]] = $row[$i];
          } elseif ($i < (count($row) + count($this->fixed_values))) {
            $values[$m[1]][$m[2]] = $this->fixed_values[$i - count($row)];
          } else {
            $values[$m[1]][$m[2]] = $this->cachedChoices[$attribute->getLdapName()][$i - count($row) - count($this->fixed_values)];
          }
        }
      }
      $this->template_object->reset();
      $this->template_object->deserialize($values);
      $tabObject = $this->template_object->apply();
      $msgs = $tabObject->check();
      if (count($msgs)) {
        $msg = '<ul><li>'.implode('</li><li>', $msgs).'</li></ul>';
        msg_dialog::display(sprintf(_('Import failed for line %d'), $rownumber), $msg, ERROR_DIALOG);
        break;
      } else {
        $tabObject->save();
        $success++;
      }
    }
    if ($success > 0) {
      msg_dialog::display(_('Success'), sprintf(_('Successfully imported %d entries'), $success), INFO_DIALOG);
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
