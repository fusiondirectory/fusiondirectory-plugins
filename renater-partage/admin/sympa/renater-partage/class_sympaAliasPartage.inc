<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017 FusionDirectory

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

class sympaAliasPartage extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('fdRenaterPartageSympaAlias');

  static function plInfo()
  {
    return array(
      'plShortName'    => _('Partage'),
      'plDescription'  => _('Partage sympa options'),
      'plObjectType'   => array('sympaAlias'),
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Information'),
        'attrs' => array(
          new SelectAttribute(
            _('Server'), _('Email server'),
            'gosaMailServer', TRUE,
            array('')
          ),
          new StringAttribute (
            _('Display name'), _('Name to display for this group'),
            'displayName'
          ),
          new BooleanAttribute (
            _('Hide from global catalog'), _('Whether this group should be hidden from the global catalog'),
            'fdRenaterPartageGroupHideInGal', FALSE,
            FALSE
          ),
          new HiddenAttribute (
            'fdRenaterPartageGroupMailStatus', TRUE, 'enabled'
          ),
          new BooleanAttribute (
            'fdRenaterPartageGroupSendShareMessageToNewMembers', '',
            'fdRenaterPartageGroupSendShareMessageToNewMembers', FALSE,
            FALSE
          ),
          new TextAreaAttribute (
            _('Notes'), _('Notes about this group'),
            'fdRenaterPartageGroupNotes', FALSE
          ),
          new SetAttribute (
            new StringAttribute (
              _('Alternative addresses'), _('Alternative mail addresses for the list'),
              'gosaMailAlternateAddress'
            )
          ),
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['fdRenaterPartageGroupSendShareMessageToNewMembers']->setVisible(FALSE);
    $this->attributesAccess['gosaMailServer']->setChoices(array_keys(mailMethodRenaterPartage::get_server_list()));
    $this->attributesAccess['gosaMailServer']->setSubmitForm('mailServerChanged');
    $this->mailServerChanged();
  }

  /* We need $mailAccount->cn to return the cn for mailMethod */
  public function __get($name)
  {
    if (($name == 'cn') && isset($this->parent)) {
      return $this->parent->getBaseObject()->$name;
    } elseif (($name == 'mail') && isset($this->parent)) {
      return $this->parent->getBaseObject()->cn.'@'.$this->getEmailDomain();
    } else {
      /* Calling default behaviour */
      return parent::__get($name);
    }
  }

  public function __isset($name)
  {
    if (($name == 'cn') && isset($this->parent)) {
      return isset($this->parent->getBaseObject()->$name);
    } elseif (($name == 'mail') && isset($this->parent)) {
      return isset($this->parent->getBaseObject()->cn);
    } else {
      /* Calling default behaviour */
      return parent::__isset($name);
    }
  }

  private function getEmailDomain()
  {
    $servers  = mailMethod::getMailServers();
    $infos    = $servers[$this->gosaMailServer];
    if (isset($infos['domain'])) {
      return $infos['domain'];
    } else {
      return '';
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
    $this->mailMethod = mailMethod::getInstance($method, $this, 'sympaAlias');
    $this->mailMethod->fixAttributesOnLoad();
  }

  protected function shouldSave()
  {
    /* mail method might have something to save */
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
