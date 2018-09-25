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
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Invitation'),
      'plDescription' => _('Invitation to fill a public form'),
      'plObjectClass' => array('fdInvitation'),
      'plObjectType'  => array('invitation' => array(
        'name'    => _('Invitation'),
        'ou'      => get_ou('invitationRDN'),
        'icon'    => 'geticon.php?context=types&icon=application&size=16',
      )),
      'plForeignKeys'   => array(
        'fdInvitationFormDN' => 'publicForm',
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    global $ui;
    return array(
      'main' => array(
        'name'  => _('Invitation'),
        'attrs' => array(
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
            'Hello,'."\n\n".
              'Please subscribe by going to https://'.$_SERVER['SERVER_NAME'].
              (($_SERVER['SERVER_PORT'] != '80') ? ':'.$_SERVER['SERVER_PORT'] : '').
              "/invitation.php?token=%s\n"
          ),
          new DisplayLDAPAttribute(
            _('Author'), _('Author of this invitation'),
            'fdInvitationAuthor', FALSE,
            $ui->dn
          ),
        )
      ),
      'emails' => array(
        'name'  => _('Emails'),
        'attrs' => array(
          new StringAttribute(
            _('From email'), _('Email address set as "From" field and "Reply to" in the sent emails'),
            'from_mail', FALSE
          ),
          new FileTextAreaAttribute(
            _('Emails'), _('List of email adresses of people to invite - one per line'),
            'emails', FALSE
          ),
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    if ($attributesInfo === NULL) {
      $attributesInfo = $this->getAttributesInfo();
    }
    $choices = objects::ls('publicForm');
    $attributesInfo['main']['attrs'][2]->setChoices(array_keys($choices), array_values($choices));

    $attributesInfo['emails']['attrs'][0]->setInLdap(FALSE);
    $attributesInfo['emails']['attrs'][1]->setInLdap(FALSE);

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
  }

  function check()
  {
    $messages = parent::check();

    $emails = explode("\n", $this->emails);
    foreach ($emails as $email) {
      if (!tests::is_email($email)) {
        $messages[] = sprintf(_('"%s" is not a valid email address'), $email);
      }
    }

    if (!empty($emails) && empty($this->from_mail)) {
      $messages[] = sprintf(_('You need to fill the "%s" field in order to send invitations'), $this->attributesAccess['from_mail']->getLabel());
    }

    return $messages;
  }

  protected function shouldSave()
  {
    return (parent::shouldSave() || !empty($this->emails));
  }

  protected function ldap_save ()
  {
    global $ui;

    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    $emails   = explode("\n", $this->emails);
    $success  = 0;
    foreach ($emails as $email) {
      if (empty($email)) {
        continue;
      }
      $token  = standAlonePage::generateRandomHash();
      $body   = sprintf($this->fdInvitationEmailContent, $token);
      if (mail_utf8($email, FALSE, $this->from_mail, $this->fdInvitationEmailSubject, $body)) {
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
        } else {
          $success++;
        }
      } else {
        $errors[] = sprintf(_('Failed to send invitation email to "%s". Check your email settings.'), $email);
      }
    }

    if ($success > 0) {
      msg_dialog::display(_('Success'), sprintf(_('%d invitations were sent and saved successfully'), $success), INFO_DIALOG);
    }

    return $errors;
  }
}
