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

class SecondFactorWebAuthn
{
  static function init ()
  {
    session::un_set('WebauthnChallenge');
    session::un_set('fdWebauthnRegistrations');
  }

  /*! \brief Checks if connected user has second factor active */
  static function hasSecondFactor (): bool
  {
    global $ui, $config;

    $ldap = $config->get_ldap_link();

    $ldap->cat($ui->dn, ['fdWebauthnRegistrations']);
    $attrs = $ldap->fetch();
    if (!$attrs) {
      throw new FusionDirectoryException(_('Could not fetch user'));
    }
    unset($attrs['fdWebauthnRegistrations']['count']);
    if (empty($attrs['fdWebauthnRegistrations'])) {
      return FALSE;
    }

    session::set('fdWebauthnRegistrations', $attrs['fdWebauthnRegistrations']);

    return TRUE;
  }

  /*! \brief Process WebAuthn Javascript Requests */
  static function earlyProcess ()
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
        session::un_set('WebauthnChallenge');
        $return           = new stdClass();
        $return->success  = FALSE;
        $return->msg      = $ex->getMessage();
        print(json_encode($return));
      }
      exit();
    }
  }

  static protected function getGetArgs ()
  {
    global $ui;

    $WebAuthn = webauthnAccount::initWebAuthnObject();

    $ids = [];
    $fdWebauthnRegistrations = session::get('fdWebauthnRegistrations');
    foreach ($fdWebauthnRegistrations as $fdWebauthnRegistration) {
      $reg = webauthnAccount::unserializeRegistration($fdWebauthnRegistration);
      $ids[] = $reg['credentialId'];
    }

    $getArgs = $WebAuthn->getGetArgs($ids);

    /* Serializing avoids failed autoload of WebAuthn classes before our require_once (at session start) */
    session::set('WebauthnChallenge', serialize($WebAuthn->getChallenge()));

    return $getArgs;
  }

  static protected function processGet ()
  {
    global $config;

    $WebAuthn = webauthnAccount::initWebAuthnObject();

    $post = trim(file_get_contents('php://input'));
    if ($post) {
      $post = json_decode($post);
    }
    $clientDataJSON       = base64_decode($post->clientDataJSON);
    $authenticatorData    = base64_decode($post->authenticatorData);
    $signature            = base64_decode($post->signature);
    $id                   = base64_decode($post->id);
    $challenge            = unserialize(session::get('WebauthnChallenge'));
    $credentialPublicKey  = NULL;

    $fdWebauthnRegistrations = session::get('fdWebauthnRegistrations');

    foreach ($fdWebauthnRegistrations as $fdWebauthnRegistration) {
      $reg = webauthnAccount::unserializeRegistration($fdWebauthnRegistration);
      if ($reg['credentialId'] === $id) {
        $credentialPublicKey = $reg['credentialPublicKey'];
        break;
      }
    }

    if ($credentialPublicKey === NULL) {
      throw new FusionDirectoryException('Public Key for credential ID not found!');
    }

    // process the get request. throws WebAuthnException if it fails
    $WebAuthn->processGet($clientDataJSON, $authenticatorData, $signature, $credentialPublicKey, $challenge);

    session::un_set('WebauthnChallenge');

    static::connect();

    $return = new stdClass();
    $return->success = TRUE;
    return $return;
  }

  static public function execute ()
  {
    global $smarty;

    if (!session::is_set('fdWebauthnRegistrations')) {
      return NULL;
    }

    $smarty->append('js_files', 'include/webauthn.js');

    $message = _('Trying to communicate with your device. Plug it in (if you haven\'t already) and press the button on the device now.');

    return htmlescape($message).
      '<script type="text/javascript">'."\n".
      '<!-- '."\n".
      'webauthnCheckRegistration();'."\n".
      '-->'."\n".
      '</script>'."\n".
      '<noscript>'.htmlescape(_('Javascript is needed for WebAuthn second factor, please enable it for this page.')).'</noscript>';
  }

  /* Same as redirect without redirection */
  static function connect ()
  {
    LoginMethod::connect();
    session::un_set('WebauthnChallenge');
    session::un_set('fdWebauthnRegistrations');
  }
}
