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

class SecondFactorRecoveryCode
{
  static function init ()
  {
    session::un_set('fdWebauthnRecoveryCode');
  }

  /*! \brief Checks if connected user has second factor active */
  static function hasSecondFactor (): bool
  {
    global $ui, $config;

    $ldap = $config->get_ldap_link();

    $ldap->cat($ui->dn, ['fdWebauthnRecoveryCode']);
    $attrs = $ldap->fetch();
    if (!$attrs) {
      throw new FusionDirectoryException(_('Could not fetch user'));
    }
    unset($attrs['fdWebauthnRecoveryCode']['fdWebauthnRecoveryCode']);
    if (empty($attrs['fdWebauthnRecoveryCode'])) {
      return FALSE;
    }

    session::set('fdWebauthnRecoveryCode', $attrs['fdWebauthnRecoveryCode']);

    return TRUE;
  }

  /*! \brief Process POST */
  static function earlyProcess ()
  {
    global $message;
    if (isset($_POST['recoverycode'])) {
      $fdWebauthnRecoveryCodes = session::get('fdWebauthnRecoveryCode');

      foreach ($fdWebauthnRecoveryCodes as $fdWebauthnRecoveryCode) {
        list(, $hash) = explode('|', $fdWebauthnRecoveryCode, 2);
        if (password_verify($_POST['recoverycode'], $hash)) {
          /* TODO:
           * delete this recovery code so that it can only be used once
           * Warn the user that he needs to generate a new one */
          static::redirect();
        }
      }

      /* Slow down brute force by making failing slow: wait 5 seconds for each fail attempt */
      sleep(5);

      /* Inform the user that validation failed */
      $message = _('Validation of recovery code failed');
    }
  }

  static public function execute ()
  {
    global $smarty;

    if (!session::is_set('fdWebauthnRecoveryCode')) {
      return NULL;
    }

    return '<label for="otpcode">'._('Or enter recovery Code here: ').'<input type="text" id="recoverycode" name="recoverycode"/></label>';
  }

  static function redirect ()
  {
    session::un_set('fdWebauthnRecoveryCode');
    LoginMethod::redirect();
  }
}
