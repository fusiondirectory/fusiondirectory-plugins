<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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
          'mainAttr'    => 'sambaDomainName',
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
            _('Minimum password age'),
            _('Minimum password age, in seconds (default: 0 => allow immediate password change)'),
            'sambaMinPwdAge', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            _('Maximum password age'),
            _('Maximum password age, in seconds (default: -1 => never expire passwords)'),
            'sambaMaxPwdAge', FALSE,
            -1, FALSE, ''
          ),
          new DisplayLDAPAttribute(
            _('Next RID'),
            _('Next NT rid to give out for anything'),
            'sambaNextRid'
          ),
          new DisplayLDAPAttribute(
            _('Next group RID'),
            _('Next NT rid to give out for groups'),
            'sambaNextGroupRid'
          ),
          new DisplayLDAPAttribute(
            _('Next user RID'),
            _('Next NT rid to give our for users'),
            'sambaNextUserRid'
          ),
          new DisplayLDAPAttribute(
            _('Minimum password length'),
            _('Minimal password length (default: 5)'),
            'sambaMinPwdLength'
          ),
          new DisplayLDAPAttribute(
            _('Password history length'),
            _('Length of Password History Entries (default: 0 => off)'),
            'sambaPwdHistoryLength'
          ),
          new DisplayLDAPAttribute(
            _('Logon to change password'),
            _('Force Users to logon for password change (default: 0 => off, 2 => on)'),
            'sambaLogonToChgPwd'
          ),
          new DisplayLDAPAttribute(
            _('Lockout duration'),
            _('Lockout duration in minutes (default: 30, -1 => forever)'),
            'sambaLockoutDuration'
          ),
          new DisplayLDAPAttribute(
            _('Lockout observation window'),
            _('Reset time after lockout in minutes (default: 30)'),
            'sambaLockoutObservationWindow'
          ),
          new DisplayLDAPAttribute(
            _('Lockout threshold'),
            _('Lockout users after bad logon attempts (default: 0 => off)'),
            'sambaLockoutThreshold'
          ),
          new DisplayLDAPAttribute(
            _('Force logoff'),
            _('Disconnect Users outside logon hours (default: -1 => off, 0 => on)'),
            'sambaForceLogoff'
          ),
          new DisplayLDAPAttribute(
            _('Refuse machine password change'),
            _('Allow Machine Password changes (default: 0 => off)'),
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

  function compute_dn ()
  {
    global $config;
    return 'sambaDomainName='.$this->sambaDomainName.','.$config->current['BASE'];
  }
}
