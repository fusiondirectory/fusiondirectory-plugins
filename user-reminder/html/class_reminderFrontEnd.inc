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
 * In order to understand the logic of standAlone class.
 * - It will first check the init method which triggers the readInLdap.
 * - Once this is done, even a post torwards the same page only goes to execute.
 * - The flow is therefore :
 * 1. Arrival on run then init, readLdapConfig, then execute, smarty init.
 * 2. Upon refresh or post , it goes to execute then smarty init.
 */

require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');

class reminderFrontEnd extends standAlonePage
{
  private $token; //string
  private $task; //array
  private $uid; //string
  private $userDN; //string
  private $message; //array

  /**
   * @return bool
   * Note : method from extension.
   * It will help to get the information from the main task.
   * It is also run before class execute during first arrival on the page. IF a post is issued, it will directly go to
   * execute.
   */
  protected function readLdapConfig (): bool
  {
    return $this->sanitizeInputs();
  }

  // attributesName to be taken
  /* - EPPN eduPersonPrincipalName
  /  - UID
  /  - mail
  /  - alias supannAliasLogin
  */


  /**
   * @return void
   * @throws Exception
   * Note : Base function ran after init (on first arrival it goes by readLapConfig), on a POST refresh, it goes here directly.
   */
  function execute ()
  {
    // Simply store all the boolean returned by below prolongation methods, initialized to FALSE.
    $results = [FALSE];

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
        // Go ahead with account prolongation as token is valid and uid passed successfully. (UID or other identifiers).
      } else {
        // Reset the results variables array to be empty, only allowing sub methods to set boolean values within.
        $results = [];

        if (!empty($this->task['resource'])) {
          // Store the boolean result for later processing during init smarty.
          $results[] = $this->prolongedSupannAccount();
        }

        if (!empty($this->task['posix']) && $this->task['posix'] === "TRUE") {
          $results[] = $this->prolongedPosixAccount();
        }

        // If at this stage $result is empty, we set it to false. Meaning no supann, nor posix could be changed
        if (empty($results)) {
          $results = ['false'];
        }

        // Delete the token from LDAP
        $this->deleteToken();
      }
    }

    // Output error message before any smarty initialization, in order to pass proper variables to it.
    $error = $this->checkErrorMessages();
    // Make sure that success is either true or false
    $success = !in_array(FALSE, $results, TRUE);

    // Initialize smarty and pass success and error variables.
    $this->initSmarty($success, $error);
  }

  /**
   * @return bool
   * Note : Simply return TRUE if errors and display error messages. Messages can be an array of error messages.
   */
  private function checkErrorMessages (): bool
  {
    $errors = FALSE;
    if (!empty($this->message)) {
      msg_dialog::displayChecks($this->message);
      $errors = TRUE;
    }

    return $errors;
  }

  /**
   * @return bool
   * Note, prolongation of the account based on main tasks criteria.
   */
  private function prolongedSupannAccount (): bool
  {
    // Generate the supann string from main task.
    $newIncompleteSupann = $this->createSupannString(TRUE);
    // retrieve all supann resource for specific user
    $userSupann = $this->retrieveUserSupann();
    // Find a match between task supann string and user supann strings
    $userSupannRessourceEtat = $this->returnMatchingSupannResource($newIncompleteSupann, $userSupann);
    // retrieve the end date for user resource matching the task supann, the new incompleteSupann.
    $userSupannEndDate = $this->returnSupannEndDate($userSupannRessourceEtat);
    // Create the final supann resource state with a new start and new end date.
    $newSupannRessourceEtatDate = $this->finalSupannGeneration($newIncompleteSupann, $userSupannEndDate);

    // Time for the update of the user resource
    return $this->updateUserSupannResource($userSupannRessourceEtat, $newSupannRessourceEtatDate);
  }

  /**
   * @return bool
   * Note, prolongation of the account posix attributes based on main tasks criteria.
   * @throws Exception
   */
  private function prolongedPosixAccount (): bool
  {
    $result = FALSE;
    // Open user tabs
    $userTabs = objects::open($this->userDN, 'user');
    // Fetch posix information about shadow expire
    if (isset($userTabs->by_object['posixAccount']) && ($userTabs->by_object['posixAccount']->attributesAccess['shadowExpire']->getValue() != '')) {
      // Retrieve the date format of shadow expire
      $date = $userTabs->by_object['posixAccount']->attributesAccess['shadowExpire']->getDateValue();

      // Set the new extension for shadow expire based on the tasks extensions days configured
      $date->add(new DateInterval('P' . $this->task['days'] . 'D'));
      // Save the information within the attribute
      $userTabs->by_object['posixAccount']->shadowExpire = $date;
    }

    // Save the user
    $error = $userTabs->save();

    // Verification of errors.
    if (!empty($error)) {
      $this->message[] = to_string($error);
    } else {
      $result = TRUE;
    }

    return $result;
  }

  /**
   * @param string $userSupannRessourceEtat
   * @param string $newSupannRessourceEtatDate
   * @return bool
   */
  private function updateUserSupannResource (string $userSupannRessourceEtat, string $newSupannRessourceEtatDate): bool
  {
    $result = FALSE;

    // Open the user tabs
    $userTabs = objects::open($this->userDN, 'user');
    // Verify if supann Account Status tab is available
    if (isset($userTabs->by_object['supannAccountStatus'])) {
      $supannRessourceEtatDateValues = [];
      // Iteration on all potential supann values existing for that user dn.
      foreach ($userTabs->by_object['supannAccountStatus']->supannRessourceEtatDate as $line) {
        list ($resource, $state, $subState, $start, $end) = $userTabs->by_object['supannAccountStatus']->attributesAccess['supannRessourceEtatDate']->attribute->readValues($line);

        // Simply create a string of the supann array received. Helping for the verification below.
        $supannString = $this->createSupannString(FALSE, $resource, $state, $subState, $start, $end);

        // If there is a match with the user supann retrieve and previous methods, we change that value to the new one.
        if ($supannString === $userSupannRessourceEtat) {
          $supannRessourceEtatDateValues[] = $newSupannRessourceEtatDate;
        } else {
          // If not we have to keep previous supann value intact. (Mail or other resources, we mostly use account).
          $supannRessourceEtatDateValues[] = $line;
        }
      }
      $userTabs->by_object['supannAccountStatus']->supannRessourceEtatDate = $supannRessourceEtatDateValues;
    }
    $error = $userTabs->save();

    if (!empty($error)) {
      $this->message[] = to_string($error);
    } else {
      $result = TRUE;
    }

    return $result;
  }

  /**
   * @return string
   * Note : Simply create a string recognized by the ldap attribute supannRessourceEtat
   */
  private function createSupannString (bool $task, ?string $resource = '', ?string $state = '', ?string $subState = '',
    ?string $start = '', ?string $end = ''): string
  {
    $supann = NULL;

    if ($task && !empty($this->task['resource'])) {
      // case of subState present
      if (!empty($this->task['subState'])) {
        $supann = '{' . $this->task['resource'] . '}' . $this->task['state'] . ':' . $this->task['subState'];
      } else {
        $supann = '{' . $this->task['resource'] . '}' . $this->task['state'];
      }
      // This is going to be used for in the last supann verification when users tabs are open before updating states.
    } elseif (!empty($resource)) {
      $supann = '{' . $resource . '}' . $state . ':' . $subState . ':' . $start . ':' . $end;
    }

    return $supann;
  }

  /**
   * Note : receive the supannRessourceEtat as string to compare, return the final supannRessourceEtatDate as well as the
   * previous stat requiring deletion.
   * We delete the old record and set the new one.
   */
  private function retrieveUserSupann (): array
  {
    global $config;
    $allSupannState = [];

    /* Retrieve hash from the ldap */
    $ldap = $config->get_ldap_link();
    $ldap->cat($this->userDN);

    if ($attrs = $ldap->fetch()) {
      // Get the allSupannState from LDAP attributes
      $allSupannState = $attrs['supannRessourceEtatDate'];
    }
    // Remove the key [count]
    unset($allSupannState['count']);

    // It is possible that only one supann resource is available. To be coherent we will always return an array.
    if (is_array($allSupannState)) {
      return $allSupannState;
    } else {
      return [$allSupannState];
    }
  }

  /**
   * @param string $supannStringFromTask
   * @param array $userSupann
   * @return mixed|null
   * Note : Create a potential match between the task supann criteria and the user supann
   */
  private function returnMatchingSupannResource (string $supannStringFromTask, array $userSupann)
  {
    $previousResource = NULL;

    // Extracts the resource or whatever is inside {}
    if (preg_match('/\{([^\}]+)\}/', $supannStringFromTask, $match)) {
      $bracketContent = $match[1];

      foreach ($userSupann as $resource) {
        // Check if the resource matches the extracted content
        if (preg_match('/\{' . preg_quote($bracketContent, '/') . '\}/', $resource)) {
          $previousResource = $resource; // To be deleted in ldap. (New resource will take its place).
          break; // Exit the loop once a match is found, only one resource can be set in FusionDirectory, allowing the break;
        }
      }
    }
    return $previousResource;
  }


  /**
   * @param string $supannStringFromTask
   * @param string $endDate
   * @return string
   */
  private function finalSupannGeneration (string $supannStringFromTask, string $endDate): string
  {
    $finalSupann = '';
    // Case when last date has been recuperated.
    if (!empty($endDate)) {
      try {
        $newEndDate = $this->addDaysToDateString($endDate, $this->task['days']);
        // Simply set the last date as beginning date and real last date as the number of day to add.
        $finalSupann = $supannStringFromTask . ':' . $endDate . ':' . $newEndDate;
      } catch (Exception $e) {
        $this->message[] = $e->getMessage(); // Handle any exceptions that occur
      }

    }

    return $finalSupann; // Return the found end date or null if no match
  }

  /**
   * @param string $resource
   * @return string
   * Note : Simply return the end date of a supann resource.
   */
  private function returnSupannEndDate (string $resource): string
  {
    // Check if resource is empty
    if (empty($resource)) {
      return ''; // or handle it as appropriate
    }

    // Split the string into parts
    $parts = explode(':', $resource);

    // Get the last element and return it.
    return end($parts);
  }


  /**
   * @param string $dateString
   * @param int $daysToAdd
   * @return string
   * @throws Exception
   * Simply add the added numbers of days to the end supann resource state.
   */
  private function addDaysToDateString (string $dateString, int $daysToAdd): string
  {
    // Convert the string to a DateTime object
    $date = DateTime::createFromFormat('Ymd', $dateString);

    if ($date === FALSE) {
      // Handle the case where the date string is invalid
      throw new Exception("Invalid date string format.");
    }

    // Add the number of days
    $date->modify("+$daysToAdd days");

    // Return the new date as a string in the format YYYYMMDD
    return $date->format('Ymd');
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
   * @param string $token
   * @return bool
   * NOTE : To be revamp as this old logic seems a bit off to me.
   */
  private function checkToken (string $token): bool
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

      // This will serve to update supann Status
      $this->userDN = $attrs['fdTokenUserDN'][0];

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

    $dn = 'cn=' . $this->uid . ',ou=tokens,' . $config->current['BASE'];
    $ldap->rmdir($dn);
    if (!$ldap->success()) {
      $this->message[] = $ldap->get_error();
    }
  }

  /**
   * @param string $value
   * @return false|int
   * Note : Simple method that verify if only char and num are present.
   */
  private function isAlphanumeric (string $value)
  {
    return preg_match('/^[a-zA-Z0-9]+$/', $value);
  }

  /**
   * @return bool
   * Note : Simply set global variables after sanitization.
   */
  private function sanitizeInputs (): bool
  {
    $result = FALSE;

    if ($_SERVER['REQUEST_METHOD'] == 'GET' && isset($_GET['token']) && isset($_GET['task'])) {
      // Sanitize and validate input fields
      $token = filter_input(INPUT_GET, 'token', FILTER_SANITIZE_STRING);
      $task  = filter_input(INPUT_GET, 'task', FILTER_SANITIZE_STRING);

      // Validate 'token'
      if (!$this->isAlphanumeric($token) || !$this->isAlphanumeric($task)) {
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

      // Validate 'token'
      if (!$this->isAlphanumeric($uid)) {
        die("Invalid values within passed arguments...");
      }

      $this->uid = $uid;
      $result    = TRUE;
    }

    return $result;
  }

  /**
   * @return bool
   * Note : Simply return the attributes set within the main task reminder.
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


