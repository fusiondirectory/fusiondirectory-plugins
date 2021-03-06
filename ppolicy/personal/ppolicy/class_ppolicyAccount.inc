<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2014-2015  Dhatim
  Copyright (C) 2014-2019  FusionDirectory

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

class ppolicyAccount extends simplePlugin implements UserTabLockingAction
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Password policy'),
      'plDescription'   => _('Edit user\'s password policy'),
      'plSelfModify'    => TRUE,
      'plObjectType'    => ['user', 'simpleSecurityObject'],
      'plIcon'          => 'geticon.php?context=applications&icon=ppolicy&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=ppolicy&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Password policy'),
        'attrs' => [
          new SelectAttribute(
            _('Policy'), _('Use a specific policy for this user'),
            'pwdPolicySubentry', FALSE
          ),
          new ReadOnlyLDAPAttribute(
            _('Last password change'), _('Last time the password for the entry was changed'),
            'pwdChangedTime', FALSE
          ),
          new DisplayLDAPAttribute(
            _('Account locked time'), _('Time the account was locked'),
            'pwdAccountLockedTime', FALSE
          ),
          new SelectAttribute(
            _('Reset locking / force change'), _('Resets the lock status of this account and/or force a password change'),
            'pwdReset', FALSE,
            ['', 'TRUE', 'FALSE'], '',
            ['', _('Force password change (resets locking)'), _('Reset locking (same password)')]
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    $attributesInfo = $this->getAttributesInfo();

    $policies = objects::ls('ppolicy');
    $choices  = array_keys($policies);
    $outputs  = array_values($policies);
    array_unshift($choices, '');
    array_unshift($outputs, _('Use the default'));
    $attributesInfo['main']['attrs'][0]->setChoices($choices, $outputs);

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    if ($this->pwdChangedTime == '') {
      $this->pwdChangedTime = _('Never');
    } else {
      $date = LdapGeneralizedTime::fromString($this->pwdChangedTime);
      $date->setTimezone(timezone::getDefaultTimeZone());
      $this->pwdChangedTime = $date->format('Y-m-d H:i:s');
    }
    if ($this->pwdAccountLockedTime == '') {
      $this->pwdAccountLockedTime = _('Unlocked');
    } elseif ($this->pwdAccountLockedTime == '000001010000Z') {
      $this->pwdAccountLockedTime = _('Locked permanently');
    } else {
      $date = LdapGeneralizedTime::fromString($this->pwdAccountLockedTime);
      $date->setTimezone(timezone::getDefaultTimeZone());
      $this->pwdAccountLockedTime = $date->format('Y-m-d H:i:s');
    }
  }

  protected function loadAttributes ()
  {
    global $config;
    if ($this->dn != 'new') {
      $ldap = $config->get_ldap_link();
      $ldap->cat($this->dn, $this->attributes);
      $attrs = $ldap->fetch();
      $this->attrs = array_merge($this->attrs, $attrs);
    }
    parent::loadAttributes();
  }

  protected function prepare_save (): array
  {
    $errors = parent::prepare_save();
    if (!empty($this->pwdReset)) {
      $this->attrs['pwdAccountLockedTime'] = [];
    } else {
      unset($this->attrs['pwdAccountLockedTime']);
    }
    return $errors;
  }

  function remove (bool $fulldelete = FALSE): array
  {
    /* ppolicy fields are not real fields, they can’t be removed through ldap_modify
     * It’s not possible to remove ppolicy info from a user node anyway */
    return [];
  }

  public function fillLockingLDAPAttrs (string $mode, array &$modify)
  {
    if ($mode == 'LOCK') {
      $modify['pwdAccountLockedTime'] = '000001010000Z';
    }
    /* For unlock there is nothing to do as core unlock changes the password it removes ppolicy lock anyway */
  }
}
