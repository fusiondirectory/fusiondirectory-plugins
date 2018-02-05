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

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
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
          new BooleanAttribute (
            _('Allow use of alternate addresses'), _('Whether the field gosaMailAlternateAddress should be used as well to send reminders'),
            'fdUserReminderUseAlternate', TRUE,
            TRUE
          ),
        )
      ),
      'emailppolicy' => array(
        'name'  => _('Ppolicy email settings'),
        'attrs' => array(
          new BooleanAttribute (
            _('Forward alerts to the manager'), _('Forward ppolicy alerts to the manager'),
            'fdUserReminderForwardPpolicyAlert', FALSE,
            TRUE
          ),
          new StringAttribute (
            _('Subject'), _('Subject of the ppolicy alert email'),
            'fdUserReminderPpolicyAlertSubject', FALSE,
            _('[FusionDirectory] Your password is about to expire')
          ),
          new TextAreaAttribute (
            _('Body (%s are cn and login)'),
            _('Body of the ppolicy alert email, sent when the user password is about to expire. Use %s for the cn and uid.'),
            'fdUserReminderPpolicyAlertBody', FALSE,
            _('Dear %1$s,'."\n".
              'your password for account %2$s is about to expire, please change your password: '."\n".
              'https://'.$_SERVER['SERVER_NAME'].
              (($_SERVER['SERVER_PORT'] != '80') ? ':'.$_SERVER['SERVER_PORT'] : '').
              (empty($_SERVER['REQUEST_URI']) ? $_SERVER['PATH_INFO'] : $_SERVER['REQUEST_URI']).
              "\n")
          )
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
            'fdUserReminderAlertSubject', FALSE,
            _('[FusionDirectory] Your account is about to expire')
          ),
          new TextAreaAttribute (
            _('Body (%s are cn, login, and link token)'),
            _('Body of the alert email, sent when the user is about to expire. Use %s for the cn, uid and token.'),
            'fdUserReminderAlertBody', FALSE,
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
          ),
          new HiddenAttribute (
            'fdReminderTokenRDN',
            FALSE,
            'ou=reminder'
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    if (!class_available('ppolicy')) {
      $this->attributesAccess['fdUserReminderForwardPpolicyAlert']->setDisabled(TRUE);
      $this->attributesAccess['fdUserReminderPpolicyAlertSubject']->setDisabled(TRUE);
      $this->attributesAccess['fdUserReminderPpolicyAlertBody']->setDisabled(TRUE);
    }
  }

  protected function post_save()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->create_missing_trees(get_ou('reminderTokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE']);
    parent::post_save();
  }
}
?>
