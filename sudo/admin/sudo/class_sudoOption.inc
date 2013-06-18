<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

/*

 * TODO
 * Find a way to translate descriptions
 */

class UsedOptionAttribute extends CompositeAttribute
{
  function __construct ($description, $ldapName, $acl = "")
  {
    parent::__construct($description, $ldapName, array(), "", "", $acl);
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

  function loadAttrValue ($attrs)
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

        $opt = preg_replace(array('/\-$/','/\+$/'),array('_moins','_plus'),$opt);

        /* Check if the given value is part of our options list.
           If it is not, add it as type STRING and display a warning.
        */
        if (!isset(AvailableSudoOptionAttribute::$options[$opt])) {
          msg_dialog::display(
            _("Unknown option"),
            sprintf(_("The sudo option '%s' is invalid!"), $opt),
            WARNING_DIALOG
          );
        } else {
          $this->addOption($opt, AvailableSudoOptionAttribute::$options[$opt], array('negate' => $negation,'value' => $value));
        }
      }
    }
  }

  function readValues ($value)
  {
    return $value;
  }

  function writeValues ($values)
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
  public static $options = array(
    'long_otp_prompt'       => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'When validating with a One Time Password (OTP) scheme such as S/Key or OPIE,
    a two-line prompt is used to make it easier to cut and paste the challenge to a local window'),
    'ignore_dot'            => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => 'If set, sudo will ignore \'.\' or \'\' (current dir) in the PATH environment variable'),
    'mail_always'           => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'Send mail to the mailto user every time a users runs sudo'),
    'mail_badpass'          => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'Send mail to the mailto user if the user running sudo does not enter the correct password'),
    'mail_no_user'          => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => 'If set, mail will be sent to the mailto user if the invoking user is not in the sudoers file'),
    'mail_no_host'          => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, mail will be sent to the mailto user if the invoking user exists in the sudoers file, but is not allowed to run commands on the current host'),
    'mail_no_perms'         => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, mail will be sent to the mailto user if the invoking user is allowed to use sudo but not for this command'),
    'tty_tickets'           => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => 'If set, users must authenticate on a per-tty basis'),
    'authenticate'          => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => 'If set, users must authenticate themselves via a password (or other means of authentication)'),
    'root_sudo'             => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => 'If set, root is allowed to run sudo too'),
    'log_host'              => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, the host name will be logged in the (non-syslog) sudo log file'),
    'log_year'              => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, the four-digit year will be logged in the (non-syslog) sudo log file'),
    'shell_noargs'          => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set and sudo is invoked with no arguments it acts as if the -s option had been given'),
    'set_home'              => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If enabled and sudo is invoked with the -s option the HOME environment variable will be set to the home directory of the target user (usually root)'),
    'always_set_home'       => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If enabled, sudo will set the HOME environment variable to the home directory of the target user (usually root)'),
    'path_info'             => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If the executable is simply not in the user\'s PATH, sudo will tell the user that they are not allowed to run it'),
    'preserve_groups'       => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, the user\'s existing group vector is left unaltered'),
    'fqdn'                  => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'Set this flag if you want to put fully qualified host names in the sudoers file'),
    'insults'               => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, sudo will insult users when they enter an incorrect password'),
    'requiretty'            => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, sudo will only run when the user is logged in to a real tty'),
    'env_editor'            => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => 'If set, visudo will use the value of the EDITOR or VISUAL environment variables before falling back on the default editor list'),
    'rootpw'                => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, sudo will prompt for the root password instead of the password of the invoking user'),
    'runaspw'               => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, sudo will prompt for the password of the user defined by the runas_default option (defaults to root) instead of the password of the invoking user'),
    'targetpw'              => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, sudo will prompt for the password of the user specified by the -u option (defaults to root) instead of the password of the invoking user'),
    'set_logname'           => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => 'If set, sudo will set the LOGNAME environment variables to the name of the target user'),
    'stay_setuid'           => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => ''),
    'env_reset'             => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'TRUE',
    'DESC' => 'If set, sudo will run the command in a minimal environment containing the TERM, PATH, HOME, MAIL, SHELL, LOGNAME, USER, USERNAME and SUDO_ variables'),
    'use_loginclass'        => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, sudo will apply the defaults specified for the target user\'s login class if one exists'),
    'noexec'                => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, all commands run via sudo will behave as if the NOEXEC tag has been set, unless overridden by a EXEC tag'),
    'ignore_local_sudoers'  => array('TYPE' => 'BOOLEAN',       'DEFAULT' => 'FALSE',
    'DESC' => 'If set, parsing of /etc/sudoers file will be skipped.'),
    'passwd_tries'          => array('TYPE' => 'INTEGER',       'DEFAULT' => 3,
    'DESC' => 'The number of tries a user gets to enter his/her password before sudo logs the failure and exits'),
    'loglinelen'            => array('TYPE' => 'BOOL_INTEGER',  'DEFAULT' => 80,
    'DESC' => 'Number of characters per line for the file log'),
    'timestamp_timeout'     => array('TYPE' => 'BOOL_INTEGER',  'DEFAULT' => 0,
    'DESC' => 'Number of minutes that can elapse before sudo will ask for a passwd again'),
    'passwd_timeout'        => array('TYPE' => 'BOOL_INTEGER',  'DEFAULT' => 15,
    'DESC' => 'Number of minutes before the sudo password prompt times out, or 0 for no timeout'),
    'umask'                 => array('TYPE' => 'BOOL_INTEGER',  'DEFAULT' => "0022",
    'DESC' => 'Umask to use when running the command. Set to FALSE to preserve the user\'s umask'),
    'mailsub'               => array('TYPE' => 'STRING',        'DEFAULT' => '*** SECURITY information for %h ***',
    'DESC' => 'Subject of the mail sent to the mailto user. The escape %h will expand to the host name of the machine'),
    'badpass_message'       => array('TYPE' => 'STRING',        'DEFAULT' => 'Sorry, try again',
    'DESC' => 'Message that is displayed if a user enters an incorrect password'),
    'timestampdir'          => array('TYPE' => 'STRING',        'DEFAULT' => '/var/run/sudo',
    'DESC' => 'The directory in which sudo stores its timestamp files'),
    'timestampowner'        => array('TYPE' => 'STRING',        'DEFAULT' => 'root',
    'DESC' => 'The owner of the timestamp directory and the timestamps stored therein'),
    'passprompt'            => array('TYPE' => 'STRING',        'DEFAULT' => '[sudo] password for %p: ',
    'DESC' => 'The default prompt to use when asking for a password; can be overridden via the -p option or the SUDO_PROMPT environment variable'),
    'runas_default'         => array('TYPE' => 'STRING',        'DEFAULT' => 'root',
    'DESC' => 'The default user to run commands as if the -u option is not specified on the command line'),
    'syslog_goodpri'        => array('TYPE' => 'STRING',        'DEFAULT' => 'notice',
    'DESC' => 'Syslog priority to use when user authenticates successfully'),
    'syslog_badpri'         => array('TYPE' => 'STRING',        'DEFAULT' => 'alert',
    'DESC' => 'Syslog priority to use when user authenticates unsuccessfully'),
    'editor'                => array('TYPE' => 'STRING',        'DEFAULT' => '/usr/bin/vi',
    'DESC' => 'A colon separated list of editors allowed to be used with visudo'),
    'noexec_file'           => array('TYPE' => 'STRING',        'DEFAULT' => '/usr/lib/sudo/sudo_noexec.so',
    'DESC' => ''),
    'lecture'               => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => 'once',
    'DESC' => 'This option controls when a short lecture will be printed along with the password prompt'),
    'lecture_file'          => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => '',
    'DESC' => 'Path to a file containing an alternate sudo lecture that will be used in place of the standard lecture'),
    'logfile'               => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => 'syslog',
    'DESC' => 'Path to the sudo log file (not the syslog log file)'),
    'syslog'                => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => 'authpriv',
    'DESC' => 'Syslog facility if syslog is being used for logging'),
    'mailerpath'            => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => '',
    'DESC' => 'Path to mail program used to send warning mail'),
    'mailerflags'           => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => '-t',
    'DESC' => 'Flags to use when invoking mailer'),
    'mailto'                => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => 'root',
    'DESC' => 'Address to send warning and error mail to'),
    'exempt_group'          => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => 'root',
    'DESC' => 'Users in this group are exempt from password and PATH requirements'),
    'verifypw'              => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => 'all',
    'DESC' => 'This option controls when a password will be required when a user runs sudo with the -v option'),
    'listpw'                => array('TYPE' => 'STRING_BOOL',   'DEFAULT' => 'any',
    'DESC' => 'This option controls when a password will be required when a user runs sudo with the -l option'),
    'env_check'             => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be removed from the user\'s environment if the variable\'s value contains % or / characters'),
    'env_delete'            => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be removed from the user\'s environment when the env_reset option is not in effect'),
    'env_keep'              => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be preserved in the user\'s environment when the env_reset option is in effect'),
    'env_check_moins'       => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be removed from the user\'s environment if the variable\'s value contains % or / characters'),
    'env_delete_moins'      => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be removed from the user\'s environment when the env_reset option is not in effect'),
    'env_keep_moins'        => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be preserved in the user\'s environment when the env_reset option is in effect'),
    'env_check_plus'        => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be removed from the user\'s environment if the variable\'s value contains % or / characters'),
    'env_delete_plus'       => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be removed from the user\'s environment when the env_reset option is not in effect'),
    'env_keep_plus'         => array('TYPE' => 'LISTS',         'DEFAULT' => '',
    'DESC' => 'Environment variables to be preserved in the user\'s environment when the env_reset option is in effect')
  );

  function __construct ($label, $description, $ldapName, $required = TRUE, $acl = "")
  {
    $choices = array_keys(self::$options);
    sort($choices);
    parent::__construct ($label, $description, $ldapName, $required, $choices, "", NULL, $acl);
    $this->setInLdap(FALSE);
  }

  function renderFormInput ()
  {
    $id = $this->getLdapName();
    $button = '<input type="submit" name="add'.$id.'" id="add'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=addButton}"/>';
    $display = parent::renderFormInput();
    $smarty = get_smarty();
    $id = $this->getLdapName();
    $choices = array_diff($this->choices, array_keys($this->plugin->attributesInfo['used_options']['attrs']['sudoOption']->attributes));
    $smarty->assign($id."_choices", $choices);
    $options = self::$options;
    $outputs = array_map(
      function ($choice) use ($options)
      {
        $opt = preg_replace(array('/_moins$/','/_plus$/'),array('-','+'),$choice);
        return sprintf(_("%s (%s)"), $opt, $options[$choice]['TYPE']);
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
      $this->plugin->attributesInfo['used_options']['attrs']['sudoOption']->addOption($this->getValue(), self::$options[$this->getValue()]);
    }
  }
}

class ListSudoOptionAttribute extends SetAttribute
{
  function __construct ($name, $desc, $defaultValue, $acl = "")
  {
    parent::__construct(new StringAttribute(preg_replace(array('/_moins$/','/_plus$/'),array('-','+'),$name), $desc, $name, FALSE, $defaultValue, $acl));
  }

  function getValue ()
  {
    if (count($this->value) <= 0) {
      return preg_replace(array('/_moins$/','/_plus$/'),array('-','+'),$this->getLdapName()).'=';
    } elseif (count($this->value) > 1) {
      return preg_replace(array('/_moins$/','/_plus$/'),array('-','+'),$this->getLdapName()).'="'.implode(' ',$this->value).'"';
    } else {
      return preg_replace(array('/_moins$/','/_plus$/'),array('-','+'),$this->getLdapName()).'='.$this->value[0];
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
        $value = array();
      } elseif ($value[0] != '"') {
        $value = array($value);
      } else {
        $value = preg_split('/ /',
          preg_replace(array('/^"/','/"$/'), '', $value)
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
    parent::__construct($name, $desc, $name, FALSE, array('negate' => FALSE, 'value' => $defaultValue), $acl);
    $this->type = $type;
  }

  function loadPostValue ()
  {
    if (in_array($this->type, array('STRING','INTEGER','BOOLEAN'))) {
      if (isset($_POST[$this->getLdapName()])) {
        $this->setPostValue(array('negate' => FALSE, 'value' => $_POST[$this->getLdapName()]));
      }
    } else {
      if (isset($_POST[$this->getLdapName().'_selection'])) {
        $sel = get_post($this->getLdapName().'_selection');
        $val = "";
        if (isset($_POST[$this->getLdapName().'_value'])) {
          $val = get_post($this->getLdapName().'_value');
        }

        if ($sel == "FALSE" || $sel == "TRUE") {
          $this->setPostValue(array('negate' => FALSE, 'value' => $sel));
        } else {
          $this->setPostValue(array('negate' => $this->postValue['negate'], 'value' => $val));
        }
      }
    }
  }

  function check ()
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
    $smarty->assign("boolStringOptions", array("FALSE","TRUE","STRING"));
    $smarty->assign("boolIntOptions", array("FALSE","TRUE","INTEGER"));
    $smarty->assign("booleanOptions", array("FALSE","TRUE"));
    if ($this->type == "STRING") {
      $display = '<input type="text" name="'.$id.'" id="'.$id.'" value="'.$value.'" />';
    } elseif ($this->type == "LISTS") { //FIXME : LIST must be handle in an other way
      $display = '<input type="text" name="'.$id.'" id="'.$id.'" value="'.$value.'" />';
    } elseif ($this->type == "INTEGER") {
      $display = '<input type="number" name="'.$id.'" id="'.$id.'" value="'.$value.'" />';
    } elseif ($this->type == "BOOLEAN") {
      $display = '<select name="'.$id.'" id="'.$id.'">
        {html_options values=$booleanOptions output=$booleanOptions selected="'.$value.'"}
      </select>';
    } elseif (($this->type == "BOOL_INTEGER") || ($this->type == "STRING_BOOL")) {
      $selected = ($value == "FALSE" || $value == "TRUE" ?$value:"STRING");
      $display = '<select name="'.$id.'_selection" id="'.$id.'_selection"
        onChange="toggle_bool_fields(\''.$id.'_selection\',\''.$id.'_value\');">
        {html_options values=$boolStringOptions output=$bool'.
        ($this->type == "BOOL_INTEGER"?'Int':'String').
        'Options selected="'.$selected.'"}
      </select>';
      $display .= '<input type="text" name="'.$id.'_value" id="'.$id.'_value" value="'.$value.'"'.
          ($selected == "STRING"?'':'disabled="disabled"').'/>';
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
    } elseif (in_array($this->type, array("STRING","INTEGER","LISTS"))) {
      if ($value == "") {
        $option = $name;
      } else {
        $option = $name."=".$value;
      }
      if ($neg) {
        $option = "!".$option;
      }
    } elseif (in_array($this->type, array("STRING_BOOL","BOOL_INTEGER"))) {
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
  var $objectclasses = array();

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'available_options' => array(
        'name'  => _('Available options'),
        'attrs' => array(
          new AvailableSudoOptionAttribute (
            _('Option'), _('Add a new sudo option'), 'availableSudoOptions'
          ),
        )
      ),
      'used_options' => array(
        'name'  => _('Used sudo role options'),
        'attrs' => array(
          new UsedOptionAttribute (_('Used sudo role options'), 'sudoOption')
        ),
        'template' => get_template_path('usedoptions_section.tpl', TRUE, dirname(__FILE__))
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }

  function execute ()
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
  public function escape_command($str)
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
  public function unescape_command($str)
  {
    /* Check if given value is a command (/[a-z]/ ..)
     */
    if (preg_match("/^\//", $str)) {
      $cmd = preg_replace("/^([^ ]*).*$/", "\\1", $str);
      $val = preg_replace("/^[^ ]*(.*)$/", "\\1", $str);
      $val = preg_replace(
        array("/\\\\\\\\/", "/\\\\,/", "/\\\\:/", "/\\\\=/"),
        array("\\",",",":","="), $val
      );
      $str = $cmd.$val;
    }
    return $str;
  }

  static function plInfo ()
  {
    return array(
      'plShortName'   => _('Options'),
      'plDescription' => _('Sudo options'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('sudo'),
      'plObjectType'  => array('sudo'),
      'plPriority'    => 2,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}
?>
