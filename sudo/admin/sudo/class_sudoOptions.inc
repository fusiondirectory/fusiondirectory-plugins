<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2017  FusionDirectory

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

class sudoOptions
{
  private static $options = NULL;

  public static function exists($option)
  {
    static::initOptionsArray();
    return isset(static::$options[$option]);
  }

  public static function get($option)
  {
    static::initOptionsArray();
    return static::$options[$option];
  }

  public static function listOptions()
  {
    static::initOptionsArray();
    return array_keys(static::$options);
  }

  private static function initOptionsArray()
  {
    if (static::$options !== NULL) {
      return;
    }
    static::$options = array(
      'long_otp_prompt'       => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('When validating with a One Time Password (OTP) scheme such as S/Key or OPIE, a two-line prompt is used to make it easier to cut and paste the challenge to a local window')
      ),
      'ignore_dot'            => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => _('If set, sudo will ignore \'.\' or \'\' (current dir) in the PATH environment variable')
      ),
      'mail_always'           => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('Send mail to the mailto user every time a users runs sudo')
      ),
      'mail_badpass'          => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('Send mail to the mailto user if the user running sudo does not enter the correct password')
      ),
      'mail_no_user'          => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => _('If set, mail will be sent to the mailto user if the invoking user is not in the sudoers file')
      ),
      'mail_no_host'          => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, mail will be sent to the mailto user if the invoking user exists in the sudoers file, but is not allowed to run commands on the current host')
      ),
      'mail_no_perms'         => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, mail will be sent to the mailto user if the invoking user is allowed to use sudo but not for this command')
      ),
      'tty_tickets'           => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => _('If set, users must authenticate on a per-tty basis')
      ),
      'authenticate'          => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => _('If set, users must authenticate themselves via a password (or other means of authentication)')
      ),
      'root_sudo'             => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => _('If set, root is allowed to run sudo too')
      ),
      'log_host'              => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, the host name will be logged in the (non-syslog) sudo log file')
      ),
      'log_year'              => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, the four-digit year will be logged in the (non-syslog) sudo log file')
      ),
      'shell_noargs'          => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set and sudo is invoked with no arguments it acts as if the -s option had been given')
      ),
      'set_home'              => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If enabled and sudo is invoked with the -s option the HOME environment variable will be set to the home directory of the target user (usually root)')
      ),
      'always_set_home'       => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If enabled, sudo will set the HOME environment variable to the home directory of the target user (usually root)')
      ),
      'path_info'             => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If the executable is simply not in the user\'s PATH, sudo will tell the user that they are not allowed to run it')
      ),
      'preserve_groups'       => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, the user\'s existing group vector is left unaltered')
      ),
      'fqdn'                  => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('Set this flag if you want to put fully qualified host names in the sudoers file')
      ),
      'insults'               => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, sudo will insult users when they enter an incorrect password')
      ),
      'requiretty'            => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, sudo will only run when the user is logged in to a real tty')
      ),
      'env_editor'            => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => _('If set, visudo will use the value of the EDITOR or VISUAL environment variables before falling back on the default editor list')
      ),
      'rootpw'                => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, sudo will prompt for the root password instead of the password of the invoking user')
      ),
      'runaspw'               => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, sudo will prompt for the password of the user defined by the runas_default option (defaults to root) instead of the password of the invoking user')
      ),
      'targetpw'              => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, sudo will prompt for the password of the user specified by the -u option (defaults to root) instead of the password of the invoking user')
      ),
      'set_logname'           => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => _('If set, sudo will set the LOGNAME environment variables to the name of the target user')
      ),
      'stay_setuid'           => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => ''
      ),
      'env_reset'             => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'TRUE',
        'DESC'    => _('If set, sudo will run the command in a minimal environment containing the TERM, PATH, HOME, MAIL, SHELL, LOGNAME, USER, USERNAME and SUDO_ variables')
      ),
      'use_loginclass'        => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, sudo will apply the defaults specified for the target user\'s login class if one exists')
      ),
      'noexec'                => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, all commands run via sudo will behave as if the NOEXEC tag has been set, unless overridden by a EXEC tag')
      ),
      'ignore_local_sudoers'  => array(
        'TYPE'    => 'BOOLEAN',
        'DEFAULT' => 'FALSE',
        'DESC'    => _('If set, parsing of /etc/sudoers file will be skipped.')
      ),
      'passwd_tries'          => array(
        'TYPE'    => 'INTEGER',
        'DEFAULT' => 3,
        'DESC'    => _('The number of tries a user gets to enter his/her password before sudo logs the failure and exits')
      ),
      'loglinelen'            => array(
        'TYPE'    => 'BOOL_INTEGER',
        'DEFAULT' => 80,
        'DESC'    => _('Number of characters per line for the file log')
      ),
      'timestamp_timeout'     => array(
        'TYPE'    => 'BOOL_INTEGER',
        'DEFAULT' => 0,
        'DESC'    => _('Number of minutes that can elapse before sudo will ask for a passwd again')
      ),
      'passwd_timeout'        => array(
        'TYPE'    => 'BOOL_INTEGER',
        'DEFAULT' => 15,
        'DESC'    => _('Number of minutes before the sudo password prompt times out, or 0 for no timeout')
      ),
      'umask'                 => array(
        'TYPE'    => 'BOOL_INTEGER',
        'DEFAULT' => "0022",
        'DESC'    => _('Umask to use when running the command. Set to FALSE to preserve the user\'s umask')
      ),
      'mailsub'               => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => '*** SECURITY information for %h ***',
        'DESC'    => _('Subject of the mail sent to the mailto user. The escape %h will expand to the host name of the machine')
      ),
      'badpass_message'       => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => 'Sorry, try again',
        'DESC'    => _('Message that is displayed if a user enters an incorrect password')
      ),
      'timestampdir'          => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => '/var/run/sudo',
        'DESC'    => _('The directory in which sudo stores its timestamp files')
      ),
      'timestampowner'        => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => 'root',
        'DESC'    => _('The owner of the timestamp directory and the timestamps stored therein')
      ),
      'passprompt'            => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => '[sudo] password for %p: ',
        'DESC'    => _('The default prompt to use when asking for a password; can be overridden via the -p option or the SUDO_PROMPT environment variable')
      ),
      'runas_default'         => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => 'root',
        'DESC'    => _('The default user to run commands as if the -u option is not specified on the command line')
      ),
      'syslog_goodpri'        => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => 'notice',
        'DESC'    => _('Syslog priority to use when user authenticates successfully')
      ),
      'syslog_badpri'         => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => 'alert',
        'DESC'    => _('Syslog priority to use when user authenticates unsuccessfully')
      ),
      'editor'                => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => '/usr/bin/vi',
        'DESC'    => _('A colon separated list of editors allowed to be used with visudo')
      ),
      'noexec_file'           => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => '/usr/lib/sudo/sudo_noexec.so',
        'DESC'    => ''
      ),
      'secure_path'           => array(
        'TYPE'    => 'STRING',
        'DEFAULT' => '/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin',
        'DESC'    => _('Path used for every command run from sudo. If you don’t trust the people running sudo to have a sane PATH environment variable you may want to use this.')
      ),
      'lecture'               => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => 'once',
        'DESC'    => _('This option controls when a short lecture will be printed along with the password prompt')
      ),
      'lecture_file'          => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => '',
        'DESC'    => _('Path to a file containing an alternate sudo lecture that will be used in place of the standard lecture')
      ),
      'logfile'               => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => 'syslog',
        'DESC'    => _('Path to the sudo log file (not the syslog log file)')
      ),
      'syslog'                => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => 'authpriv',
        'DESC'    => _('Syslog facility if syslog is being used for logging')
      ),
      'mailerpath'            => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => '',
        'DESC'    => _('Path to mail program used to send warning mail')
      ),
      'mailerflags'           => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => '-t',
        'DESC'    => _('Flags to use when invoking mailer')
      ),
      'mailto'                => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => 'root',
        'DESC'    => _('Address to send warning and error mail to')
      ),
      'exempt_group'          => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => 'root',
        'DESC'    => _('Users in this group are exempt from password and PATH requirements')
      ),
      'verifypw'              => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => 'all',
        'DESC'    => _('This option controls when a password will be required when a user runs sudo with the -v option')
      ),
      'listpw'                => array(
        'TYPE'    => 'STRING_BOOL',
        'DEFAULT' => 'any',
        'DESC'    => _('This option controls when a password will be required when a user runs sudo with the -l option')
      ),
      'env_check'             => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be removed from the user\'s environment if the variable\'s value contains % or / characters')
      ),
      'env_delete'            => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be removed from the user\'s environment when the env_reset option is not in effect')
      ),
      'env_keep'              => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be preserved in the user\'s environment when the env_reset option is in effect')
      ),
      'env_check_moins'       => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be removed from the user\'s environment if the variable\'s value contains % or / characters')
      ),
      'env_delete_moins'      => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be removed from the user\'s environment when the env_reset option is not in effect')
      ),
      'env_keep_moins'        => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be preserved in the user\'s environment when the env_reset option is in effect')
      ),
      'env_check_plus'        => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be removed from the user\'s environment if the variable\'s value contains % or / characters')
      ),
      'env_delete_plus'       => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be removed from the user\'s environment when the env_reset option is not in effect')
      ),
      'env_keep_plus'         => array(
        'TYPE'    => 'LISTS',
        'DEFAULT' => '',
        'DESC'    => _('Environment variables to be preserved in the user\'s environment when the env_reset option is in effect')
      )
    );
  }
}
