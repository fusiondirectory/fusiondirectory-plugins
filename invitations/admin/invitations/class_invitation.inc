<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018 FusionDirectory

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

class invitation extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Invitation'),
      'plDescription' => _('Invitation to fill a public form'),
      'plObjectClass' => ['fdInvitation'],
      'plObjectType'  => ['invitation' => [
        'name'    => _('Invitation'),
        'ou'      => get_ou('invitationRDN'),
        'icon'    => 'geticon.php?context=types&icon=invitation&size=16',
      ]],
      'plForeignKeys'   => [
        'fdInvitationFormDN' => 'publicForm',
      ],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $ui, $config;
    return [
      'main' => [
        'name'  => _('Invitation'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('invitationRDN')),
          new HostNameAttribute(
            _('Name'), _('Name used to identify this invitation'),
            'cn', TRUE
          ),
          new SelectAttribute(
            _('Form'), _('Public form this invitation should link to'),
            'fdInvitationFormDN', TRUE
          ),
          new StringAttribute(
            _('Email subject'), _('Subject of the email sent for this invitation'),
            'fdInvitationEmailSubject', TRUE
          ),
          new TextAreaAttribute(
            _('Email content'), _('Content of the email sent for this invitation'),
            'fdInvitationEmailContent', TRUE,
            $config->get_cfg_value('InvitationDefaultEmailContent')
          ),
          new DisplayLDAPAttribute(
            _('Author'), _('Author of this invitation'),
            'fdInvitationAuthor', FALSE,
            $ui->dn
          ),
        ]
      ],
      'emails' => [
        'name'  => _('Emails'),
        'attrs' => [
          new StringAttribute(
            _('Reply to'), _('Email address set as "Reply to" in the sent emails'),
            'reply_to', FALSE
          ),
          new FileTextAreaAttribute(
            _('Emails'), _('List of email adresses of people to invite - one per line'),
            'emails', FALSE,
            '.txt', TRUE, FALSE
          ),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    global $ui;
    if ($attributesInfo === NULL) {
      $attributesInfo = $this->getAttributesInfo();
    }
    $choices = objects::ls('publicForm');
    $attributesInfo['main']['attrs'][2]->setChoices(array_keys($choices), array_values($choices));

    $attributesInfo['emails']['attrs'][0]->setInLdap(FALSE);
    $attributesInfo['emails']['attrs'][1]->setInLdap(FALSE);

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    /* Fill reply_to with connected user email address */
    $users = objects::ls('user', 'mail', $ui->dn, '(mail=*)', FALSE, 'base');
    if (!empty($users)) {
      $this->reply_to = reset($users);
    }
  }

  function check (): array
  {
    $messages = parent::check();

    $emails = explode("\n", $this->emails);
    foreach ($emails as $email) {
      if (!tests::is_email($email)) {
        $messages[] = sprintf(_('"%s" is not a valid email address'), $email);
      }
    }

    return $messages;
  }

  protected function shouldSave (): bool
  {
    return (parent::shouldSave() || !empty($this->emails));
  }

  protected function ldap_save (): array
  {
    global $ui, $config;

    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    $emails   = explode("\n", $this->emails);
    $success  = 0;
    $failed   = [];
    foreach ($emails as $email) {
      if (empty($email)) {
        continue;
      }
      $registrations = objects::count('registration', NULL, '(fdRegistrationEmailAddress='.ldap_escape_f($email).')');
      if ($registrations > 0) {
        $errors[] = sprintf(_('There is already a registration object for "%s"'), $email);
        $failed[] = $email;
        continue;
      }
      $token  = standAlonePage::generateRandomHash();
      $body   = sprintf($this->fdInvitationEmailContent, $token);
      if (mail_utf8($email, $ui->cn, $config->get_cfg_value('InvitationFromEmailAddress'), $this->fdInvitationEmailSubject, $body, FALSE, $this->reply_to)) {
        $tabObject  = objects::create('registration');
        $baseObject = $tabObject->getBaseObject();

        $baseObject->base                       = $this->base;
        $baseObject->fdRegistrationEmailAddress = $email;
        $baseObject->fdRegistrationState        = 'sent';
        $baseObject->fdRegistrationLastChange   = date('c').'|'.$ui->dn;
        $baseObject->fdRegistrationInvitationDN = $this->dn;
        $baseObject->fdRegistrationToken        = $token;

        $messages = $tabObject->save();
        if (!empty($messages)) {
          $errors[] = sprintf(_('Failed to create registration object for "%s", because of the following errors: %s'), $email, implode("\n", $messages));
          $failed[] = $email;
        } else {
          $success++;
        }
      } else {
        $errors[] = sprintf(_('Failed to send invitation email to "%s". Check your email settings.'), $email);
        $failed[] = $email;
      }
    }

    $this->emails = implode("\n", $failed);

    if ($success > 0) {
      msg_dialog::display(_('Success'), sprintf(_('%d invitations were sent and saved successfully'), $success), INFO_DIALOG);
    }

    return $errors;
  }
}
