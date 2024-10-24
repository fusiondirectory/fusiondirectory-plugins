<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

/**
 *
 */

require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');

class reminderFrontEnd extends standAlonePage
{
  private string $token;
  private array  $task;
  private string $uid;

  /**
   * @return bool
   * Note : method from extension.
   * It will help to get the information from the main task.
   * It is also run before class execute during first arrival on the page. IF the a post is issued, it will directly go to
   * execute.
   */
  protected function readLdapConfig (): bool
  {
    $result = FALSE;

    return $this->sanitizeInputs();
  }


  // attributesName to be taken
  /* - EPPN eduPersonPrincipalName
  /  - UID
  /  - mail
  /  - alias supannAliasLogin
  */

  function execute ()
  {
    global $error_collector;
    $success = FALSE;
    $error =  FALSE;

    if (!empty($_POST)) {
      $this->sanitizeInputs();
      $this->getTaskInfo();
    }

    // This is a protection in case the reading of ldap failed (from readLdapConfig). (Previous logic from userReminder).
    if (!$this->activated) {
      return;
    }

    // Verify if uid is set (thanks to post) and verify token validity.
    if (!empty($this->uid)) {
      // verification of token
      if (!$this->checkToken($this->token)) {
        $this->message[] = "Error, data received are incorrect!";
      } else {
        $success = TRUE;
      }

    }

    if (!empty($this->message)) {
      msg_dialog::displayChecks($this->message);
      $error = TRUE;
    }

    // Initialize smarty and pass the status of potential error ??
    $this->initSmarty($success, $error);

  }

  /**
   * @param bool $success
   * @param bool $error
   * @return void
   * Note : success allows to display certain part of the tpl '<div>' element.
   */
  private function initSmarty (bool $success, bool $error)
  {
    $smarty = get_smarty();

    $smarty->assign('PHPSESSID', session_id());
    if (session::is_set('errors')) {
      $smarty->assign('errors', session::get('errors'));
    } else {
      $smarty->assign('php_errors', '');
    }

    $smarty->assign('msg_dialogs', msg_dialog::get_dialogs());
    $smarty->assign('usePrototype', 'FALSE');
    $smarty->append('js_files', 'include/pwdStrength.js');
    $smarty->append('css_files', get_template_path('login.css'));
    $lang = session::global_get('lang');
    $smarty->assign('lang', preg_replace('/_.*$/', '', $lang));
    $smarty->assign('rtl', Language::isRTL($lang));
    $smarty->display(get_template_path('headers.tpl'));

    $smarty->assign('version', FD_VERSION);
    $smarty->assign('activated', $this->activated);
    $smarty->assign('success', $success);
    $smarty->assign('error', $error);
    $smarty->assign('taskName', $this->task['name']);
    $smarty->assign('token', $this->token);
    $smarty->display(get_template_path('taskReminder.tpl'));
  }

  /**
   * @param $token
   * @return bool
   * NOTE : To be revamp as this old logic seems a bit off to me.
   */
  private function checkToken ($token): bool
  {
    global $config;
    $result = FALSE;

    /* Retrieve hash from the ldap */
    $ldap = $config->get_ldap_link();

    $dn = 'cn=' . $this->uid . ',ou=tokens,' . $config->current['BASE'];
    $ldap->cat($dn);

    if ($attrs = $ldap->fetch()) {
      $ldapToken = $attrs['fdToken'][0];
      $tokenDate = $attrs['fdTokenTimestamp'][0];
      $tokenType = $attrs['fdTokenType'][0];

      // Check timestamp in UTC format
      if ($ldapToken === $token && $tokenType === 'reminder') {
        if (time() < $tokenDate) {
          $result = TRUE;
        }
      }
    }
    return $result;
  }

  /**
   * @return void
   * Note : Simple deletion of token
   */
  function deleteToken ()
  {
    global $config;

    $ldap = $config->get_ldap_link();

    $dn = 'ou=' . $this->uid . ',ou=tokens' . $config->current['BASE'];
    $ldap->rmdir($dn);
    if (!$ldap->success()) {
      $this->message[] = $ldap->get_error();
    }
  }

  /**
   * @return void
   * Note : Simply set global variables after sanitization.
   */
  private function sanitizeInputs (): bool
  {
    $result = FALSE;

    if ($_SERVER['REQUEST_METHOD'] == 'GET' && isset($_GET['token']) && isset($_GET['task'])) {
      // Sanitize and validate input fields
      $token = filter_input(INPUT_GET, 'token', FILTER_SANITIZE_STRING);
      $task  = filter_input(INPUT_GET, 'task', FILTER_SANITIZE_STRING);

      // Function to validate if inputs are strictly alphanumeric
      function is_alphanumeric ($value)
      {
        return preg_match('/^[a-zA-Z0-9]+$/', $value);
      }

      // Validate 'token'
      if (!is_alphanumeric($token) || !is_alphanumeric($task)) {
        die("Invalid values within passed arguments...");
      }

      // Assign the sanitized values
      $this->token = $token;
      // Simply set the $task representing the name of main task to an array (Simplify further process down the line).
      $this->task['name'] = $task;

      $result = TRUE;
    }

    // Case of the post submission
    if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['login'])) {
      //
      $uid = filter_input(INPUT_POST, 'login', FILTER_SANITIZE_STRING);

      // Function to validate if inputs are strictly alphanumeric
      function is_alphanumeric ($value)
      {
        return preg_match('/^[a-zA-Z0-9]+$/', $value);
      }

      // Validate 'token'
      if (!is_alphanumeric($uid)) {
        die("Invalid values within passed arguments...");
      }

      $this->uid = $uid;
      $result    = TRUE;
    }

    return $result;
  }

  /**
   * @return void
   */
  private function getTaskInfo (): bool
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $result = FALSE;

    if (!empty($this->task['name'])) {
      $dn = 'cn=' . $this->task['name'] . ',ou=tasks,' . $config->current['BASE'];
      // Retrieve tasks information
      $ldap->cat($dn);
      if ($attrs = $ldap->fetch()) {
        // Fills the required value here.
        $this->task['posix']    = $attrs['fdTasksReminderPosix'][0]; // bool as string
        $this->task['ppolicy']  = $attrs['fdTasksReminderPPolicy'][0]; // bool as string
        $this->task['resource'] = $attrs['fdTasksReminderNextResource'][0]; //string
        $this->task['days']     = $attrs['fdTasksReminderSupannNewEndDate'][0]; // int
        $this->task['state']    = $attrs['fdTasksReminderNextState'][0]; // string
        $this->task['subState'] = $attrs['fdTasksReminderNextSubState'][0]; // string

        $result = TRUE;
      }
    }
    return $result;
  }
}


