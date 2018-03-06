<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2009-2010 Bas Tichelaar
  Copyright (C) 2013-2016  FusionDirectory

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

class GroupMailsAttribute extends MailsAttribute
{
  function getForbiddenValues ()
  {
    $forbidden = array_values($this->getValue());
    $forbidden = array_merge($forbidden, array_values($this->plugin->attributesAccess['gosaMailAlternateAddress']->getValue()));
    $forbidden[] = $this->plugin->attributesAccess['mail']->getValue();
    return $forbidden;
  }

  function getFilterBlackList ()
  {
    return array('mail' => $this->getForbiddenValues());
  }
}

class mailGroup extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('fdGroupMail');
  /* Return plugin information for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Mail'),
      'plDescription'   => _('Group mail options'),
      'plFilter'        => '(objectClass=fdGroupMail)',
      'plIcon'          => 'geticon.php?context=applications&icon=internet-mail&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=internet-mail&size=16',
      'plPriority'      => 10,
      'plObjectType'    => array('group', 'ogroup-user'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Information'),
        'attrs' => array(
          new MailAttribute (
            _('Primary address'), _('The primary mail address'),
            'mail', TRUE
          ),
          new SelectAttribute (
            _('Server'), _('Email server'),
            'gosaMailServer', TRUE,
            array('')
          ),
          new SetAttribute (
            new StringAttribute (
              _('Alternative addresses'), _('Alternative mail addresses for the group'),
              'gosaMailAlternateAddress'
            )
          ),
          new GroupMailsAttribute (
            _('Forward messages to non group members'), _('Forward messages to non group members'),
            'gosaMailForwardingAddress'
          ),
          new BooleanAttribute (
            _('Only allowed to receive local mail'), _('Whether this group mail is only allowed to receive messages from local senders'),
            'fdGroupMailLocalOnly', FALSE,
            FALSE
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['gosaMailServer']->setChoices(array_keys(mailMethod::getMailServers()));
    $this->attributesAccess['gosaMailServer']->setSubmitForm('mailServerChanged');
    $this->mailServerChanged();
  }

  /* We need $mailAccount->cn to return the cn for mailMethod */
  public function __get($name)
  {
    if (($name == 'cn') && isset($this->parent)) {
      return $this->parent->getBaseObject()->$name;
    } else {
      /* Calling default behaviour */
      return parent::__get($name);
    }
  }

  public function __isset($name)
  {
    if (($name == 'cn') && isset($this->parent)) {
      return isset($this->parent->getBaseObject()->$name);
    } else {
      /* Calling default behaviour */
      return parent::__isset($name);
    }
  }

  public function mailServerChanged()
  {
    /* Intialize the used mailMethod */
    if ($this->gosaMailServer == '') {
      $method = 'mailMethod';
    } else {
      $method = mailMethod::getServerMethod($this->gosaMailServer);
    }
    $this->mailMethod = mailMethod::getInstance($method, $this);
    $this->mailMethod->fixAttributesOnLoad();

    /* Initialize configured values */
    if (!$this->is_template && $this->is_account && $this->initially_was_account) {
      $this->attributesAccess['mail']->setDisabled(!$this->mailMethod->isModifyableMail());
      $this->attributesAccess['gosaMailServer']->setDisabled(!$this->mailMethod->isModifyableServer());
    }
  }

  protected function shouldSave()
  {
    /* mail method might have something to save (like password change from main tab for instance) */
    return TRUE;
  }

  public function ldap_save()
  {
    if (!empty($this->attrs)) {
      $errors = parent::ldap_save();
      if (!empty($errors)) {
        return $errors;
      }
    }

    $errors = array();

    /* Only do IMAP actions if we are not a template */
    if (!$this->is_template) {
      $this->mailMethod->connect();
      if (!$this->mailMethod->is_connected()) {
        $errors[] = sprintf(_('Mail method cannot connect: %s'), $this->mailMethod->get_error());
      } else {
        if (!$this->mailMethod->updateMailbox()) {
          $errors[] = sprintf(_('Cannot update mailbox: %s'), $this->mailMethod->get_error());
        }
      }
    }
    $this->mailMethod->disconnect();

    if (!empty($errors) && !$this->initially_was_account) {
      /* Mail method failed to create, cancel ldap save */
      $this->prepareNextCleanup();
      $this->prepare_remove();
      $this->ldap_remove();
    }

    return $errors;
  }

  function post_remove()
  {
    /* Let the mailMethod remove this mailbox */
    if (!$this->is_template) {
      if (!$this->mailMethod->connect()) {
        msg_dialog::display(_('Mail error'), sprintf(_('Cannot remove mailbox, mail method cannot connect: %s'),
              $this->mailMethod->get_error()), ERROR_DIALOG);
      } else {
        if (!$this->mailMethod->deleteMailbox()) {
          msg_dialog::display(_('Mail error'), sprintf(_('Cannot remove mailbox: %s'),
                $this->mailMethod->get_error()), ERROR_DIALOG);
        }
      }
    }
    $this->mailMethod->disconnect();
    parent::post_remove();
  }
}
