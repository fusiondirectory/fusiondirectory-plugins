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

class webauthnAccount extends simplePlugin
{
  var $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('WebAuthn'),
      'plDescription'   => _('Manage double factor authentication'),
      'plSelfModify'    => TRUE,
      'plObjectClass'   => ['fdWebauthnAccount'],
      'plObjectType'    => ['user'],
      'plIcon'          => 'geticon.php?context=applications&icon=webauthn&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=webauthn&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Second factor'),
        'class' => ['fullwidth'],
        'attrs' => [
          new WebauthnRegistrationsAttribute(
            _('WebAuthn'), _('Registrations for this user'),
            'fdWebauthnRegistrations'
          ),
          new TOTPRegistrationsAttribute(
            _('TOTP'), _('TOTP codes for this user'),
            'fdTOTPTokens'
          ),
          new RecoveryCodesAttribute(
            _('Recovery codes'), _('Single use recovery codes in case you lose all your devices. Print it and store it. Do not save it on your computer.'),
            'fdWebauthnRecoveryCode'
          )
        ]
      ]
    ];
  }

  function execute (): string
  {
    global $smarty;

    if (isset($_GET['webauthn'])) {
      try {
        switch ($_GET['webauthn']) {
          case 'getCreateArgs':
            print(json_encode($this->getCreateArgs()));
            break;
          case 'processCreate':
            print(json_encode($this->processCreate()));
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

    $smarty->append('js_files', 'include/webauthn.js');

    return parent::execute();
  }

  static public function initWebAuthnObject ()
  {
    global $config;

    // Formats
    $formats = $config->get_cfg_value('WebauthnFormats', ['android-key','fido-u2f','packed']);

    // new Instance of the server library.
    // make sure that $rpId is the domain name.
    if (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) {
      $host = $_SERVER['HTTP_X_FORWARDED_HOST'];
    } else {
      $host = $_SERVER['SERVER_NAME'];
    }
    $WebAuthn = new \WebAuthn\WebAuthn('FusionDirectory', $host, $formats);

    return $WebAuthn;
  }

  protected function getCreateArgs ()
  {
    global $ui;

    $WebAuthn = static::initWebAuthnObject();

    $createArgs = $WebAuthn->getCreateArgs($ui->dn, $ui->uid, $ui->cn, 20, FALSE);

    session::set('WebauthnChallenge', $WebAuthn->getChallenge());

    return $createArgs;
  }

  protected function processCreate ()
  {
    $WebAuthn = static::initWebAuthnObject();

    $post = trim(file_get_contents('php://input'));
    if ($post) {
        $post = json_decode($post);
    }
    $clientDataJSON     = base64_decode($post->clientDataJSON);
    $attestationObject  = base64_decode($post->attestationObject);
    $challenge          = session::get('WebauthnChallenge');

    $data = $WebAuthn->processCreate($clientDataJSON, $attestationObject, $challenge);

    $this->attributesAccess['fdWebauthnRegistrations']->addRegistration(static::serializeRegistration($data));

    $return           = new stdClass();
    $return->success  = TRUE;
    $return->msg      = 'Registration Success! ' . count($this->fdWebauthnRegistrations) . ' registrations.';

    session::un_set('WebauthnChallenge');

    return $return;
  }

  static public function serializeRegistration (object $data): string
  {
    $safeData = [];
    foreach (['rpId','credentialPublicKey','certificateChain','certificate','certificateIssuer','certificateSubject','signatureCounter'] as $stringField) {
      $safeData[$stringField] = $data->$stringField;
    }
    foreach (['credentialId','AAGUID'] as $binField) {
      // base64 encode binary fields
      $safeData[$binField] = base64_encode($data->$binField);
    }

    return json_encode($safeData);
  }

  static public function unserializeRegistration (string $json): array
  {
    $data = json_decode($json, TRUE);
    foreach (['credentialId','AAGUID'] as $binField) {
      $data[$binField] = base64_decode($data[$binField]);
    }

    return $data;
  }
}
