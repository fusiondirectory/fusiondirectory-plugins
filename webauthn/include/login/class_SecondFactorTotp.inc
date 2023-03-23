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

require_once('variables_webauthn.inc');

class SecondFactorTotp
{
  static function init ()
  {
    session::un_set('fdTOTPTokens');
  }

  /*! \brief Checks if connected user has second factor active */
  static function hasSecondFactor (): bool
  {
    global $ui, $config;

    $ldap = $config->get_ldap_link();

    $ldap->cat($ui->dn, ['fdTOTPTokens']);
    $attrs = $ldap->fetch();
    if (!$attrs) {
      throw new FusionDirectoryException(_('Could not fetch user'));
    }
    unset($attrs['fdTOTPTokens']['count']);
    if (empty($attrs['fdTOTPTokens'])) {
      return FALSE;
    }

    session::set('fdTOTPTokens', $attrs['fdTOTPTokens']);

    return TRUE;
  }

  /*! \brief Process POST */
  static function earlyProcess ()
  {
    global $message;
    if (!empty($_POST['otpcode'])) {
      $fdTOTPTokens = session::get('fdTOTPTokens');

      foreach ($fdTOTPTokens as $fdTOTPToken) {
        list(, $uri) = explode('|', $fdTOTPToken);
        $otp = OTPHP\Factory::loadFromProvisioningUri($uri);
        if ($otp->verify($_POST['otpcode'])) {
          static::redirect();
        }
      }

      /* Slow down brute force by making failing slow: wait 5 seconds for each fail attempt */
      sleep(5);

      /* Inform the user that validation failed */
      $message = _('Validation of TOTP code failed');
    }
  }

  static public function execute ()
  {
    global $smarty;

    if (!session::is_set('fdTOTPTokens')) {
      return NULL;
    }

    return '<label for="otpcode">'._('Enter TOTP Code here: ').'<input type="text" id="otpcode" name="otpcode"/></label>';
  }

  static function redirect ()
  {
    session::un_set('fdTOTPTokens');
    LoginMethod::redirect();
  }
}
