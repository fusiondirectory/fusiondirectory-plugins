<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2014-2015 Dhatim
  Copyright (C) 2014-2016 FusionDirectory

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

class ppolicy extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('device', 'pwdPolicy');

  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Password policy'),
      'plDescription' => _('Password policy for ppolicy overlay'),
      'plObjectType'  => array('ppolicy' => array(
        'name'        => _('Password policy'),
        'filter'      => 'objectClass=pwdPolicy',
        'ou'          => get_ou('ppolicyRDN'),
        'icon'        => 'geticon.php?context=applications&icon=ppolicy&size=16'
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('Policy'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('ppolicyRDN')),
          new HiddenAttribute('pwdAttribute', TRUE, 'userPassword'),
          new HostNameAttribute (
            _('Policy name'), _('Policy name'),
            'cn', TRUE
          ),
          new TextAreaAttribute(
            _('Description'), _('A short description of this policy'),
            'description', FALSE
          ),
          new IntAttribute(
            _('Minimum length'), _('Minimum length of the user supplied password - passwords shorter than this value will be rejected'),
            'pwdMinLength', FALSE,
            0, FALSE
          ),
          new IntAttribute(
            _('Passwords in history'), _('Number of passwords that are maintained in a list of previously used passwords'),
            'pwdInHistory', FALSE,
            0, FALSE
          ),
          new TimeAttribute(
            _('Minimum password age'), _('Minimum time between password changes'),
            'pwdMinAge', FALSE,
            0, FALSE
          ),
          new TimeAttribute(
            _('Maximum password age'), _('Maximum time a password is valid, after which it is deemed to be no longer usable and any bind operations attempted with the expired password will be treated as invalid'),
            'pwdMaxAge', FALSE,
            0, FALSE
          ),
          new TimeAttribute(
            _('Expiry warning delay'), _('Defines the start time - in seconds - prior to the password expiry that password expiry warning messages are returned in bind responses. 0 to disable'),
            'pwdExpireWarning', FALSE,
            0, FALSE
          ),
          new IntAttribute(
            _('Grace period'), _('Number of times a user is allowed to successfully bind using an expired password'),
            'pwdGraceAuthNLimit', FALSE,
            0, FALSE
          ),
          new BooleanAttribute(
            _('Allow user change'), _('Whether users are allowed to change their own passwords'),
            'pwdAllowUserChange', FALSE,
            TRUE // default
          ),
          new BooleanAttribute(
            _('Safe modify'), _('Whether a user must send the current password during a password modification operation'),
            'pwdSafeModify', FALSE,
            FALSE // default
          ),
          //~ new SelectAttribute(
            //~ _('Check quality'), _('Decides what to do if the function in "Check module" is not available'),
            //~ 'pwdCheckQuality', FALSE,
            //~ array(0, 1, 2), 0,
            //~ array(_('Disabled'), _('Ignore errors'), _('Reject on errors'))
          //~ ),
          new HiddenAttribute('pwdCheckQuality', FALSE, 0),
        )
      ),
      'lockout' => array(
        'name'  => _('Lock out'),
        'attrs' =>
        array(
          new BooleanAttribute(
            _('Activate lock out'), _('Whether to lock an account that had more consecutive failed bind attempts with invalid passwords than is defined by "Maximum failures"'),
            'pwdLockout', FALSE,
            TRUE // default
          ),
          new TimeAttribute(
            _('Lock out duration'), _('Time the account remains locked after an automatic lock out (0 means for ever)'),
            'pwdLockoutDuration', FALSE,
            0, FALSE
          ),
          new IntAttribute(
            _('Maximum failures'), _('Number of consecutive password failures allowed before automatic lock out'),
            'pwdMaxFailure', FALSE,
            0, FALSE
          ),
          new TimeAttribute(
            _('Failure count interval'), _('Time after which the count of consecutive password failures is reset even if no successful authentication has occurred'),
            'pwdFailureCountInterval', FALSE,
            0, FALSE
          ),
          new BooleanAttribute(
            _('Must change'), _('Whether the user must change their password after an account is reset by an administrator following an automatic lockout'),
            'pwdMustChange', FALSE,
            FALSE // default
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['pwdMaxAge']->attributes[0]->setManagedAttributes(
      array (
        'disable' => array (
          0 => array (
            'pwdExpireWarning', 'pwdGraceAuthNLimit'
          )
        )
      )
    );
    $this->attributesAccess['pwdLockout']->setManagedAttributes(
      array (
        'disable' => array (
          FALSE => array (
            'pwdLockoutDuration',       'pwdMaxFailure',
            'pwdFailureCountInterval',  'pwdMustChange'
          )
        )
      )
    );
    $this->attributesAccess['pwdAllowUserChange']->setManagedAttributes(
      array (
        'disable' => array (
          FALSE => array (
            'pwdSafeModify',
          )
        )
      )
    );

    if (objects::count('ppolicy') == 0) {
      $this->attributesAccess['cn']->setDefaultValue($config->get_cfg_value('ppolicyDefaultCn', 'default'));
      $this->attributesAccess['cn']->resetToDefault();
    }
  }
}
?>
