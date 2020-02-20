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
    unset($attrs['fdWebauthnRecoveryCode']['count']);
    if (empty($attrs['fdWebauthnRecoveryCode'])) {
      return FALSE;
    }

    session::set('fdWebauthnRecoveryCode', $attrs['fdWebauthnRecoveryCode']);

    return TRUE;
  }

  /*! \brief Process POST */
  static function earlyProcess ()
  {
    global $message, $config, $ui;

    if (!empty($_POST['recoverycode'])) {
      $fdWebauthnRecoveryCodes = session::get('fdWebauthnRecoveryCode');

      foreach ($fdWebauthnRecoveryCodes as $key => $fdWebauthnRecoveryCode) {
        list(, $hash) = explode('|', $fdWebauthnRecoveryCode, 2);
        if (password_verify($_POST['recoverycode'], $hash)) {
          /* Delete this recovery code so that it can only be used once */
          unset($fdWebauthnRecoveryCodes[$key]);
          $ldap = $config->get_ldap_link();
          $ldap->cd($ui->dn);
          $ldap->modify(['fdWebauthnRecoveryCode' => array_values($fdWebauthnRecoveryCodes)]);

          /* Warn the user that he needs to generate a new one */
          if ($ldap->success()) {
            msg_dialog::display(
              _('Recovery code'),
              _('You connected using a recovery code as second factor. This code has been deleted and will not be usable again in the future. Generate a new one if you need.'),
              INFO_DIALOG
            );
          } else {
            msg_dialog::display(
              _('Recovery code'),
              sprintf(
                _('You connected using a recovery code as second factor, but we failed to delete it for future use: %s'),
                msgPool::ldaperror($ldap->get_error(), $ui->dn, LDAP_MOD)
              ),
              ERROR_DIALOG
            );
          }

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

    return '<label for="recoverycode">'._('Or enter recovery Code here: ').'<input type="text" id="recoverycode" name="recoverycode"/></label>';
  }

  static function redirect ()
  {
    session::un_set('fdWebauthnRecoveryCode');
    LoginMethod::redirect();
  }
}
