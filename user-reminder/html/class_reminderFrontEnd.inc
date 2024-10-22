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
   * It is also run before class execute.
   */
  protected function readLdapConfig (): bool
  {
    global $config;
    $result = FALSE;

    echo "coucou";

    $ldap = $config->get_ldap_link();
    $this->sanitizeInputs();

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


  // attributesName to be taken
  /* - EPPN eduPersonPrincipalName
  /  - UID
  /  - mail
  /  - alias supannAliasLogin
  */

  function execute ()
  {
    global $error_collector;

    // This is a protection in case the reading of ldap failed (from readLdapConfig). (Previous logic from userReminder).
    if (!$this->activated) {
      return;
    }

    // token is set during sanitization
    if (!empty($this->uid)) {
      $success = TRUE;
    } else {
      $this->message[] = _('This token is invalid or missing!');
    }

    if (!empty($this->message)) {
      msg_dialog::displayChecks($this->message);
      $success = FALSE;
    }

    // Initialize smarty and pass the status of potential error ??
    $this->initSmarty($success);

  }

  private function initSmarty ($success)
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
    $smarty->assign('success', $success);
    $smarty->assign('activated', $this->activated);
    $smarty->assign('taskName', $this->task['name']);
    $smarty->assign('token', $this->token);
    $smarty->display(get_template_path('taskReminder.tpl'));
  }

  /**
   * @param $token
   * @return bool
   * NOTE : To be revamp as this old logic seems a bit off to me.
   */
  function checkToken ($token)
  {
    global $config;

    /* Retrieve hash from the ldap */
    $ldap = $config->get_ldap_link();

    $dn = 'ou=' . $this->uid . ',ou=tokens' . $config->current['BASE'];
    $ldap->cat($dn);
    if ($attrs = $ldap->fetch()) {
      $ldapToken      = $attrs['tokens'][0];
      $tokenTimestamp = $attrs['timestamp'][0];
      $tokenDate      = $attrs['type'][0];

      // Check timestamp in UTC format
      if ($ldapToken == $token) {
        if (time() < $tokenDate) {
          return TRUE;
        }
      } else {
        return FALSE;
      }
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

    function postponeExpiration ()
    {
      global $config;
      $dn = $this->getUserDn();
      if (empty($dn)) {
        return;
      }
      $userTabs = objects::open($dn, 'user');
      if (
        isset($userTabs->by_object['posixAccount']) &&
        $userTabs->by_object['posixAccount']->is_account &&
        ($userTabs->by_object['posixAccount']->attributesAccess['shadowExpire']->getValue() != '')
      ) {
        $date = $userTabs->by_object['posixAccount']->attributesAccess['shadowExpire']->getDateValue();
        $date->add(new DateInterval('P' . $this->postpone_days . 'D'));
        $userTabs->by_object['posixAccount']->shadowExpire = $date;
      }
      if (
        isset($userTabs->by_object['supannAccountStatus']) &&
        $userTabs->by_object['supannAccountStatus']->is_account
      ) {
        $supannRessourceEtatDateValues = [];
        foreach ($userTabs->by_object['supannAccountStatus']->supannRessourceEtatDate as $line) {
          list ($label, $state, $substate, $start, $end) = $userTabs->by_object['supannAccountStatus']->attributesAccess['supannRessourceEtatDate']->attribute->readValues($line);
          if (($label == 'COMPTE') && ($state == 'A') && ($end != '')) {
            /* Postpone enddate */
            $date = new DateTime($end);
            $date->add(new DateInterval('P' . $this->postpone_days . 'D'));
            if ($substate != 'SupannSursis') {
              /* Set start date to current date if substate changed */
              $start = date('Ymd');
            }
            $supannRessourceEtatDateValues[] = '{COMPTE}A:SupannSursis:' . $start . ':' . $date->format('Ymd');
          } else {
            $supannRessourceEtatDateValues[] = $line;
          }
        }
        $userTabs->by_object['supannAccountStatus']->supannRessourceEtatDate = $supannRessourceEtatDateValues;
      }
      $error = $userTabs->save();
      if (!empty($error)) {
        $this->message = $error;
        return;
      }
      $this->deleteToken();

      $cn            = $userTabs->getBaseObject()->cn;
      $manager_dn    = $userTabs->getBaseObject()->manager;
      $email_address = '';
      if (isset($userTabs->by_object['mailAccount'])) {
        $email_address = $userTabs->by_object['mailAccount']->mail;
      } elseif ($this->use_alternate && isset($userTabs->by_object['mailAccount']) && !empty($userTabs->by_object['mailAccount']->gosaMailAlternateAddress)) {
        $email_address = reset($userTabs->by_object['mailAccount']->gosaMailAlternateAddress);
      } elseif (isset($userTabs->by_object['supannAccount']) && !empty($userTabs->by_object['supannAccount']->supannAutreMail)) {
        $email_address = reset($userTabs->by_object['supannAccount']->supannAutreMail);
      } elseif (isset($userTabs->by_object['personalInfo']) && !empty($userTabs->by_object['personalInfo']->fdPrivateMail)) {
        $email_address = $userTabs->by_object['personalInfo']->fdPrivateMail;
      }

      $manager_mail = '';
      if ($this->forward_postpone) {
        $ldap = $config->get_ldap_link();
        if (empty($manager_dn)) {
          $ldap->cat($userTabs->getBaseObject()->base, ['manager']);
          if (($attrs = $ldap->fetch()) && isset($attrs['manager'][0])) {
            $manager_dn = $attrs['manager'][0];
          }
        }
        if (!empty($manager_dn)) {
          $ldap->cat($manager_dn, ['cn', 'mail', 'gosaMailAlternateAddress', 'supannAutreMail', 'fdPrivateMail']);
          if ($attrs = $ldap->fetch()) {
            if (!empty($attrs['mail'][0])) {
              $manager_mail = $attrs['mail'][0];
            } elseif ($this->use_alternate && !empty($attrs['gosaMailAlternateAddress'][0])) {
              $manager_mail = $attrs['gosaMailAlternateAddress'][0];
            } elseif (!empty($attrs['supannAutreMail'][0])) {
              $manager_mail = $attrs['supannAutreMail'][0];
            } elseif (!empty($attrs['fdPrivateMail'][0])) {
              $manager_mail = $attrs['fdPrivateMail'][0];
            }
          }
        }
      }
    }

    /**
     * @return void
     * Note : Simply set global variables after sanitization.
     */
    private
    function sanitizeInputs ()
    {
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
        if (!is_alphanumeric($token) && !is_alphanumeric($task['name'])) {
          die("Invalid values within passed arguments...");
        }

        // Assign the sanitized values
        $this->token = $token;
        // Simply set the $task representing the name of main task to an array (Simplify further process down the line).
        $this->task['name'] = $task;
      }
    }
  }
