<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class MungedAttribute extends \FusionDirectory\Core\SimplePlugin\Attribute
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

  protected function loadAttrValue (array $attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $this->setValue($this->inputValue($attrs[$this->getLdapName()][0]));
    } else {
      $this->resetToDefault();
    }
  }

  function setParent (&$plugin)
  {
    $this->plugin = $plugin;
    if (is_object($this->plugin)) {
      foreach ($this->attrList as $key => $attr) {
        if (isset($this->plugin->attributesAccess[$attr])) {
          $this->plugin->attributesAccess[$attr]->setInLdap(FALSE);
        } else {
          unset($this->attrList[$key]);
        }
      }
      foreach ($this->flagList as $attr) {
        $this->plugin->attributesAccess[$attr]->setInLdap(FALSE);
      }
    }
  }

  function setValue ($value)
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

  function getValue ()
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

  function getAttrList ()
  {
    return array_merge($this->attrList, $this->flagList);
  }
}

class SambaFlagsAttribute extends FlagsAttribute
{
  function getValue ()
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

class WinstationsAttribute extends SystemsAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $required, array $objectTypes = ['workstation'], array $defaultValue = [], string $store_attr = 'cn', string $display_attr = 'cn', string $acl = '')
  {
    $filterElementDefinitions = [
      ['TabFilterElement',    []],
      ['FixedFilterElement',  ['(objectClass=sambaSamAccount)']],
    ];
    parent::__construct($label, $description, $ldapName, $required, $objectTypes, $defaultValue, $store_attr, $display_attr, $filterElementDefinitions, $acl);
  }
}

class sambaAccount extends simplePlugin implements UserTabLockingAction
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    global $config;

    if ($config->get_cfg_value('sambaDependsOnPosix', 'TRUE') !== 'FALSE') {
      $plDepends = ['posixAccount'];
    } else {
      $plDepends = [];
    }

    return [
      'plShortName'     => _('Samba'),
      'plDescription'   => _('Samba settings'),
      'plFilter'        => '(objectClass=sambaSamAccount)',
      'plIcon'          => 'geticon.php?context=applications&icon=samba&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=samba&size=16',
      'plSelfModify'    => TRUE,
      'plDepends'       => $plDepends,
      'plPriority'      => 5,
      'plObjectClass'   => ['sambaSamAccount'],
      'plObjectType'    => ['user'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    static $letters = [];
    if (empty($letters)) {
      $letters[] = '';
      for ($i = 68; $i < 91; $i++) {
        $letters[] = chr($i).':';
      }
    }
    static $sambaRegEx = '/^\\\\\\\\([a-z0-9%_.:$+-\\\\]+\\\\?)+$/i';
    return [
      'generic' => [
        'name'  => _('Samba profile'),
        'icon'  => 'geticon.php?context=applications&icon=samba&size=16',
        'attrs' => [
          new HiddenAttribute('sambaSID'),
          new HiddenAttribute('sambaPrimaryGroupSID'),
          new SelectAttribute(
            _('Home directory drive'), _('Letter for the home drive'),
            'sambaHomeDrive', FALSE,
            $letters
          ),
          new StringAttribute(
            _('Home directory path'), _('UNC path for the home drive'),
            'sambaHomePath', FALSE, '', '',
            $sambaRegEx
          ),
          new SelectAttribute(
            _('Domain'), _('Samba domain name'),
            'sambaDomainName', TRUE
          ),
          new StringAttribute(
            _('Script path'), _('Login script path'),
            'sambaLogonScript'
          ),
          new StringAttribute(
            _('Profile path'), _('UNC profile path'),
            'sambaProfilePath', FALSE, '', '',
            $sambaRegEx
          ),
        ]
      ],
      'terminal' => [
        'name'  => _('Terminal server'),
        'icon'  => 'geticon.php?context=devices&icon=terminal&size=16',
        'attrs' => [
          new BooleanAttribute(
            _('Allow login on terminal server'), _('Allow login on terminal server'),
            'TsLogin', FALSE
          ),
          new SelectAttribute(
            _('Home directory drive'), _('Letter for the home drive'),
            'CtxWFHomeDirDrive', FALSE,
            $letters
          ),
          new StringAttribute(
            _('Home directory path'), _('UNC path for the home drive'),
            'CtxWFHomeDir', FALSE, '', '',
            $sambaRegEx
          ),
          new StringAttribute(
            _('Profile path'), _('UNC profile path'),
            'CtxWFProfilePath', FALSE, '', '',
            $sambaRegEx
          ),
          new BooleanAttribute(
            _('Inherit client config'), _('Inherit client configuration'),
            'InheritMode', FALSE
          ),
          new StringAttribute(
            _('Initial progam'), _('Program to start after connecting'),
            'CtxInitialProgram'
          ),
          new StringAttribute(
            _('Working directory'), _('Basic working directory'),
            'CtxWorkDirectory'
          ),
          new IntAttribute(
            _('Connection timeout'), _('Timeout when connecting to terminal server'),
            'CtxMaxConnectionTime', FALSE,
            0, FALSE
          ),
          new IntAttribute(
            _('Disconnection timeout'), _('Timeout before disconnecting from terminal server'),
            'CtxMaxDisconnectionTime', FALSE,
            0, FALSE
          ),
          new IntAttribute(
            _('Idle timeout'), _('Idle timeout before disconnecting from terminal server'),
            'CtxMaxIdleTime', FALSE,
            0, FALSE
          ),
          new BooleanAttribute(
            _('Connect client drives at logon'), _('Drive to connect after login'),
            'ConnectClientDrives', FALSE
          ),
          new BooleanAttribute(
            _('Connect client printers at logon'), _('Printers to connect after login'),
            'ConnectClientPrinters', FALSE
          ),
          new BooleanAttribute(
            _('Default to main client printer'), _('Default printer for this client'),
            'DefaultPrinter', FALSE
          ),
          new SelectAttribute(
            _('Shadowing'), _('Shadowing'),
            'Shadow', TRUE,
            [0,1,2,3,4], 0,
            [_('disabled'), _('input on, notify on'), _('input on, notify off'),
                  _('input off, notify on'), _('input off, nofify off')]
          ),
          new SelectAttribute(
            _('On broken or timed out'), _('What happen if disconnected or timeout'),
            'BrokenConn', TRUE,
            [0,1], 0,
            [_('disconnect'), _('reset')]
          ),
          new SelectAttribute(
            _('Reconnect if disconnected'), _('Reconnect if disconnected'),
            'ReConn', TRUE,
            [0,1], 0,
            [_('from any client'), _('from previous client only')]
          ),
          new MungedAttribute(
            'sambaMungedDial',
            [
              'ConnectClientDrives','ConnectClientPrinters','DefaultPrinter',
              'Shadow','ReConn','BrokenConn','TsLogin','InheritMode'
            ]
          )
        ]
      ],
      'access' => [
        'name'  => _('Access options'),
        'icon'  => 'geticon.php?context=status&icon=dialog-password&size=16',
        'attrs' => [
          new BooleanAttribute(
            _('Enforce password change'), _('Force the user to change his password'),
            'sambaPwdLastSet', FALSE, FALSE, '',
            '0', ''
          ),
          new BooleanAttribute(
            _('The password never expire'), _('The password will never expire'),
            'flag_pwdExpire', FALSE, FALSE, '',
            'X', ''
          ),
          new BooleanAttribute(
            _('Login from windows client requires no password'), _('Login from a client without a password'),
            'flag_noPwdRequired', FALSE, FALSE, '',
            'N', ''
          ),
          new BooleanAttribute(
            _('Lock samba account'), _('Lock the account'),
            'flag_lockSamba', FALSE, FALSE, '',
            '[LD]', ''
          ),
          new BooleanAttribute(
            _('Cannot change password'), _('Not allowed to change password'),
            'fd_pwdCantChange'
          ),
          new DateAttribute(
            _('Account expiration'), _('When does the account expire'),
            'sambaKickoffTime', FALSE, 'U', ''
          ),
          new SambaFlagsAttribute(
            'sambaAcctFlags',
            ['flag_pwdExpire','flag_noPwdRequired','flag_lockSamba']
          ),
          new DialogButtonAttribute(
            _('Samba logon times'), _('What is the allowed time to connect'),
            'sambaLogonHours', _('Edit settings'), 'SambaLogonHoursDialog'
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
          new HiddenAttribute('sambaNTPassword'),
          new HiddenAttribute('sambaLMPassword'),
        ]
      ],
      'system_trust' => [
        'name'  => _('System trust'),
        'icon'  => 'geticon.php?context=categories&icon=acl&size=16',
        'attrs' => [
          new CommaListAttribute(
            'sambaUserWorkstations',
            new WinstationsAttribute(
              _('Allow connection from these workstations only'), _('Only allow this user to connect to this list of hosts'),
              'sambaUserWorkstations_ta', FALSE
            )
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $baseobject = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $baseobject, $parent, $mainTab);

    // User wants me to fake the idMappings? This is useful for
    //  making winbind resolve the user names in a reasonable amount
    //  of time in combination with larger databases.
    if ($config->get_cfg_value('sambaidmapping') == 'TRUE') {
      $this->objectclasses[] = 'sambaIdmapEntry';
    }


    $domains = sambaDomainManagement::getSambaDomains();
    $this->attributesAccess['sambaDomainName']->setChoices(array_keys($domains));
    $this->attributesAccess['TsLogin']->setManagedAttributes(
      [
        'disable' => [
          FALSE => [
            'CtxWFHomeDir', 'CtxWFHomeDirDrive', 'CtxWFProfilePath',
            'InheritMode', 'CtxInitialProgram', 'CtxWorkDirectory',
            'CtxMaxConnectionTime', 'CtxMaxDisconnectionTime',
            'CtxMaxIdleTime', 'ConnectClientDrives', 'ConnectClientPrinters',
            'DefaultPrinter', 'Shadow','ReConn','BrokenConn'
          ]
        ]
      ]
    );
    $this->attributesAccess['InheritMode']->setManagedAttributes(
      [
        'disable' => [
          TRUE => [
            'CtxInitialProgram', 'CtxWorkDirectory',
          ]
        ]
      ]
    );
    $this->attributesAccess['sambaHomeDrive']->setManagedAttributes(
      [
        'erase' => ['' => ['sambaHomePath']]
      ]
    );
    $this->attributesAccess['sambaPwdLastSet']->setManagedAttributes(
      [
        'disable' => [TRUE => ['fd_pwdCantChange']]
      ]
    );
    $this->attributesAccess['fd_pwdCantChange']->setInLdap(FALSE);
    $value = (isset($this->attrs['sambaPwdLastSet']) && ($this->attrs['sambaPwdLastSet'][0] == '4294967295'));
    $this->attributesAccess['fd_pwdCantChange']->setInitialValue($value);
    $this->attributesAccess['fd_pwdCantChange']->setValue($value);

    /* Get samba domain */
    if ($this->sambaSID != '') {
      $SID = preg_replace('/-[^-]+$/', '', $this->sambaSID);
      foreach ($domains as $name => $domain) {
        if ($domain['SID'] == $SID) {
          $this->sambaDomainName = $name;
          break;
        }
      }
    }

    $this->attributesAccess['sambaDomainName']->setInitialValue($this->sambaDomainName);

    // Set kickOffTime to date
    if ($config->get_cfg_value('sambaExpirationSync') == 'posix') {
      if (isset($this->attrs['shadowExpire'][0])) {
        $this->sambaKickoffTime = date('d.m.Y', $this->attrs['shadowExpire'][0] * EpochDaysDateAttribute::$secondsPerDay);
      }
      $this->attributesAccess['sambaKickoffTime']->setDisabled(TRUE);
    }
    $this->prepareSavedAttributes();
  }

  function resetCopyInfos ()
  {
    parent::resetCopyInfos();

    /* Set a new SID */
    $this->sambaSID = '';
  }

  /* Check for input problems */
  function check (): array
  {
    $errors = parent::check();

    /* sambaHomePath requires sambaHomeDrive and vice versa */
    if (!empty($this->sambaHomePath) && empty($this->sambaHomeDrive)) {
      $errors[] = new SimplePluginCheckError(
        $this->attributesAccess['sambaHomeDrive'],
        htmlescape(sprintf(
          _('Field "%s" is required when "%s" is filled! Fill both field or empty them.'),
          $this->attributesAccess['sambaHomeDrive']->getLabel(),
          $this->attributesAccess['sambaHomePath']->getLabel()
        ))
      );
    }
    if (!empty($this->sambaHomeDrive) && empty($this->sambaHomePath)) {
      $errors[] = new SimplePluginCheckError(
        $this->attributesAccess['sambaHomePath'],
        htmlescape(sprintf(
          _('Field "%s" is required when "%s" is filled! Fill both field or empty them.'),
          $this->attributesAccess['sambaHomePath']->getLabel(),
          $this->attributesAccess['sambaHomeDrive']->getLabel()
        ))
      );
    }

    /* Too many workstations? Windows usrmgr only supports eight */
    if (count($this->attributesAccess['sambaUserWorkstations']->attributes[0]->getValue()) >= 8) {
      $errors[] = new SimplePluginCheckError(
        $this->attributesAccess['sambaUserWorkstations'],
        htmlescape(_('The windows user manager allows eight clients at maximum!'))
      );
    }

    return $errors;
  }

  function prepare_save (): array
  {
    global $config;

    $errors = parent::prepare_save();
    if (!empty($errors)) {
      return $errors;
    }

    if ($this->is_template) {
      /* Skip synchronisations and generations on templates */
      return $errors;
    }

    // Generate rid / primaryGroupId
    $domains = sambaDomainManagement::getSambaDomains();
    if (!isset($domains[$this->sambaDomainName]['SID'])) {
      $error = new SimplePluginError($this, htmlescape(_('Undefined Samba SID detected. Please fix this problem manually!')));
      $error->display();
    } else {
      // Need to generate a new unique uid/gid combination?
      if (($this->sambaSID == '') || $this->attributesAccess['sambaDomainName']->hasChanged()) {
        $SIDBase  = $domains[$this->sambaDomainName]['SID'];
        $ridBase  = $domains[$this->sambaDomainName]['RIDBASE'];
        $ldap = $config->get_ldap_link();
        $ldap->cd($config->current['BASE']);
        if ($config->get_cfg_value('sambaDependsOnPosix', 'TRUE') !== 'FALSE') {
          $tmpNumber = $this->parent->by_object['posixAccount']->uidNumber;
        } else {
          $tmpNumber = 1;
        }
        do {
          $sambaSID = $SIDBase.'-'.($tmpNumber * 2 + $ridBase);
          $ldap->search('(sambaSID='.$sambaSID.')', ['sambaSID']);
          $tmpNumber++;
        } while ($ldap->count() > 0);
        $this->attrs['sambaSID'] = $sambaSID;
      }
    }

    if (isset($this->parent->by_object['posixAccount']) && $this->parent->by_object['posixAccount']->is_account) {
      $ldap = $config->get_ldap_link();
      // Check for users primary group
      $ldap->cd($config->current['BASE']);
      $ldap->search('(&(objectClass=posixGroup)(gidNumber='.$this->parent->by_object['posixAccount']->gidNumber.'))', ['cn']);
      if ($ldap->count() != 1) {
        $error = new SimplePluginError(
          $this,
          htmlescape(_('Cannot convert primary group to samba group: group cannot be identified!'))
        );
        $error->display();
      } else {
        $attrs = $ldap->fetch();
        try {
          $tabGroup = objects::open($attrs['dn'], 'group');
        } catch (NonExistingObjectTypeException $e) {
          $tabGroup = objects::open($attrs['dn'], 'ogroup');
        }
        if ($tabGroup->by_object['sambaGroup']->is_account) {
          if ($tabGroup->by_object['sambaGroup']->sambaSID == "") {
            $tabGroup->by_object['sambaGroup']->sambaDomainName = $this->sambaDomainName;
            $tabGroup->by_object['sambaGroup']->smbgroup        = TRUE;
            $tabGroup->save();
          }
          $this->attrs['sambaPrimaryGroupSID'] = $tabGroup->by_object['sambaGroup']->sambaSID;
        } else {
          if ($config->get_cfg_value('sambaPrimaryGroupWarning') == 'TRUE') {
            $warning = new FusionDirectoryWarning(
              htmlescape(_('Cannot convert primary group to samba group: group samba tab is disabled!'))
            );
            $warning->display();
          }
        }
      }
    }

    /* Do not modify values if not needed */
    if (!$this->attributesAccess['sambaPwdLastSet']->hasChanged() && !$this->attributesAccess['fd_pwdCantChange']->hasChanged()) {
      unset($this->attrs['sambaPwdLastSet']);
    } elseif (!$this->sambaPwdLastSet) {
      $this->attrs['sambaPwdLastSet'] = ($this->fd_pwdCantChange ? ['4294967295'] : '');
    }

    if ($this->parent->getBaseObject()->attributesAccess['userPassword']->hasChanged()) {
      $password = $this->parent->getBaseObject()->attributesAccess['userPassword']->getClear();
      if ($password !== '' ) {
        $smbHash = new smbHash();
        if ($config->get_cfg_value('sambaGenLMPassword', 'FALSE') == 'TRUE') {
          $this->attrs['sambaLMPassword'] = $smbHash->lmhash($password);
        } else {
          $this->attrs['sambaLMPassword'] = [];
        }
        $this->attrs['sambaNTPassword'] = $smbHash->nthash($password);
        $this->attrs['sambaPwdLastSet'] = date('U');

        $this->attrs['sambaBadPasswordCount'] = '0';
        $this->attrs['sambaBadPasswordTime']  = '0';
      }
    }


    // Handle "sambaKickoffTime"
    if ($config->get_cfg_value('sambaExpirationSync') == 'samba') {
      if ($this->sambaKickoffTime != "") {
        $this->attrs['shadowExpire'] = $this->attrs['sambaKickoffTime'] / EpochDaysDateAttribute::$secondsPerDay;
      } else {
        $this->attrs['shadowExpire'] = $this->attrs['sambaKickoffTime'];
      }
    } elseif ($config->get_cfg_value('sambaExpirationSync') == 'posix') {
      if ($this->parent->by_object['posixAccount']->shadowExpire != "") {
        try {
          $this->attrs['sambaKickoffTime'] = $this->parent->by_object['posixAccount']->attributesAccess['shadowExpire']->getDateValue()->format('U');
        } catch (Exception $e) {
          // shadowExpire is an invalid date, most likely because we’re a template and it uses patterns
          $this->attrs['sambaKickoffTime'] = [];
        }
      } else {
        $this->attrs['sambaKickoffTime'] = [];
      }
    }

    return $errors;
  }

  function adapt_from_template (array $attrs, array $skip = [])
  {
    parent::adapt_from_template($attrs, $skip);

    $this->sambaSID = "";
  }

  /* Returns TRUE if this attribute should be asked in the creation by template dialog */
  function showInTemplate (string $attr, array $templateAttrs): bool
  {
    if (in_array($attr, $this->attributesAccess['sambaMungedDial']->getAttrList())) {
      return FALSE;
    }
    return parent::showInTemplate($attr, $templateAttrs);
  }

  public function fillLockingLDAPAttrs (string $mode, array &$modify)
  {
    global $config;

    if (!isset($this->attrs['sambaNTPassword'][0])) {
      return;
    }

    $modify['sambaNTPassword'] = $this->attrs['sambaNTPassword'][0];
    if ($config->get_cfg_value('sambaGenLMPassword', 'FALSE') == 'TRUE') {
      $modify['sambaLMPassword'] = $this->attrs['sambaLMPassword'][0];
    } else {
      $modify['sambaLMPassword'] = [];
    }
    foreach (['sambaNTPassword', 'sambaLMPassword'] as $attr) {
      if (is_array($modify[$attr])) {
        continue;
      }
      if ($mode == 'LOCK') {
        /* Lock entry */
        if (!preg_match('/^\!/', $modify[$attr])) {
          $modify[$attr] = '!'.$modify[$attr];
        }
      } else {
        /* Unlock entry */
        $modify[$attr] = preg_replace('/^\!/', '', $modify[$attr]);
      }
    }
  }
}
