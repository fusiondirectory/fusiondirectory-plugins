<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class MungedAttribute extends Attribute
{
  protected $mungedObject;
  protected $attrList;
  protected $flagList;

  function __construct ($ldapName, $flagList, $acl = "")
  {
    parent::__construct('', _('Samba Munged Dial'), $ldapName, FALSE, '', $acl);
    $this->setVisible(FALSE);
    $this->mungedObject = new sambaMungedDial;
    $this->attrList     = $this->mungedObject->ctxattributes;
    $this->flagList     = $flagList;
  }

  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $this->setValue($this->inputValue($attrs[$this->getLdapName()][0]));
    } else {
      $this->resetToDefault();
    }
  }

  function setParent(&$plugin)
  {
    $this->plugin = $plugin;
    if (is_object($this->plugin)) {
      foreach ($this->attrList as $key => $attr) {
        if (isset($this->plugin->attributesAccess[$attr])) {
          $this->plugin->attributesAccess[$attr]->setInLdap(FALSE);
          $this->plugin->attributesAccess[$attr]->setAcl('sambaMungedDial');
        } else {
          unset($this->attrList[$key]);
        }
      }
      foreach ($this->flagList as $attr) {
        $this->plugin->attributesAccess[$attr]->setInLdap(FALSE);
        $this->plugin->attributesAccess[$attr]->setAcl('sambaMungedDial');
      }
    }
  }

  function setValue($value)
  {
    if (!empty($value)) {
      $this->mungedObject->load($value);
    }
    if (is_object($this->plugin)) {
      foreach ($this->attrList as $attr) {
        if (isset($this->mungedObject->ctx[$attr])) {
          $this->plugin->attributesAccess[$attr]->setValue($this->mungedObject->ctx[$attr]);
        }
      }
      foreach ($this->flagList as $attr) {
        $func = "get$attr";
        $this->plugin->$attr = $this->mungedObject->$func();
      }
    }
  }

  function getValue()
  {
    if (is_object($this->plugin)) {
      foreach ($this->attrList as $attr) {
        $this->mungedObject->ctx[$attr] = $this->plugin->$attr;
      }
      foreach ($this->flagList as $attr) {
        $func = "set$attr";
        $this->mungedObject->$func($this->plugin->$attr);
      }
    }
    return $this->mungedObject->getMunged();
  }
}

class SambaFlagsAttribute extends FlagsAttribute
{
  function getValue()
  {
    $value = '[U';
    if (is_object($this->plugin)) {
      foreach ($this->flags as $attr) {
        $ldapValue = $this->plugin->attributesAccess[$attr]->computeLdapValue();
        if ($ldapValue == '[LD]') {
          if (preg_match('/L/i', $this->initialValue)) {
            $ldapValue = 'L';
          } else {
            $ldapValue = 'D';
          }
        }
        $value .= $ldapValue;
      }
    }
    while (strlen($value) < 13) {
      $value .= ' ';
    }
    $value .= ']';
    return $value;
  }
}

class sambaAccount extends simplePlugin
{
  var $objectclasses = array('sambaSamAccount');

  var $displayHeader = TRUE;

  var $SID      = "";
  var $ridBase  = 0;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Samba'),
      'plDescription'   => _('Samba settings'),
      'plIcon'          => 'plugins/samba/images/icon.png',
      'plSmallIcon'     => 'plugins/samba/images/iconMini.png',
      'plSelfModify'    => TRUE,
      'plDepends'       => array('posixAccount'),
      'plPriority'      => 5,
      'plCategory'      => array('users'),
      'plObjectType'    => array('user'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    static $letters = array();
    if (empty($letters)) {
      $letters[] = '';
      for ($i = 68; $i < 91; $i++) {
        $letters[] = chr($i).':';
      }
    }
    static $sambaRegEx = '/^\\\\\\\\([a-z0-9%_.:$+-\\\\]+\\\\?)+$/i';
    return array(
      'generic' => array(
        'name'  => _('Generic'),
        'icon'  => 'images/rightarrow.png',
        'attrs' => array(
          new HiddenAttribute ('sambaSID'),
          new HiddenAttribute ('sambaPrimaryGroupSID'),
          new SelectAttribute (
            _('Home directory drive'), _('Letter for the home drive'),
            'sambaHomeDrive', FALSE,
            $letters
          ),
          new StringAttribute (
            _('Home directory path'), _('UNC path for the home drive'),
            'sambaHomePath', FALSE, '', '',
            $sambaRegEx
          ),
          new SelectAttribute (
            _('Domain'), _('Samba domain name'),
            'sambaDomainName', TRUE
          ),
          new StringAttribute (
            _('Script path'), _('Login script path'),
            'sambaLogonScript'
          ),
          new StringAttribute (
            _('Profile path'), _('UNC profile path'),
            'sambaProfilePath', FALSE, '', '',
            $sambaRegEx
          ),
        )
      ),
      'terminal' => array(
        'name'  => _('Terminal server'),
        'icon'  => 'plugins/systems/images/select_terminal.png',
        'attrs' => array(
          new BooleanAttribute (
            _('Allow login on terminal server'), _('Allow login on terminal server'),
            'TsLogin', FALSE
          ),
          new SelectAttribute (
            _('Home directory drive'), _('Letter for the home drive'),
            'CtxWFHomeDirDrive', FALSE,
            $letters
          ),
          new StringAttribute (
            _('Home directory path'), _('UNC path for the home drive'),
            'CtxWFHomeDir', FALSE, '', '',
            $sambaRegEx
          ),
          new StringAttribute (
            _('Profile path'), _('UNC profile path'),
            'CtxWFProfilePath', FALSE, '', '',
            $sambaRegEx
          ),
          new BooleanAttribute (
            _('Inherit client config'), _('Inherit client configuration'),
            'InheritMode', FALSE
          ),
          new StringAttribute (
            _('Initial progam'), _('Program to start after connecting'),
            'CtxInitialProgram'
          ),
          new StringAttribute (
            _('Working directory'), _('Basic working directory'),
            'CtxWorkDirectory'
          ),
          new IntAttribute (
            _('Connection timeout'), _('Timeout when connecting to terminal server'),
            'CtxMaxConnectionTime', FALSE,
            0, FALSE
          ),
          new IntAttribute (
            _('Disconnection timeout'), _('Timeout before disconnecting from terminal server'),
            'CtxMaxDisconnectionTime', FALSE,
            0, FALSE
          ),
          new IntAttribute (
            _('Idle timeout'), _('Idle timeout before disconnecting from terminal server'),
            'CtxMaxIdleTime', FALSE,
            0, FALSE
          ),
          new BooleanAttribute (
            _('Connect client drives at logon'), _('Drive to connect after login'),
            'ConnectClientDrives', FALSE
          ),
          new BooleanAttribute (
            _('Connect client printers at logon'), _('Printers to connect after login'),
            'ConnectClientPrinters', FALSE
          ),
          new BooleanAttribute (
            _('Default to main client printer'), _('Default printer for this client'),
            'DefaultPrinter', FALSE
          ),
          new SelectAttribute (
            _('Shadowing'), _('Shadowing'),
            'Shadow', TRUE,
            array(0,1,2,3,4), 0,
            array(_('disabled'), _('input on, notify on'), _('input on, notify off'),
                  _('input off, notify on'), _('input off, nofify off'))
          ),
          new SelectAttribute (
            _('On broken or timed out'), _('What happen if disconnected or timeout'),
            'BrokenConn', TRUE,
            array(0,1), 0,
            array(_('disconnect'), _('reset'))
          ),
          new SelectAttribute (
            _('Reconnect if disconnected'), _('Reconnect if disconnected'),
            'ReConn', TRUE,
            array(0,1), 0,
            array(_('from any client'), _('from previous client only'))
          ),
          new MungedAttribute (
            'sambaMungedDial',
            array(
              'ConnectClientDrives','ConnectClientPrinters','DefaultPrinter',
              'Shadow','ReConn','BrokenConn','TsLogin','InheritMode'
            )
          )
        )
      ),
      'access' => array(
        'name'  => _('Access options'),
        'icon'  => 'images/lists/key.png',
        'attrs' => array(
          new BooleanAttribute (
            _('Enforce password change'), _('Force the user to change his password'),
            'sambaPwdLastSet', FALSE, FALSE, '',
            '0', ''
          ),
          new BooleanAttribute (
            _('The password never expire'), _('The password will never expire'),
            'flag_pwdExpire', FALSE, FALSE, '',
            'X', ''
          ),
          new BooleanAttribute (
            _('Login from windows client requires no password'), _('Login from a client without a password'),
            'flag_noPwdRequired', FALSE, FALSE, '',
            'N', ''
          ),
          new BooleanAttribute (
            _('Lock samba account'), _('Lock the account'),
            'flag_lockSamba', FALSE, FALSE, '',
            '[LD]', ''
          ),
          new BooleanAttribute (
            _('Cannot change password'), _('Not allowed to change password'),
            'fd_pwdCantChange'
          ),
          new DateAttribute (
            _('Account expiration'), _('When does the account expire'),
            'sambaKickoffTime', FALSE, 'U', ''
          ),
          new SambaFlagsAttribute (
            'sambaAcctFlags',
            array('flag_pwdExpire','flag_noPwdRequired','flag_lockSamba')
          ),
          new DialogButtonAttribute (
            _('Samba logon times'), _('What is the allowed time to connect'),
            'sambaLogonHours', FALSE, 'SambaLogonHoursDialog', _('Edit settings')
          ),
          /* Unused attributes, that needs to appear in attributes array in order to be
           * removed when disabling the samba tab */
          new HiddenAttribute('sambaLogonTime'),
          new HiddenAttribute('sambaLogoffTime'),
          new HiddenAttribute('sambaPwdCanChange'),
          new HiddenAttribute('sambaPwdMustChange'),
          new HiddenAttribute('sambaPasswordHistory'),
          new HiddenAttribute('sambaBadPasswordTime'),
          new HiddenAttribute('sambaBadPasswordCount'),
        )
      ),
      'system_trust' => array(
        'name'  => _('System trust'),
        'icon'  => 'images/lists/locked.png',
        'attrs' => array(
          new CommaListAttribute(
            'sambaUserWorkstations',
            new TrustsAttribute(
              _('Allow connection from these workstations only'), _('Only allow this user to connect to this list of hosts'),
              'sambaUserWorkstations_ta', FALSE
            )
          )
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $baseobject = NULL)
  {
    // User wants me to fake the idMappings? This is useful for
    //  making winbind resolve the user names in a reasonable amount
    //  of time in combination with larger databases.
    if ($config->get_cfg_value('sambaidmapping') == 'TRUE') {
      $this->objectclasses[] = 'sambaIdmapEntry';
    }

    parent::__construct($config, $dn, $baseobject);

    $this->attributesAccess['sambaDomainName']->setChoices(array_keys($this->config->data['SERVERS']['SAMBA']));
    $this->attributesAccess['TsLogin']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'CtxWFHomeDir', 'CtxWFHomeDirDrive', 'CtxWFProfilePath',
            'InheritMode', 'CtxInitialProgram', 'CtxWorkDirectory',
            'CtxMaxConnectionTime', 'CtxMaxDisconnectionTime',
            'CtxMaxIdleTime', 'ConnectClientDrives', 'ConnectClientPrinters',
            'DefaultPrinter', 'Shadow','ReConn','BrokenConn'
          )
        )
      )
    );
    $this->attributesAccess['InheritMode']->setManagedAttributes(
      array(
        'disable' => array (
          TRUE => array (
            'CtxInitialProgram', 'CtxWorkDirectory',
          )
        )
      )
    );
    $this->attributesAccess['sambaHomeDrive']->setManagedAttributes(
      array(
        'erase' => array ('' => array ('sambaHomePath'))
      )
    );
    $this->attributesAccess['sambaPwdLastSet']->setManagedAttributes(
      array(
        'disable' => array (TRUE => array ('fd_pwdCantChange'))
      )
    );
    $this->attributesAccess['fd_pwdCantChange']->setInLdap(FALSE);
    $value = (isset($this->attrs['sambaPwdLastSet']) && ($this->attrs['sambaPwdLastSet'][0] == '4294967295'));
    $this->attributesAccess['fd_pwdCantChange']->setInitialValue($value);
    $this->attributesAccess['fd_pwdCantChange']->setValue($value);

    // Get samba domain and its sid/rid base
    if ($this->sambaSID != "") {
      $this->SID = preg_replace ("/-[^-]+$/", "", $this->sambaSID);
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search ("(&(objectClass=sambaDomain)(sambaSID=$this->SID))", array("sambaAlgorithmicRidBase","sambaDomainName"));
      if ($ldap->count() != 0) {
        $attrs = $ldap->fetch();
        if (isset($attrs['sambaAlgorithmicRidBase'])) {
          $this->ridBase = $attrs['sambaAlgorithmicRidBase'][0];
        } else {
          $this->ridBase = $this->config->get_cfg_value("sambaRidBase");
        }
        if ($this->sambaDomainName == "") {
          $this->sambaDomainName = $attrs['sambaDomainName'][0];
        }
      } else {
        // Fall back to a 'DEFAULT' domain, if none was found in LDAP.
        if ($this->sambaDomainName == "") {
          $this->sambaDomainName = "DEFAULT";
        }

        // Nothing in ldap, use configured sid and rid values.
        $this->ridBase  = $this->config->get_cfg_value("sambaRidBase");
        $this->SID      = $this->config->get_cfg_value("sambaSid");
      }
    }

    $this->attributesAccess['sambaDomainName']->setInitialValue($this->sambaDomainName);

    // Set kickOffTime to date
    if ($this->config->get_cfg_value('sambaExpirationSync') == 'posix') {
      if (isset($this->attrs['shadowExpire'][0])) {
        $this->sambaKickoffTime = date('d.m.Y', $this->attrs['shadowExpire'][0] * EpochDaysDateAttribute::$secondsPerDay);
      }
      $this->attributesAccess['sambaKickoffTime']->setDisabled(TRUE);
    }

    $this->updateAttributesValues();
    $this->prepareSavedAttributes();
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();

    /* Set a new SID */
    $this->sambaSID = "";
  }


  /* Check for input problems */
  function check()
  {
    $messages = parent::check();

    /* sambaHomePath requires sambaHomeDrive and vice versa */
    if (!empty($this->sambaHomePath) && empty($this->sambaHomeDrive)) {
        $messages[] = msgPool::required(_("Home drive"));
    }
    if (!empty($this->sambaHomeDrive) && empty($this->sambaHomePath)) {
        $messages[] = msgPool::required(_("Home path"));
    }

    /* Too many workstations? Windows usrmgr only supports eight */
    if (count($this->attributesAccess['sambaUserWorkstations']->attributes[0]->getValue()) >= 8) {
        $messages[] = _("The windows user manager allows eight clients at maximum!");
    }

    return $messages;
  }

  function prepare_save()
  {
    parent::prepare_save();
    /* Load uid and gid of this 'dn' */
    $posixAccount = $this->parent->by_object['posixAccount'];
    $uidNumber    = $posixAccount->uidNumber;
    $gidNumber    = $posixAccount->gidNumber;

    // Generate rid / primaryGroupId
    if (!isset($this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'])) {
      msg_dialog::display(_("Warning"), _("Undefined Samba SID detected. Please fix this problem manually!"), WARNING_DIALOG);
    } else {
      $this->SID      = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'];
      $this->ridBase  = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'];
    }

    // Need to generate a new uniqe uid/gid combination?
    if (($this->sambaSID == "") || $this->attributesAccess['sambaDomainName']->hasChanged()) {
      $ldap = $this->config->get_ldap_link();
      $uidNumber_tmp = $uidNumber;
      do {
        $sid = $this->SID."-".($uidNumber_tmp * 2 + $this->ridBase);
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(sambaSID=$sid)", array("sambaSID"));
        $uidNumber_tmp++;
      } while ($ldap->count() > 0);
      $this->attrs['sambaSID'] = $sid;

      // Check for users primary group
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=posixGroup)(gidNumber=".$gidNumber."))", array("cn"));
      if ($ldap->count() != 1) {
        msg_dialog::display(_("Warning"),
                  _("Cannot convert primary group to samba group: group cannot be identified!"),
                  WARNING_DIALOG);
      } else {
        $attrs = $ldap->fetch();
        $g = new group($this->config, $ldap->getDN());
        if ($g->sambaSID == "") {
          $g->sambaDomainName = $this->sambaDomainName;
          $g->smbgroup        = TRUE;
          $g->save();
        }
        $this->attrs['sambaPrimaryGroupSID'] = $g->sambaSID;
      }
    }

    /* Do not modify values if not needed */
    if (!$this->attributesAccess['sambaPwdLastSet']->hasChanged() && !$this->attributesAccess['fd_pwdCantChange']->hasChanged()) {
      unset($this->attrs['sambaPwdLastSet']);
    } elseif (!$this->sambaPwdLastSet) {
      $this->attrs['sambaPwdLastSet'] = ($this->fd_pwdCantChange?array('4294967295'):'');
    }

    // Handle "sambaKickoffTime"
    if ($this->config->get_cfg_value('sambaExpirationSync') == 'samba') {
      if ($this->sambaKickoffTime != "") {
        $this->attrs['shadowExpire'] = $this->attrs['sambaKickoffTime'] / EpochDaysDateAttribute::$secondsPerDay;
      } else {
        $this->attrs['shadowExpire'] = $this->attrs['sambaKickoffTime'];
      }
    } elseif ($this->config->get_cfg_value('sambaExpirationSync') == 'posix') {
      if ($this->parent->by_object['posixAccount']->shadowExpire != "") {
        $this->attrs['sambaKickoffTime'] = $this->parent->by_object['posixAccount']->attributesAccess['shadowExpire']->getDateValue()->format('U');
      } else {
        $this->attrs['sambaKickoffTime'] = array();
      }
    }
  }

  function adapt_from_template($dn, $skip = array())
  {
    parent::adapt_from_template($dn, $skip);

    $this->sambaSID = "";
  }
}

?>
