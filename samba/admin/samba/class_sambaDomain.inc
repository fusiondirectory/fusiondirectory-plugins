<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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

class sambaDomain extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('sambaDomain');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Samba Domain'),
      'plDescription' => _('Samba Domain'),
      'plObjectType'  => array(
        'sambaDomain' => array(
          'name'        => _('Samba Domain'),
          'description' => _('Samba domain settings'),
          'filter'      => '(objectClass=sambaDomain)',
          'icon'        => 'geticon.php?context=applications&icon=samba&size=16',
          'ou'          => '',
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new StringAttribute (
            _('Domain name'), _('Name of this domain'),
            'sambaDomainName', TRUE
          ),
          new StringAttribute (
            _('SID'), _('SID of this domain'),
            'sambaSID', TRUE
          ),
          new StringAttribute (
            _('Rid base'), _('Algorithmic rid base'),
            'sambaAlgorithmicRidBase', FALSE
          ),
          new IntAttribute(
            _('Minimum password age'), 'sambaMinPwdAge',
            'sambaMinPwdAge', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            _('Maximum password age'), 'sambaMaxPwdAge',
            'sambaMaxPwdAge', FALSE,
            0, FALSE, ''
          ),
          new DisplayLDAPAttribute(
            _('Next RID'), 'sambaNextRid',
            'sambaNextRid'
          ),
          new DisplayLDAPAttribute(
            _('Next group RID'), 'sambaNextGroupRid',
            'sambaNextGroupRid'
          ),
          new DisplayLDAPAttribute(
            _('Next user RID'), 'sambaNextUserRid',
            'sambaNextUserRid'
          ),
          new DisplayLDAPAttribute(
            _('Minimum password length'), 'sambaMinPwdLength',
            'sambaMinPwdLength'
          ),
          new DisplayLDAPAttribute(
            _('Password history length'), 'sambaPwdHistoryLength',
            'sambaPwdHistoryLength'
          ),
          new DisplayLDAPAttribute(
            _('Logon to change password'), 'sambaLogonToChgPwd',
            'sambaLogonToChgPwd'
          ),
          new DisplayLDAPAttribute(
            _('Lockout duration'), 'sambaLockoutDuration',
            'sambaLockoutDuration'
          ),
          new DisplayLDAPAttribute(
            _('Lockout observation window'), 'sambaLockoutObservationWindow',
            'sambaLockoutObservationWindow'
          ),
          new DisplayLDAPAttribute(
            _('Lockout threshold'), 'sambaLockoutThreshold',
            'sambaLockoutThreshold'
          ),
          new DisplayLDAPAttribute(
            _('Force logoff'), 'sambaForceLogoff',
            'sambaForceLogoff'
          ),
          new DisplayLDAPAttribute(
            _('Refuse machine password change'), 'sambaRefuseMachinePwdChange',
            'sambaRefuseMachinePwdChange'
          ),
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }
}
