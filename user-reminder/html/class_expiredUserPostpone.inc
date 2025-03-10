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

class expiredUserPostpone extends standAlonePage
{
  protected $forward_postpone;
  protected $alert_delay;
  protected $postpone_days;
  protected $mail_subject;
  protected $mail_body;
  protected $from_mail;
  protected $use_alternate;

  protected $message;
  protected $uid;

  protected function readLdapConfig () : bool
  {
    global $config;
    $this->forward_postpone = ($config->get_cfg_value('userReminderForwardConfirmation', 'TRUE') == 'TRUE');
    $this->alert_delay      = $config->get_cfg_value('userReminderAlertDelay', 15);
    $this->postpone_days    = $config->get_cfg_value('userReminderPostponeDays', 15);
    $this->mail_subject     = $config->get_cfg_value('userReminderConfirmationSubject', '');
    $this->mail_body        = $config->get_cfg_value('userReminderConfirmationBody', '');
    $this->from_mail        = $config->get_cfg_value('userReminderEmail');
    $this->use_alternate    = ($config->get_cfg_value('userReminderUseAlternate', 'TRUE') == 'TRUE');

    return TRUE;
  }

  function execute ()
  {
    global $error_collector, $error_collector_mailto;

    if (!$this->activated) {
      return;
    }

    $success        = TRUE;
    $this->message  = [];

    if (($_SERVER['REQUEST_METHOD'] == 'GET') && isset($_GET['token'])) {
      // Check if the token match
      $this->uid = $_GET['uid'];
      if ($this->checkToken($_GET['token'])) {
        $this->postponeExpiration();
      } else {
        $this->message[] = _('This token is invalid');
      }
    } else {
      $this->message[] = _('Token is missing from URL');
    }

    /* Do we need to show error messages? */
    if (!empty($this->message)) {
      msg_dialog::displayChecks($this->message);
      $success = FALSE;
    }

    $smarty = get_smarty();

    $smarty->assign('PHPSESSID', session_id());
    if (session::is_set('errors')) {
      $smarty->assign('errors', session::get('errors'));
    }
    if ($error_collector != '') {
      $smarty->assign('php_errors', preg_replace('/%BUGBODY%/', $error_collector_mailto, $error_collector).'</div>');
    } else {
      $smarty->assign('php_errors', '');
    }

    $smarty->assign('msg_dialogs',  msg_dialog::get_dialogs());
    $smarty->assign('usePrototype', 'FALSE');
    $smarty->append('js_files',     'include/pwdStrength.js');
    $smarty->append('css_files',    get_template_path('login.css'));
    $lang = session::global_get('lang');
    $smarty->assign('lang', preg_replace('/_.*$/', '', $lang));
    $smarty->assign('rtl', Language::isRTL($lang));
    $smarty->display(get_template_path('headers.tpl'));

    $smarty->assign('version',        FD_VERSION);
    $smarty->assign('success',        $success);
    $smarty->assign('activated',      $this->activated);
    $smarty->display(get_template_path('user-reminder.tpl'));
  }

  function checkToken ($token)
  {
    global $config;
    $sha_token = '{SHA}'.base64_encode(hash('sha256', 'expired'.$token, TRUE));

    /* Retrieve hash from the ldap */
    $ldap = $config->get_ldap_link();

    $dn   = 'ou='.$this->uid.','.get_ou('reminderTokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $ldap->cat($dn);
    if ($attrs = $ldap->fetch()) {
      $ldap_token = $attrs['userPassword'][0];
      $token_date = $attrs['description'][0];

      /* Return TRUE if the token match and is still valid */
      return ($token_date + $this->alert_delay >= (time() / 86400)) &&
             ($ldap_token == $sha_token);
    } else {
      return FALSE;
    }
  }

  function deleteToken ()
  {
    global $config;

    $ldap = $config->get_ldap_link();

    $dn   = 'ou='.$this->uid.','.get_ou('reminderTokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
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
      $date->add(new DateInterval('P'.$this->postpone_days.'D'));
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
          $date->add(new DateInterval('P'.$this->postpone_days.'D'));
          if ($substate != 'SupannSursis') {
            /* Set start date to current date if substate changed */
            $start = date('Ymd');
          }
          $supannRessourceEtatDateValues[] = '{COMPTE}A:SupannSursis:'.$start.':'.$date->format('Ymd');
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

    $cn             = $userTabs->getBaseObject()->cn;
    $manager_dn     = $userTabs->getBaseObject()->manager;
    $email_address  = '';
    if (isset($userTabs->by_object['mailAccount'])) {
      $email_address  = $userTabs->by_object['mailAccount']->mail;
    } elseif ($this->use_alternate && isset($userTabs->by_object['mailAccount']) && !empty($userTabs->by_object['mailAccount']->gosaMailAlternateAddress)) {
      $email_address  = reset($userTabs->by_object['mailAccount']->gosaMailAlternateAddress);
    } elseif (isset($userTabs->by_object['supannAccount']) && !empty($userTabs->by_object['supannAccount']->supannAutreMail)) {
      $email_address  = reset($userTabs->by_object['supannAccount']->supannAutreMail);
    } elseif (isset($userTabs->by_object['personalInfo']) && !empty($userTabs->by_object['personalInfo']->fdPrivateMail)) {
      $email_address  = $userTabs->by_object['personalInfo']->fdPrivateMail;
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
    $this->sendConfirmationMail($cn, $email_address, $manager_mail);
  }

  function sendConfirmationMail ($cn, $email_address, $manager_mail)
  {
    if (empty($this->mail_body) || empty($this->mail_subject)) {
      return;
    }
    $body = sprintf($this->mail_body, $cn, $this->uid);

    if (!empty($manager_mail)) {
      $email_address .= ','.$manager_mail;
    }

    if (!mail_utf8($email_address, FALSE, $this->from_mail, $this->mail_subject, $body)) {
      $this->message[] = _('Contact your administrator, there was a problem with mail server');
    }
  }

  function getUserDn ()
  {
    $users = objects::ls('user', NULL, NULL, '(uid='.$this->uid.')');

    if (count($users) < 1) {
      $this->message[] = sprintf(_('Did not find an account with login "%s"'), $this->uid);
      return;
    } elseif (count($users) > 1) {
      $this->message[] = sprintf(_('Found multiple accounts with login "%s"'), $this->uid);
      return;
    }

    return key($users);
  }
}
