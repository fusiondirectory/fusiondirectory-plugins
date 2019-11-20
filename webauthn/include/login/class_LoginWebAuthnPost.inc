<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2018-2019  FusionDirectory

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

require_once('WebAuthn/WebAuthn.php');

/*!
 * \brief Login via POST + 2nd factor
 */
class LoginWebAuthnPost extends LoginPost
{
  /*! \brief Displayed name */
  static function getLabel ()
  {
    return _('HTML form and 2nd factor');
  }

  static function processWebAuthnJavascriptRequests ()
  {
    if (isset($_GET['webauthn'])) {
      try {
        switch ($_GET['webauthn']) {
          case 'getGetArgs':
            print(json_encode(static::getGetArgs()));
            break;
          case 'processGet':
            print(json_encode(static::processGet()));
            break;
          default:
            throw new FusionDirectoryException('Unknown operation '.$_GET['webauthn']);
        }
      } catch (Throwable $ex) {
        session::un_set('challenge');
        $return           = new stdClass();
        $return->success  = FALSE;
        $return->msg      = "$ex";//->getMessage();
        print(json_encode($return));
      }
      exit();
    }
  }

  /*! \brief All login steps in the right order for standard POST login */
  static function loginProcess ()
  {
    global $smarty, $config, $message;

    static::processWebAuthnJavascriptRequests();

    static::init();

    $smarty->assign('focusfield', 'username');

    if (($_SERVER['REQUEST_METHOD'] == 'POST') && isset($_POST['login']) && isset($_POST['username']) && isset($_POST['password'])) {
      static::$username = $_POST['username'];
      static::$password = $_POST['password'];

      $success = static::runSteps([
        'validateUserInput',
        'checkForLockingBranch',
        'ldapLoginUser',
        'loginAndCheckExpired',
        'runSchemaCheck',
        'secondFactorAuth',
      ]);

      if ($success) {
        static::redirectSecondFactorPage();
      }
    }

    /* Translation of cookie-warning. Whether to display it, is determined by JavaScript */
    $smarty->assign('cookies', '<b>'._('Warning').':</b> '._('Your browser has cookies disabled. Please enable cookies and reload this page before logging in!'));

    static::displayLogin();
  }

  /*! \brief Called after successful login, return FALSE if account is expired */
  static function secondFactorAuth ()
  {
    global $ui, $config, $plist, $message, $smarty;

    $ldap = $config->get_ldap_link();

    $ldap->cat($ui->dn, ['fdWebauthnRegistrations']);
    $attrs = $ldap->fetch();
    if (!$attrs) {
      $message = _('Could not fetch user');
      return FALSE;
    }
    unset($attrs['fdWebauthnRegistrations']['count']);
    if (empty($attrs['fdWebauthnRegistrations'])) {
      $message = _('2nd factor information missing');
      return FALSE;
    }

    session::set('fdWebauthnRegistrations', $attrs['fdWebauthnRegistrations']);

    return TRUE;
  }

  /*! \brief Display the login page and exit() */
  static protected function redirectSecondFactorPage ()
  {
    session::un_set('connected');
    header('Location: secondfactor.php');
    exit;
  }

  /*! \brief Display the login page and exit() */
  static function displaySecondFactorPage ()
  {
    global $smarty,$message,$config,$ssl,$error_collector,$error_collector_mailto;

    $lang = session::get('lang');

    error_reporting(E_ALL | E_STRICT);

    /* Fill template with required values */
    $username = '';
    if (isset($_POST['username'])) {
      $username = trim($_POST['username']);
    }
    $smarty->assign('date',       gmdate('D, d M Y H:i:s'));
    $smarty->assign('username',   $username);
    $smarty->assign('revision',   FD_VERSION);
    $smarty->assign('year',       date('Y'));
    $smarty->append('css_files',  get_template_path('login.css'));
    $smarty->assign('title',      _('Second factor'));

    /* Some error to display? */
    if (!isset($message)) {
      $message = '';
    }
    $smarty->assign('message', $message);

    /* Display SSL mode warning? */
    if (($ssl != '') && ($config->get_cfg_value('warnSSL') == 'TRUE')) {
      $smarty->assign('ssl', sprintf(_('Warning: <a href="%s">Session is not encrypted!</a>'), $ssl));
    } else {
      $smarty->assign('ssl', '');
    }

    /* show login screen */
    $smarty->assign('PHPSESSID', session_id());
    if ($error_collector != '') {
      $smarty->assign('php_errors', preg_replace('/%BUGBODY%/', $error_collector_mailto, $error_collector).'</div>');
    } else {
      $smarty->assign('php_errors', '');
    }
    $smarty->assign('msg_dialogs',  msg_dialog::get_dialogs());
    $smarty->assign('usePrototype', 'false');
    $smarty->assign('date',         date('l, dS F Y H:i:s O'));
    $smarty->assign('lang',         preg_replace('/_.*$/', '', $lang));
    $smarty->assign('rtl',          Language::isRTL($lang));

    $smarty->append('js_files', 'include/webauthn.js');

    $smarty->display(get_template_path('headers.tpl'));
    $smarty->assign('version', FD_VERSION);

    $smarty->display(get_template_path('secondfactor.tpl'));
    exit();
  }

  static protected function initWebAuthnObject ()
  {
    // Formats
    $formats = [];
    //~ if ($_GET['fmt_android-key']) {
      $formats[] = 'android-key';
    //~ }
    //~ if ($_GET['fmt_android-safetynet']) {
      $formats[] = 'android-safetynet';
    //~ }
    //~ if ($_GET['fmt_fido-u2f']) {
      $formats[] = 'fido-u2f';
    //~ }
    //~ if ($_GET['fmt_none']) {
      $formats[] = 'none';
    //~ }
    //~ if ($_GET['fmt_packed']) {
      $formats[] = 'packed';
    //~ }
    //~ if ($_GET['fmt_tpm']) {
      //~ $formats[] = 'tpm';
    //~ }

    // new Instance of the server library.
    // make sure that $rpId is the domain name.
    if (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) {
      $host = $_SERVER['HTTP_X_FORWARDED_HOST'];
    } else {
      $host = $_SERVER['SERVER_NAME'];
    }
    $WebAuthn = new \WebAuthn\WebAuthn('FusionDirectory', $host, $formats);

    // add root certificates to validate new registrations
    /*if ($_GET['solo']) {
      $WebAuthn->addRootCertificates('rootCertificates/solo.pem');
    }
    if ($_GET['yubico']) {
      $WebAuthn->addRootCertificates('rootCertificates/yubico.pem');
    }
    if ($_GET['hypersecu']) {
      $WebAuthn->addRootCertificates('rootCertificates/hypersecu.pem');
    }
    if ($_GET['google']) {
      $WebAuthn->addRootCertificates('rootCertificates/globalSign.pem');
      $WebAuthn->addRootCertificates('rootCertificates/googleHardware.pem');
    }*/

    return $WebAuthn;
  }

  static protected function getGetArgs ()
  {
    global $ui;

    $WebAuthn = static::initWebAuthnObject();

    $ids = [];
    $fdWebauthnRegistrations = session::get('fdWebauthnRegistrations');
    foreach ($fdWebauthnRegistrations as $fdWebauthnRegistration) {
      $reg = webauthnAccount::unserializeRegistration($fdWebauthnRegistration);
      $ids[] = $reg['credentialId'];
    }

    $getArgs = $WebAuthn->getGetArgs($ids);

    session::set('challenge', serialize($WebAuthn->getChallenge()));

    return $getArgs;
  }

  static protected function processGet ()
  {
    global $config;

    $WebAuthn = static::initWebAuthnObject();

    $post = trim(file_get_contents('php://input'));
    if ($post) {
      $post = json_decode($post);
    }
    $clientDataJSON = base64_decode($post->clientDataJSON);
    $authenticatorData = base64_decode($post->authenticatorData);
    $signature = base64_decode($post->signature);
    $id = base64_decode($post->id);
    $challenge = unserialize(session::get('challenge'));
    $credentialPublicKey = NULL;

    $fdWebauthnRegistrations = session::get('fdWebauthnRegistrations');

    foreach ($fdWebauthnRegistrations as $fdWebauthnRegistration) {
      $reg = webauthnAccount::unserializeRegistration($fdWebauthnRegistration);
      if ($reg['credentialId'] === $id) {
        $credentialPublicKey = $reg['credentialPublicKey'];
        break;
      }
    }

    if ($credentialPublicKey === NULL) {
      throw new Exception('Public Key for credential ID not found!');
    }

    // process the get request. throws WebAuthnException if it fails
    $WebAuthn->processGet($clientDataJSON, $authenticatorData, $signature, $credentialPublicKey, $challenge);

    session::un_set('challenge');

    static::connect();

    $return = new stdClass();
    $return->success = TRUE;
    return $return;
  }

  /* Same as redirect without redirection */
  static function connect ()
  {
    global $config, $ui;

    $ui = session::get('ui');

    /* Not account expired or password forced change go to main page */
    logging::log('security', 'login', $ui->uid, [], 'Logged in successfully');
    session::set('connected', 1);
    session::set('DEBUGLEVEL', $config->get_cfg_value('DEBUGLEVEL'));
  }
}
