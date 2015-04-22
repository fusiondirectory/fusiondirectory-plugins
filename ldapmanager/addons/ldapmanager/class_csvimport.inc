<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015 FusionDirectory

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
            'import', FALSE
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

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
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
    $this->csv_data = $this->parse_csv($this->import);
    /*
     * Add nonrequired field filling feature
     * */
    $this->template_object = new template($this->type, $this->template_dn);
    $attributes = array();
    $tpl_attributes = $this->template_object->serialize();
    foreach ($tpl_attributes as $class => $class_infos) {
      foreach ($class_infos['attrs'] as $attr => $attr_infos) {
        $attributes[$class.':'.$attr_infos['id']] = $class_infos['name'].' > '.$attr_infos['label'];
      }
    }
    $fields = array();
    foreach ($this->csv_data[0] as $key => $field_value) {
      $fields[$key] = $key.' - '.$field_value;
    }
    $fields = array_merge($fields, $this->fixed_values);
    $attributesObjects = array();
    reset($fields);
    foreach ($attributes as $key => $label) {
      $attributesObjects[] =
        new SelectAttribute(
          $label, '',
          'template_'.$key, TRUE,
          array_keys($fields), key($fields),
          array_values($fields)
        );
      next($fields);
    }
    $this->attributesAccess['fields']->setAttributes($attributesObjects);
    $this->attributesAccess['import_submit_fields']->setDisabled(FALSE);
  }

  function handle_import_submit_fields()
  {
    $success = 0;
    foreach ($this->csv_data as $row) {
      $values = array();
      foreach ($this->attributesAccess['fields']->attributes as $attribute) {
        preg_match('/^template_([^:]+):(.*)$/', $attribute->getLdapName(), $m);
        $i = $attribute->getValue();
        if ($i < count($row)) {
          $values[$m[1]][$m[2]] = $row[$i];
        } else {
          $values[$m[1]][$m[2]] = $this->fixed_values[$i - count($row)];
        }
      }
      $this->template_object->deserialize($values);
      $tabObject = $this->template_object->apply();
      $msgs = $tabObject->check();
      if (count($msgs)) {
        foreach ($msgs as $msg) {
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
        }
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
              if (!empty($cell)) {
                $cnt ++;
                $line_data[] = trim($cell);
              }
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
