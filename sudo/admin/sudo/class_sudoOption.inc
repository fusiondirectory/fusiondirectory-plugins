<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class UsedOptionAttribute extends CompositeAttribute
{
  function __construct ($description, $ldapName, $acl = "")
  {
    parent::__construct($description, $ldapName, [], "", "", $acl);
  }

  function addOption ($name, $params, $value = NULL)
  {
    if ($params['TYPE'] == 'LISTS') {
      $this->attributes[$name] = new ListSudoOptionAttribute($name, $params['DESC'], $params['DEFAULT'], $this->getAcl());
    } else {
      $this->attributes[$name] = new SudoOptionAttribute($name, $params['DESC'], $params['TYPE'], $params['DEFAULT'], $this->getAcl());
    }
    if ($value !== NULL) {
      $this->attributes[$name]->setValue($value);
    }
    $this->attributes[$name]->setParent($this->plugin);
  }

  function loadAttrValue (array $attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $values = $attrs[$this->getLdapName()];
      unset($values['count']);
      foreach ($values as $opt) {
        /* Get negation */
        $negation = FALSE;
        if (preg_match("/^!/", $opt)) {
          $negation = TRUE;
          $opt = preg_replace("/^!/", "", $opt);
        }

        /* Get value / name*/
        $value    = "";
        if (preg_match("/=/", $opt)) {
          $value  = preg_replace("/^[^=]*+=/", "", $opt);
          $opt    = preg_replace("/=.*$/", "", $opt);
        }

        /* Special chars are escaped, remove escape char now.
            \\ => \
            \: => :
            \, => ,
            \= => =
         */
        $value = $this->plugin->unescape_command($value);

        $opt = preg_replace(['/\-$/','/\+$/'], ['_moins','_plus'], $opt);

        /* Check if the given value is part of our options list.
           If it is not, add it as type STRING and display a warning.
        */
        if (!sudoOptions::exists($opt)) {
          msg_dialog::display(
            _("Unknown option"),
            sprintf(_("The sudo option '%s' is invalid!"), $opt),
            WARNING_DIALOG
          );
        } else {
          $this->addOption($opt, sudoOptions::get($opt), ['negate' => $negation,'value' => $value]);
        }
      }
    }
  }

  function readValues ($value)
  {
    return $value;
  }

  function writeValues (array $values)
  {
    return array_values($values);
  }

  function loadPostValue ()
  {
    foreach ($_POST as $name => $value) {
      if (preg_match("/^delOption_/", $name)) {
        $opt = preg_replace("/^delOption_/", "", $name);
        $opt = preg_replace("/_[^_]*$/", "", $opt);
        if (isset($this->attributes[$opt])) {
          unset($this->attributes[$opt]);
        }
      }
    }
    parent::loadPostValue();
  }
}

class AvailableSudoOptionAttribute extends SelectAttribute
{
  function __construct ($label, $description, $ldapName, $required = TRUE, $acl = "")
  {
    $choices = sudoOptions::listOptions();
    sort($choices);
    parent::__construct($label, $description, $ldapName, $required, $choices, "", NULL, $acl);
    $this->setInLdap(FALSE);
  }

  function renderFormInput ()
  {
    $id = $this->getLdapName();
    $button = '<input type="submit" formnovalidate="formnovalidate" name="add'.$id.'" id="add'.$id.'"'.
                ($this->disabled ? ' disabled="disabled"' : '').
                ' value="{msgPool type=addButton}"/>';
    $display = parent::renderFormInput();
    $smarty = get_smarty();
    $id = $this->getLdapName();
    $choices = array_diff($this->choices, array_keys($this->plugin->attributesInfo['used_options']['attrs']['sudoOption']->attributes));
    $smarty->assign($id."_choices", $choices);
    $outputs = array_map(
      function ($choice)
      {
        $opt = preg_replace(['/_moins$/','/_plus$/'], ['-','+'], $choice);
        return sprintf(_("%s (%s)"), $opt, sudoOptions::get($choice)['TYPE']);
      },
      $choices
    );
    $smarty->assign($id."_outputs", $outputs);

    return $display.$this->renderAcl($button);
  }

  function loadPostValue ()
  {
    $this->postValue = $this->value;
    if (isset($_POST["add".$this->getLdapName()])) {
      parent::loadPostValue();
      parent::applyPostValue();
      $this->plugin->attributesInfo['used_options']['attrs']['sudoOption']->addOption($this->getValue(), sudoOptions::get($this->getValue()));
    }
  }
}

class ListSudoOptionAttribute extends SetAttribute
{
  function __construct ($name, $desc, $defaultValue, $acl = "")
  {
    parent::__construct(new StringAttribute(preg_replace(['/_moins$/','/_plus$/'], ['-','+'], $name), $desc, $name, FALSE, $defaultValue, $acl));
  }

  function getValue ()
  {
    if (count($this->value) <= 0) {
      return preg_replace(['/_moins$/','/_plus$/'], ['-','+'], $this->getLdapName()).'=';
    } elseif (count($this->value) > 1) {
      return preg_replace(['/_moins$/','/_plus$/'], ['-','+'], $this->getLdapName()).'="'.implode(' ', $this->value).'"';
    } else {
      return preg_replace(['/_moins$/','/_plus$/'], ['-','+'], $this->getLdapName()).'='.$this->value[0];
    }
  }

  function computeLdapValue ()
  {
    return $this->getValue();
  }

  function setValue ($value)
  {
    if (isset($value['value'])) {
      $value = $value['value'];
      if ($value == "") {
        $value = [];
      } elseif ($value[0] != '"') {
        $value = [$value];
      } else {
        $value = preg_split('/ /',
          preg_replace(['/^"/','/"$/'], '', $value)
        );
      }
    }
    parent::setValue($value);
  }
}

class SudoOptionAttribute extends Attribute
{
  private $type;

  function __construct ($name, $desc, $type, $defaultValue, $acl = "")
  {
    parent::__construct($name, $desc, $name, FALSE, ['negate' => FALSE, 'value' => $defaultValue], $acl);
    $this->type = $type;
  }

  function loadPostValue ()
  {
    if (in_array($this->type, ['STRING','INTEGER','BOOLEAN'])) {
      if (isset($_POST[$this->getLdapName()])) {
        $this->setPostValue(['negate' => FALSE, 'value' => $_POST[$this->getLdapName()]]);
      }
    } else {
      if (isset($_POST[$this->getLdapName().'_selection'])) {
        $sel = get_post($this->getLdapName().'_selection');
        $val = "";
        if (isset($_POST[$this->getLdapName().'_value'])) {
          $val = get_post($this->getLdapName().'_value');
        }

        if ($sel == "FALSE" || $sel == "TRUE") {
          $this->setPostValue(['negate' => FALSE, 'value' => $sel]);
        } else {
          $this->setPostValue(['negate' => $this->postValue['negate'], 'value' => $val]);
        }
      }
    }
  }

  function check (): array
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } elseif ($this->type == "INTEGER") {
      if (!is_numeric($this->value) && !empty($this->value)) {
        return msgPool::invalid($this->getLabel(), $this->value);
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getLdapName();
    $value = htmlentities($this->value['value'], ENT_COMPAT, 'UTF-8');
    $smarty = get_smarty();
    $smarty->assign("boolStringOptions", ["FALSE","TRUE","STRING"]);
    $smarty->assign("boolIntOptions", ["FALSE","TRUE","INTEGER"]);
    $smarty->assign("booleanOptions", ["FALSE","TRUE"]);
    if ($this->type == "STRING") {
      $display = '<input type="text" name="'.$id.'" id="'.$id.'" value="'.$value.'" />';
    } elseif ($this->type == "LISTS") {
      $display = '<input type="text" name="'.$id.'" id="'.$id.'" value="'.$value.'" />';
    } elseif ($this->type == "INTEGER") {
      $display = '<input type="number" name="'.$id.'" id="'.$id.'" value="'.$value.'" />';
    } elseif ($this->type == "BOOLEAN") {
      $display = '<select name="'.$id.'" id="'.$id.'">
        {html_options values=$booleanOptions output=$booleanOptions selected="'.$value.'"}
      </select>';
    } elseif (($this->type == "BOOL_INTEGER") || ($this->type == "STRING_BOOL")) {
      $selected = ($value == "FALSE" || $value == "TRUE" ? $value : "STRING");
      $display = '<select name="'.$id.'_selection" id="'.$id.'_selection"
        onChange="toggle_bool_fields(\''.$id.'_selection\',\''.$id.'_value\');">
        {html_options values=$boolStringOptions output=$bool'.
        ($this->type == "BOOL_INTEGER" ? 'Int' : 'String').
        'Options selected="'.$selected.'"}
      </select>';
      $display .= '<input type="text" name="'.$id.'_value" id="'.$id.'_value" value="'.$value.'"'.
          ($selected == "STRING" ? '' : 'disabled="disabled"').'/>';
    }
    return $this->renderAcl($display);
  }

  function setValue ($value)
  {
    if ($value['value'] == '') {
      $value['value']   = ($value['negate'] ? 'FALSE' : 'TRUE');
      $value['negate']  = FALSE;
    }
    parent::setValue($value);
  }

  function getValue ()
  {
    /* Escape special chars */
    $name   = $this->getLdapName();
    $value  = $this->plugin->escape_command($this->value['value']);
    $neg    = $this->value['negate'];

    if ($this->type == "BOOLEAN") {
      $option = $name;
      if ($value == "FALSE") {
        $option = "!".$option;
      }
    } elseif (in_array($this->type, ["STRING","INTEGER","LISTS"])) {
      if ($value == "") {
        $option = $name;
      } else {
        $option = $name."=".$value;
      }
      if ($neg) {
        $option = "!".$option;
      }
    } elseif (in_array($this->type, ["STRING_BOOL","BOOL_INTEGER"])) {
      if ($value == "FALSE") {
        $option = "!".$name;
      } elseif ($value == "TRUE") {
        $option = $name;
      } else {
        if ($value != "") {
          $option = $name."=".$value;
        } else {
          $option = $name;
        }
        if ($neg) {
          $option = "!".$option;
        }
      }
    }

    return $option;
  }
}

/*! \brief  Sudo option class.
            Allows setting flags/options for a sudo role.
 */
class sudoOption extends simplePlugin
{
  var $displayHeader = FALSE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Options'),
      'plDescription' => _('Sudo options'),
      'plObjectType'  => ['sudo'],
      'plPriority'    => 2,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'available_options' => [
        'name'  => _('Available options'),
        'attrs' => [
          new AvailableSudoOptionAttribute(
            _('Option'), _('Add a new sudo option'), 'availableSudoOptions'
          ),
        ]
      ],
      'used_options' => [
        'name'  => _('Used sudo role options'),
        'attrs' => [
          new UsedOptionAttribute(_('Used sudo role options'), 'sudoOption')
        ],
        'template' => get_template_path('usedoptions_section.tpl', TRUE, dirname(__FILE__))
      ],
    ];
  }

  function execute (): string
  {
    $script = <<<SCRIPT
  <script type="text/javascript">
   <!--
    function toggle_bool_fields (source_select, target_input)
    {
     var select= document.getElementById(source_select);
     var input = document.getElementById(target_input);
     if (select.value == "TRUE" || select.value == "FALSE") {
      input.disabled = true;
      input.value = select.value;
     } else {
      input.disabled = false;
      input.value = "";
     }
    }
   -->
  </script>
SCRIPT;
    return $script.parent::execute();
  }

  /*!  \brief   Escape special chars in function parameters.
       @param   String the string to that must be escaped.
   */
  public function escape_command ($str)
  {
    /* Check if given value is a command (/[a-z]/ ..)
     */
    if (preg_match("/^\//", $str)) {
      $cmd = preg_replace("/^([^ ]*).*$/", "\\1", $str);
      $val = preg_replace("/^[^ ]*(.*)$/", "\\1", $str);
      $str = $cmd.addcslashes($val, ":.,\\");
    }
    return $str;
  }

  /*!  \brief   Unescape special chars in function parameters.
       @param   String the string to that must be unescaped.
   */
  public function unescape_command ($str)
  {
    /* Check if given value is a command (/[a-z]/ ..)
     */
    if (preg_match("/^\//", $str)) {
      $cmd = preg_replace("/^([^ ]*).*$/", "\\1", $str);
      $val = preg_replace("/^[^ ]*(.*)$/", "\\1", $str);
      $val = preg_replace(
        ["/\\\\\\\\/", "/\\\\,/", "/\\\\:/", "/\\\\=/"],
        ["\\",",",":","="], $val
      );
      $str = $cmd.$val;
    }
    return $str;
  }
}
