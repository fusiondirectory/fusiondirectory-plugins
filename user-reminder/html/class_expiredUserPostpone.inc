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
  function readLdapConfig()
  {
    global $config;
    $this->forward_postpone = ($config->get_cfg_value('userReminderForwardConfirmation', 'TRUE') == 'TRUE');
    $this->alert_delay      = $config->get_cfg_value('userReminderAlertDelay', 15);
    $this->postpone_days    = $config->get_cfg_value('userReminderPostponeDays', 15);
    $this->mail_subject     = $config->get_cfg_value('userReminderConfirmationSubject', '');
    $this->mail_body        = $config->get_cfg_value('userReminderConfirmationBody', '');
    $this->from_mail        = $config->get_cfg_value('userReminderEmail');

    return TRUE;
  }

  function execute()
  {
    global $error_collector, $error_collector_mailto;
    if (!$this->activated) {
      return;
    }

    if (($_SERVER['REQUEST_METHOD'] == 'GET') && isset($_GET['token'])) {
      # Check if the token match
      $this->uid = $_GET['uid'];
      if ($this->checkToken($_GET['token'])) {
        $this->message = array();
        $this->postponeExpiration();
        if (!empty($this->message)) {
          msg_dialog::displayChecks($this->message);
        }
      } else {
        $this->message[] = _('This token is invalid');
      }
    }

    $success = TRUE;
    /* Do we need to show error messages? */
    if (count($this->message) != 0) {
      /* Show error message and continue editing */
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
    $smarty->display(get_template_path('headers.tpl'));

    $smarty->assign('version',        FD_VERSION);
    $smarty->assign('success',        $success);
    $smarty->assign('activated',      $this->activated);

    $smarty->display(get_template_path('user-reminder.tpl'));
  }

  function checkToken($token)
  {
    global $config;
    $sha_token = '{SHA}'.base64_encode(hash('sha256', 'expired'.$token, TRUE));

    /* Retrieve hash from the ldap */
    $ldap = $config->get_ldap_link();

    $dn   = 'ou='.$this->uid.','.get_ou('tokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $ldap->cat($dn);
    $attrs = $ldap->fetch();

    $ldap_token = $attrs['userPassword'][0];
    $token_date = $attrs['description'][0];

    /* Return TRUE if the token match and is still valid */
    return ($token_date + $this->alert_delay >= (time() / 86400)) &&
           ($ldap_token == $sha_token);
  }

  function postponeExpiration()
  {
    global $config;

    $dn = $this->getUserDn();
    if (empty($dn)) {
      return;
    }
    $userTabs = objects::open($dn, 'user');
    $date = $userTabs->by_object['posixAccount']->attributesAccess['shadowExpire']->getDateValue();
    $date->add(new DateInterval('P'.$this->postpone_days.'D'));
    $userTabs->by_object['posixAccount']->shadowExpire = $date;
    $error = $userTabs->check();
    if (!empty($error)) {
      $this->message = $error;
      return;
    }
    $userTabs->save();
    $cn             = $userTabs->getBaseObject()->cn;
    $manager_dn     = $userTabs->getBaseObject()->manager;
    $email_address  = $userTabs->by_object['mailAccount']->mail;

    $manager_mail = '';
    if ($this->forward_postpone) {
      $ldap = $config->get_ldap_link();
      if (empty($manager_dn)) {
        $ldap->cat($userTabs->getBaseObject()->base, array('manager'));
        if ($attrs = $ldap->fetch() && isset($attrs['manager'][0])) {
          $manager_dn = $attrs['manager'][0];
        }
      }
      if (!empty($manager_dn)) {
        $ldap->cat($manager_dn, array('cn', 'mail'));
        if ($attrs = $ldap->fetch() && isset($attrs['mail'][0])) {
          $manager_mail = $attrs['mail'][0];
        }
      }
    }
    $this->sendConfirmationMail($cn, $email_address, $manager_mail);
  }

  function sendConfirmationMail($cn, $email_address, $manager_mail)
  {
    global $config;

    if (empty($this->mail_body) || empty($this->mail_subject)) {
      return;
    }
    $body = sprintf($this->mail_body, $cn, $this->uid);

    /* From */
    $headers = "From: ".$this->from_mail."\r\n";
    $headers .= "Reply-To: ".$this->from_mail."\r\n";

    $additional_parameters = "-f".$this->from_mail;

    if (!empty($manager_mail)) {
      $email_address .= ','.$manager_mail;
    }

    if (!mail($email_address, $this->mail_subject, $body, $headers, $additional_parameters)) {
      $this->message[] = _('Contact your administrator, there was a problem with mail server');
    }
  }

  function getUserDn()
  {
    global $config;

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
