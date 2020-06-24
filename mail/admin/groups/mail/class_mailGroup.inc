<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2009-2010 Bas Tichelaar
  Copyright (C) 2013-2019 FusionDirectory

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
    return ['mail' => $this->getForbiddenValues()];
  }
}

class mailGroup extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Mail'),
      'plDescription'   => _('Group mail options'),
      'plFilter'        => '(objectClass=fdGroupMail)',
      'plIcon'          => 'geticon.php?context=applications&icon=internet-mail&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=internet-mail&size=16',
      'plPriority'      => 10,
      'plObjectClass'   => ['fdGroupMail'],
      'plObjectType'    => ['group', 'ogroup-user'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Information'),
        'attrs' => [
          new MailAttribute(
            _('Primary address'), _('The primary mail address'),
            'mail', TRUE
          ),
          new SelectAttribute(
            _('Server'), _('Email server'),
            'gosaMailServer', FALSE,
            []
          ),
          new SetAttribute(
            new StringAttribute(
              _('Alternative addresses'), _('Alternative mail addresses for the group'),
              'gosaMailAlternateAddress'
            )
          ),
        ]
      ],
      'options' => [
        'name'  => _('Options'),
        'attrs' => [
          new GroupMailsAttribute(
            _('Forward messages to non group members'), _('Forward messages to non group members'),
            'gosaMailForwardingAddress'
          ),
          new BooleanAttribute(
            _('Only allowed to receive local mail'), _('Whether this group mail is only allowed to receive messages from local senders'),
            'fdGroupMailLocalOnly', FALSE,
            FALSE
          ),
          new SelectAttribute(
            _('Default ACL'), _('ACL for people not in the group'),
            'defaultACL', FALSE
          ),
          new SelectAttribute(
            _('Members ACL'), _('ACL for members of the group'),
            'fdGroupMailMembersAcl', FALSE
          ),
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Special ACLs for specific members'),
              'specialACLs',
              [
                new SelectAttribute(
                  '', _('Email'),
                  'specialACLsEmail', TRUE,
                  []
                ),
                new SelectAttribute(
                  '', _('ACL'),
                  'specialACLsAcl', TRUE,
                  []
                ),
              ],
              '',
              _('Special ACLs')
            ),
            // no order
            FALSE,
            []
          ),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['defaultACL']->setInLdap(FALSE);
    $this->attributesAccess['specialACLs']->setInLdap(FALSE);

    $this->attributesAccess['gosaMailServer']->setChoices(array_keys(mailMethod::getMailServers()));
    $this->attributesAccess['gosaMailServer']->setSubmitForm('mailServerChanged');
    $this->mailServerChanged();

    $this->attributesAccess['specialACLs']->attribute->attributes[0]->setChoices($this->getMemberEmailList());
  }

  /* We need $mailAccount->cn to return the cn for mailMethod */
  public function __get ($name)
  {
    if (($name == 'cn') && isset($this->parent)) {
      return $this->parent->getBaseObject()->$name;
    } else {
      /* Calling default behaviour */
      return parent::__get($name);
    }
  }

  public function __isset ($name)
  {
    if (($name == 'cn') && isset($this->parent)) {
      return isset($this->parent->getBaseObject()->$name);
    } else {
      /* Calling default behaviour */
      return parent::__isset($name);
    }
  }

  function foreignKeyUpdate (string $field, $oldvalue, $newvalue, array $source)
  {
    if ($field == 'gosaMailServer') {
      mailMethod::resetMailServersCache();
      $this->attributesAccess['gosaMailServer']->setChoices(array_keys(mailMethod::getMailServers()));
    }
    parent::foreignKeyUpdate($field, $oldvalue, $newvalue, $source);
  }

  public function mailServerChanged ()
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

    /* Feature switches */

    if ($this->mailMethod->localOnlyEnabled()) {
      $this->attributesAccess['fdGroupMailLocalOnly']->setDisabled(FALSE);
      $this->attributesAccess['fdGroupMailLocalOnly']->setVisible(TRUE);
    } else {
      $this->attributesAccess['fdGroupMailLocalOnly']->setDisabled(TRUE);
      $this->attributesAccess['fdGroupMailLocalOnly']->setVisible(FALSE);
      $this->attributesAccess['fdGroupMailLocalOnly']->setValue(FALSE);
    }

    if ($this->mailMethod->groupForwardingEnabled()) {
      $this->attributesAccess['gosaMailForwardingAddress']->setDisabled(FALSE);
      $this->attributesAccess['gosaMailForwardingAddress']->setVisible(TRUE);
    } else {
      $this->attributesAccess['gosaMailForwardingAddress']->setDisabled(TRUE);
      $this->attributesAccess['gosaMailForwardingAddress']->setVisible(FALSE);
      $this->attributesAccess['gosaMailForwardingAddress']->setValue([]);
    }

    if ($this->mailMethod->groupAclsEnabled()) {
      $this->attributesAccess['defaultACL']->setDisabled(FALSE);
      $this->attributesAccess['defaultACL']->setVisible(TRUE);
      $this->attributesAccess['fdGroupMailMembersAcl']->setDisabled(FALSE);
      $this->attributesAccess['fdGroupMailMembersAcl']->setVisible(TRUE);
      $this->attributesAccess['specialACLs']->setDisabled(FALSE);
      $this->attributesAccess['specialACLs']->setVisible(TRUE);

      $this->attributesAccess['defaultACL']->setDefaultValue('p');
      $this->attributesAccess['fdGroupMailMembersAcl']->setDefaultValue('lrsp');

      if ($this->mailMethod->connect()) {
        $acls = $this->mailMethod->getFolderACLs();
      }
      $aclTypes = $this->mailMethod->getAclTypes();
      $this->attributesAccess['defaultACL']->setChoices(array_keys($aclTypes), array_values($aclTypes));
      $this->attributesAccess['fdGroupMailMembersAcl']->setChoices(array_keys($aclTypes), array_values($aclTypes));
      $this->attributesAccess['specialACLs']->attribute->attributes[1]->setChoices(array_keys($aclTypes), array_values($aclTypes));
      if (isset($acls)) {
        $this->attributesAccess['defaultACL']->setValue($acls['anyone']);
        $specialACLs = [];
        foreach ($acls as $email => $acl) {
          if (($email != 'anyone') && ($acl != $this->fdGroupMailMembersAcl)) {
            $specialACLs[] = $email.'|'.$acl;
          }
        }
        $this->specialACLs = $specialACLs;
      }
    } else {
      $this->attributesAccess['defaultACL']->setDisabled(TRUE);
      $this->attributesAccess['defaultACL']->setVisible(FALSE);
      $this->attributesAccess['defaultACL']->setValue('');
      $this->attributesAccess['fdGroupMailMembersAcl']->setDisabled(TRUE);
      $this->attributesAccess['fdGroupMailMembersAcl']->setVisible(FALSE);
      $this->attributesAccess['fdGroupMailMembersAcl']->setValue('');
      $this->attributesAccess['specialACLs']->setDisabled(TRUE);
      $this->attributesAccess['specialACLs']->setVisible(FALSE);
      $this->attributesAccess['specialACLs']->setValue([]);
    }
  }

  public function check (): array
  {
    if ($this->is_account) {
      $this->attributesAccess['specialACLs']->attribute->attributes[0]->setChoices($this->getMemberEmailList());
    }

    $errors = parent::check();

    if (!$this->is_template && is_object($this->mailMethod)) {
      $errors = array_merge($errors, $this->mailMethod->check());
    }

    return $errors;
  }

  protected function shouldSave (): bool
  {
    /* mail method might have something to save (like password change from main tab for instance) */
    return TRUE;
  }

  protected function getMemberEmailList (): array
  {
    global $config;

    $mainTab = $this->parent->getBaseObject();
    $newmembers = [];
    if (isset($mainTab->member)) {
      $ldap = $config->get_ldap_link();
      $dns  = $mainTab->member;
      foreach ($dns as $dn) {
        $ldap->cat($dn);
        $attrs = $ldap->fetch();
        if (isset($attrs['mail'][0])) {
          $newmembers[] = $attrs['mail'][0];
        }
      }
    } elseif (isset($mainTab->memberUid)) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $uids = $mainTab->memberUid;
      foreach ($uids as $uid) {
        $ldap->search('(&(objectClass=inetOrgPerson)(uid='.$uid.'))');
        if ($ldap->count() > 1) {
          $error = new SimplePluginError(
            $this,
            htmlescape(sprintf(
              _('Several users have uid "%s". Ignoring this member.'),
              $uid
            ))
          );
          $error->display();
        } elseif ($ldap->count() == 1) {
          $attrs = $ldap->fetch();
          if (isset($attrs['mail'][0])) {
            $newmembers[] = $attrs['mail'][0];
          }
        }
      }
    }

    return $newmembers;
  }

  public function execute (): string
  {
    if ($this->is_account) {
      $this->attributesAccess['specialACLs']->attribute->attributes[0]->setChoices($this->getMemberEmailList());
    }
    return parent::execute();
  }

  public function save_object ()
  {
    if ($this->is_account) {
      $this->attributesAccess['specialACLs']->attribute->attributes[0]->setChoices($this->getMemberEmailList());
    }
    parent::save_object();
  }

  public function ldap_save (): array
  {
    if (!empty($this->attrs)) {
      $errors = parent::ldap_save();
      if (!empty($errors)) {
        return $errors;
      }
    }

    $errors = [];

    /* Only do IMAP actions if we are not a template */
    if (!$this->is_template) {
      $this->mailMethod->connect();
      if (!$this->mailMethod->is_connected()) {
        $errors[] = new SimplePluginError(
          $this,
          htmlescape(sprintf(_('Mail method cannot connect: %s'), $this->mailMethod->get_error()))
        );
      } else {
        if (!$this->mailMethod->updateMailbox()) {
          $errors[] = new SimplePluginError(
            $this,
            htmlescape(sprintf(_('Cannot update mailbox: %s'), $this->mailMethod->get_error()))
          );
        }
        if ($this->mailMethod->groupAclsEnabled()) {
          $acls = [
            'anyone' => $this->defaultACL
          ];

          foreach ($this->getMemberEmailList() as $member) {
            $acls[$member] = $this->fdGroupMailMembersAcl;
          }

          foreach ($this->specialACLs as $specialACL) {
            list($email, $acl) = explode('|', $specialACL, 2);
            $acls[$email] = $acl;
          }

          $this->mailMethod->setFolderACLs($acls);
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

  function post_remove ()
  {
    /* Let the mailMethod remove this mailbox */
    if (!$this->is_template) {
      if (!$this->mailMethod->connect()) {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Cannot remove mailbox, mail method cannot connect: %s'),
            $this->mailMethod->get_error()
          ))
        );
        $error->display();
      } else {
        if (!$this->mailMethod->deleteMailbox()) {
          $error = new FusionDirectoryError(
            htmlescape(sprintf(
              _('Cannot remove mailbox: %s'),
              $this->mailMethod->get_error()
            ))
          );
          $error->display();
        }
      }
    }
    $this->mailMethod->disconnect();
    parent::post_remove();
  }
}
