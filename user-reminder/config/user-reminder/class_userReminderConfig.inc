<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016 FusionDirectory

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

class userReminderConfig extends simplePlugin
{
  var $objectclasses  = array('fdUserReminderPluginConf');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('User reminder'),
      'plDescription'   => _('Configuration for the reminder of accounts expiration'),
      'plObjectType'    => array('configuration'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('User reminder settings'),
        'attrs' => array(
          new IntAttribute (
            _('Delay before expiration'), _('Days before expiration to send the first mail'),
            'fdUserReminderAlertDelay', TRUE,
            1, FALSE, 15
          ),
          new IntAttribute (
            _('Delay before sending again'), _('Days before sending a second mail (0 to disable)'),
            'fdUserReminderResendDelay', TRUE,
            0, FALSE, 7
          ),
          new IntAttribute (
            _('Extension of the validity'), _('Extension of the duration of the account in days'),
            'fdUserReminderPostponeDays', TRUE,
            1, FALSE, 15
          ),
          new MailAttribute (
            _('Sender email address'), _('Email address from which mails will be sent'),
            'fdUserReminderEmail', TRUE,
            'to.be@chang.ed'
          ),
        )
      ),
      'email1' => array(
        'name'  => _('Alert email settings'),
        'attrs' => array(
          new BooleanAttribute (
            _('Forward alerts to the manager'), _('Forward alerts to the manager'),
            'fdUserReminderForwardAlert', TRUE,
            TRUE
          ),
          new StringAttribute (
            _('Subject'), _('Subject of the alert email'),
            'fdUserReminderAlertSubject', TRUE,
            _('[FusionDirectory] Your account is about to expire')
          ),
          new TextAreaAttribute (
            _('Body (%s are cn, login, and link token)'),
            _('Body of the alert email, sent when the user is about to expire. Use %s for the cn, uid and token.'),
            'fdUserReminderAlertBody', TRUE,
            _('Dear %1$s,'."\n".
              'your account %2$s is about to expire, please use this link to avoid this: '."\n".
              'https://'.$_SERVER['SERVER_NAME'].
              (($_SERVER['SERVER_PORT'] != '80') ? ':'.$_SERVER['SERVER_PORT'] : '').
              (empty($_SERVER['REQUEST_URI']) ? $_SERVER['PATH_INFO'] : $_SERVER['REQUEST_URI']).
              '/expired_postpone.php?uid=%2$s&token=%3$s'."\n")
          )
        )
      ),
      'email2' => array(
        'name'  => _('Confirmation email settings'),
        'attrs' => array(
          new BooleanAttribute (
            _('Forward confirmation to the manager'), _('Forward account extension confirmation to the manager'),
            'fdUserReminderForwardConfirmation', TRUE,
            TRUE
          ),
          new StringAttribute (
            _('Subject'), _('Subject of the confirmation email'),
            'fdUserReminderConfirmationSubject', FALSE,
            _('[FusionDirectory] Your account expiration has been postponed')
          ),
          new TextAreaAttribute (
            _('Body (%s are cn and login)'),
            _('Body of the confirmation email, sent when the user has postponed expiration. Use %s for the cn and uid.'),
            'fdUserReminderConfirmationBody', FALSE,
            _('Dear %1$s,'."\n".' your account %2$s expiration has been successfully postponed.'."\n")
          )
        )
      )
    );
  }
}
?>
