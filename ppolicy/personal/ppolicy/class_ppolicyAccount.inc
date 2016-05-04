<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2015  Dhatim
  Copyright (C) 2014-2016  FusionDirectory

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

class ppolicyAccount extends simplePlugin
{
  var $objectclasses = array();

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Password policy'),
      'plDescription'   => _('Edit user\'s password policy'),
      'plSelfModify'    => TRUE,
      'plObjectType'    => array('user'),
      'plIcon'          => 'geticon.php?context=applications&icon=ppolicy&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=ppolicy&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Password policy'),
        'attrs' => array(
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
            array('', 'TRUE', 'FALSE'), '',
            array('', _('Force password change (resets locking)'), _('Reset locking (same password)'))
          )
        )
      )
    );
  }

  function __construct ($unused, $dn = NULL, $parent = NULL)
  {
    $attributesInfo = $this->getAttributesInfo();

    $policies = objects::ls('ppolicy');
    $choices  = array_keys($policies);
    $outputs  = array_values($policies);
    array_unshift($choices, '');
    array_unshift($outputs, _('Use the default'));
    $attributesInfo['main']['attrs'][0]->setChoices($choices, $outputs);

    parent::__construct($unused, $dn, $parent, $attributesInfo);

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

  protected function loadAttributes()
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

  protected function prepare_save ()
  {
    parent::prepare_save();
    if (!empty($this->pwdReset)) {
      $this->attrs['pwdAccountLockedTime'] = array();
    } else {
      unset($this->attrs['pwdAccountLockedTime']);
    }
  }

  function remove_from_parent()
  {
    /* ppolicy fields are not real fields, they can’t be removed through ldap_modify
     * It’s not possible to remove ppolicy info from a user node anyway */
  }
}
?>
