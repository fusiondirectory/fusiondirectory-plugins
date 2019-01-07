<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

/*!
  \brief   posixAccount plugin
  \author  Cajus Pollmeier <pollmeier@gonicus.de>
  \version 2.00
  \date    24.07.2003

  This class provides the functionality to read and write all attributes
  relevant for posixAccounts and shadowAccounts from/to the LDAP. It
  does syntax checking and displays the formulars required.
 */

class EpochDaysDateAttribute extends DateAttribute
{
  /* 24 * 60 * 60 = 86400 */
  public static $secondsPerDay = 86400;

  function __construct ($label, $description, $ldapName, $required, $defaultValue = 'now', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, '', $defaultValue, $acl);
  }

  protected function ldapToDate($ldapValue)
  {
    $date = DateTime::createFromFormat('U', $ldapValue * static::$secondsPerDay, timezone::utc());
    if ($date !== FALSE) {
      return $date;
    } else {
      trigger_error('LDAP value for '.$this->getLdapName().' was not in the right date format.');
      return new DateTime($ldapValue, timezone::utc());
    }
  }

  protected function dateToLdap($dateValue)
  {
    return floor($dateValue->format('U') / static::$secondsPerDay);
  }

  function getEpochDays()
  {
    if (empty($this->value)) {
      return 0;
    } else {
      try {
        return $this->dateToLdap($this->getDateValue());
      } catch (Exception $e) {
        if (is_object($this->plugin) && $this->plugin->is_template) {
          return $this->value;
        } else {
          throw $e;
        }
      }
    }
  }
}

class posixAccount extends simplePlugin
{
  var $displayHeader = TRUE;
  var $objectclasses = array('posixAccount', 'shadowAccount');
  protected $locks = array();

  protected $fromTemplate = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Unix'),
      'plDescription' => _('Edit users POSIX settings'),
      'plIcon'        => 'geticon.php?context=applications&icon=os-linux&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=os-linux&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 2,
      'plObjectType'  => array('user'),
      'plForeignKeys' => array(
        'gidNumber' => array(
          array('posixGroup','gidNumber'),
          array('mixedGroup','gidNumber'),
        ),
        'host'      => array(
          array('serverGeneric', 'cn'),
          array('workstationGeneric', 'cn'),
          array('terminalGeneric', 'cn'),
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    global $config;
    return array(
      'main' => array(
        'name'  => _('Unix'),
        'icon'  => 'geticon.php?context=applications&icon=os-linux&size=16',
        'attrs' => array(
          new PathAttribute(
            _('Home directory'), _('The path to the home directory of this user'),
            'homeDirectory', TRUE
          ),
          new StringAttribute('gecos', 'gecos', 'gecos'),
          new SelectAttribute(
            _('Shell'), _('Which shell should be used when this user log in'),
            'loginShell', TRUE,
            $config->get_cfg_value('Shells', array(_('unconfigured'))),
            $config->get_cfg_value('DefaultShell', '')
          ),
          new SelectAttribute(
            _('Primary group'), _('Primary group for this user'),
            'primaryGroup', FALSE
          ),
          new DisplayAttribute(
            _('Status'), _('Status of this user unix account'),
            'posixStatus', FALSE
          ),
          new BooleanAttribute(
            _('Force user/group id'), _('Force user id and group id values for this user'),
            'force_ids', FALSE
          ),
          new IntAttribute(
            _('User id'), _('User id value for this user'),
            'uidNumber', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            _('Group id'), _('Group id value for this user'),
            'gidNumber', FALSE,
            0, FALSE, ''
          )
        )
      ),
      'groups' => array(
        'name'  => _('Group membership'),
        'icon'  => 'geticon.php?context=types&icon=user-group&size=16',
        'attrs' => array(
          new GroupsAttribute('', _('Group membership'), 'groupMembership')
        )
      ),
      'account' => array(
        'name'  => _('Account'),
        'icon'  => 'geticon.php?context=devices&icon=terminal&size=16',
        'attrs' => array(
          new BooleanAttribute(
            _('User must change password on first login'), _('User must change password on first login (needs a value for Delay before forcing password change)'),
            'mustchangepassword', FALSE
          ),
          new IntAttribute(
            _('Minimum delay between password changes (days)'), _('The user won\'t be able to change his password before this number of days (leave empty to disable)'),
            'shadowMin', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            _('Delay before forcing password change (days)'), _('The user will be forced to change his password after this number of days (leave empty to disable)'),
            'shadowMax', FALSE,
            0, FALSE, ''
          ),
          new EpochDaysDateAttribute(
            _('Password expiration date'), _('Date after which this user password will expire (leave empty to disable)'),
            'shadowExpire', FALSE,
            ''
          ),
          new IntAttribute(
            _('Delay of inactivity before disabling user (days)'), _('Maximum delay of inactivity after password expiration before the user is disabled (leave empty to disable)'),
            'shadowInactive', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            _('Delay for user warning before password expiry (days)'), _('The user will be warned this number of days before his password expiration (leave empty to disable)'),
            'shadowWarning', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            'No label', 'No description',
            'shadowLastChange', FALSE,
            0, FALSE, ''
          ),
        )
      ),
      'system_trust' => array(
        'name'  => _('System trust'),
        'icon'  => 'geticon.php?context=categories&icon=acl&size=16',
        'attrs' => array(
          new SelectAttribute(
            _('Trust mode'), _('Type of authorization for those hosts'),
            'trustMode', FALSE,
            array('', 'fullaccess', 'byhost'),
            '',
            array(_('disabled'), _('full access'), _('allow access to these hosts'))
          ),
          new SystemsAttribute(
            '', _('Only allow this user to connect to this list of hosts'),
            'host', FALSE
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['gecos']->setVisible(FALSE);

    $this->attributesAccess['trustMode']->setInLdap(FALSE);
    if (!class_available('systemManagement')) {
      $this->attributesAccess['trustMode']->setChoices(
            array('', 'fullaccess'),
            array(_('disabled'), _('full access'))
      );
    }
    $this->attributesAccess['trustMode']->setManagedAttributes(
      array(
        'multiplevalues' => array('notbyhost' => array('','fullaccess')),
        'erase' => array(
          'notbyhost' => array('host')
        )
      )
    );
    if ((count($this->host) == 1) && ($this->host[0] == '*')) {
      $this->trustMode = 'fullaccess';
    } elseif (count($this->host) > 0) {
      $this->trustMode = 'byhost';
    }

    $this->attributesAccess['uidNumber']->setUnique(TRUE);
    $this->attributesAccess['force_ids']->setInLdap(FALSE);
    $this->attributesAccess['force_ids']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'uidNumber',
            'gidNumber',
          )
        )
      )
    );
    $this->attributesAccess['primaryGroup']->setInLdap(FALSE);

    $this->attributesAccess['mustchangepassword']->setInLdap(FALSE);
    $this->attributesAccess['shadowLastChange']->setVisible(FALSE);
    $this->attributesAccess['shadowMax']->setManagedAttributes(
      array(
        'disable' => array (
          '' => array (
            'mustchangepassword',
          )
        )
      )
    );

    if ($dn !== NULL) {
      /* Fill group */
      $this->primaryGroup = $this->gidNumber;
    }

    /* Generate shell list from config */
    $loginShellList = $this->attributesAccess['loginShell']->getChoices();

    /* Insert possibly missing loginShell */
    $loginShell = $this->attributesAccess['loginShell']->getValue();
    if (($loginShell != '') && !in_array($loginShell, $loginShellList)) {
      $loginShellList[] = $loginShell;
    }
    $this->attributesAccess['loginShell']->setChoices($loginShellList);

    $secondaryGroups = array();
    $secondaryGroups[''] = '- '._('automatic').' -';
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=posixGroup)', array('cn', 'gidNumber'));
    while ($attrs = $ldap->fetch()) {
      $secondaryGroups[$attrs['gidNumber'][0]] = $attrs['cn'][0];
    }
    asort($secondaryGroups);
    $this->attributesAccess['primaryGroup']->setChoices(array_keys($secondaryGroups), array_values($secondaryGroups));

    $this->attributesAccess['groupMembership']->setInLdap(FALSE);
    if (class_available('mixedGroup')) {
      $this->attributesAccess['groupMembership']->setDisabled(TRUE);
      $this->attributesAccess['groupMembership']->setVisible(FALSE);
    } else {
      if ($this->is_template) {
        if (isset($this->attrs['posixGroups'])) {
          unset($this->attrs['posixGroups']['count']);
          $this->groupMembership = $this->attrs['posixGroups'];
        }
      } else {
        /* Groups handling */
        $ldap->cd($config->current['BASE']);
        $ldap->search('(&(objectClass=posixGroup)(memberUid='.ldap_escape_f($this->getUid()).'))', array('cn', 'description'));
        $groupMembership = array();
        while ($attrs = $ldap->fetch()) {
          $entry = $attrs['cn'][0];
          if (isset($attrs['description'][0])) {
            $entry .= ' ['.$attrs['description'][0].']';
          }
          $groupMembership[$attrs['dn']] = $entry;
        }
        asort($groupMembership);
        reset($groupMembership);
        $this->attributesAccess['groupMembership']->setValue(array_keys($groupMembership));
        $this->attributesAccess['groupMembership']->setDisplayValues(array_values($groupMembership));
        $this->savedGroupMembership = array_keys($groupMembership);
      }
    }
    if ($this->is_template) {
      /* Template specific handling */
      if (isset($this->attrs['force_ids'])) {
        $this->force_ids = ($this->attrs['force_ids'][0] != 'FALSE');
      }
      if (!$this->force_ids) {
        $this->uidNumber = '';
        $this->gidNumber = '';
      }
      if (isset($this->attrs['mustchangepassword'])) {
        $this->mustchangepassword = ($this->attrs['mustchangepassword'][0] != 'FALSE');
      }
    } else {
      /* Status is not computed for templates */
      $current = floor(date('U') / EpochDaysDateAttribute::$secondsPerDay);

      $shadowExpire     = $this->attributesAccess['shadowExpire']->getEpochDays();
      $shadowInactive   = $this->attributesAccess['shadowInactive']->getValue();
      $shadowMin        = $this->attributesAccess['shadowMin']->getValue();
      $shadowMax        = $this->attributesAccess['shadowMax']->getValue();
      $shadowLastChange = $this->attributesAccess['shadowLastChange']->getValue();
      if (($current >= $shadowExpire) && ($shadowExpire > 0)) {
        $status = _('expired');
        if ($shadowInactive != '' && ($current - $shadowExpire) < $shadowInactive) {
          $status .= ', '._('grace time active');
        }
      } elseif ($shadowMax != '' && ($shadowLastChange + $shadowMax) <= $current) {
        $status = _('active').', '._('password expired');
      } elseif ($shadowMin != '' && ($shadowLastChange + $shadowMin) <= $current) {
        $status = _('active').', '._('password not changeable');
      } else {
        $status = _('active');
      }
      $this->attributesAccess['posixStatus']->setValue($status);
    }
  }

  function is_this_account($attrs)
  {
    /* shadowAccount is not required */
    return (isset($attrs['objectClass']) && in_array_ics('posixAccount', $attrs['objectClass']));
  }

  function getUid()
  {
    if (isset($this->parent)) {
      $baseobject = $this->parent->getBaseObject();
      return $baseobject->uid;
    }
    if (isset($this->attrs['uid'][0])) {
      return $this->attrs['uid'][0];
    }
  }

  function resetCopyInfos()
  {
    global $config;
    parent::resetCopyInfos();

    $this->savedGroupMembership = array();

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=posixGroup)(gidNumber='.ldap_escape_f($this->gidNumber).')(cn='.ldap_escape_f($this->getUid()).'))', array('cn','gidNumber'));

    if ($ldap->count() > 0) {
      /* The copied user had its own group: switch back to automatic */
      $this->primaryGroup = 0;
    }

    $this->force_ids = FALSE;
    $this->attributesAccess['uidNumber']->setInitialValue('');
    $this->attributesAccess['gidNumber']->setInitialValue('');
    $this->uidNumber = '';
    $this->gidNumber = '';
  }

  function check()
  {
    global $config;
    if (isset($this->parent) &&
        isset($this->parent->getBaseObject()->is_template) &&
        $this->parent->getBaseObject()->is_template) {
      $message = array();
    } else {
      $message = parent::check();
    }

    /* Check ID's if they are forced by user */
    if ($this->force_ids) {
      if ($this->uidNumber < $config->get_cfg_value('minId')) {
        $message[] = msgPool::toosmall(_('UID'), $config->get_cfg_value('minId'));
      }
      if ($this->gidNumber < $config->get_cfg_value('minId')) {
        $message[] = msgPool::toosmall(_('GID'), $config->get_cfg_value('minId'));
      }
    }

    /* Check shadow settings */
    if ($this->shadowWarning !== '') {
      if ($this->shadowMax === '') {
        $message[] = msgPool::depends('shadowWarning', 'shadowMax');
      }
      if ($this->shadowWarning > $this->shadowMax) {
        $message[] = msgPool::toobig('shadowWarning', 'shadowMax');
      }
      if (($this->shadowMin !== '') && ($this->shadowWarning < $this->shadowMin)) {
        $message[] = msgPool::toosmall('shadowWarning', 'shadowMin');
      }
    }

    if (($this->shadowInactive !== '') && ($this->shadowMax === '')) {
      $message[] = msgPool::depends('shadowInactive', 'shadowMax');
    }
    if (($this->shadowMin !== '') && ($this->shadowMax !== '') &&
        ($this->shadowMin > $this->shadowMax)) {
      $message[] = msgPool::toobig('shadowMin', 'shadowMax');
    }

    return $message;
  }

  protected function prepare_save()
  {
    global $config;

    /* Fill gecos */
    if (isset($this->parent) && $this->parent !== NULL) {
      $this->gecos = rewrite($this->parent->getBaseObject()->cn);
      if (!preg_match('/^[a-z0-9 -]+$/i', $this->gecos)) {
        $this->gecos = '';
      }
    }

    if (!$this->force_ids) {
      /* Handle uidNumber.
       * - use existing number if possible
       * - if not, try to create a new uniqe one.
       * */
      if ($this->is_template) {
        $this->uidNumber = '';
      } else {
        if ($this->attributesAccess['uidNumber']->getInitialValue() != '') {
          $this->uidNumber = $this->attributesAccess['uidNumber']->getInitialValue();
        } else {
          $this->uidNumber = static::getNextIdLock('uidNumber', $this->dn);
          $this->locks[] = 'uidNumber';
        }
      }
    }

    /* Handle gidNumber
     * - If we do not have a primary group selected (automatic), we will check if there
     *    is already a group  with the same name and use this as primary.
     * - .. if we couldn't find a group with the same name, we will create a new one,
     *    using the users uid as cn and a generated uniqe gidNumber.
     * */
    if ($this->is_template && $this->force_ids) {
      /* Nothing to do in this case */
    } elseif ($this->is_template && ($this->primaryGroup == 0)) {
      $this->gidNumber = '';
    } elseif (($this->primaryGroup == 0) || $this->force_ids) {
      /* Search for existing group */
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);

      /* Are we forced to use a special gidNumber? */
      if ($this->force_ids) {
        $ldap->search('(&(objectClass=posixGroup)(gidNumber='.ldap_escape_f($this->gidNumber).'))',      array('cn','gidNumber'));
      } else {
        $ldap->search('(&(objectClass=posixGroup)(gidNumber=*)(cn='.ldap_escape_f($this->getUid()).'))', array('cn','gidNumber'));
      }

      /* No primary group found, create a new one */
      if ($ldap->count() == 0) {
        $groupcn  = $this->getUid();

        /* Request a new and unique gidNumber, if required */
        if (!$this->force_ids) {
          $this->gidNumber = static::getNextIdLock('gidNumber', $this->dn);
          $this->locks[] = 'gidNumber';
        }

        /* If forced gidNumber could not be found, then check if the given group name already exists.
         */
        $cnt = 0;
        $ldap->search('(&(objectClass=posixGroup)(cn='.ldap_escape_f($groupcn).'))', array('cn'));
        while ($ldap->count() && ($cnt < 100)) {
          $cnt++;
          $groupcn = $this->getUid().'_'.$cnt;
          $ldap->search('(&(objectClass=posixGroup)(cn='.ldap_escape_f($groupcn).'))', array('cn'));
        }

        /* Create new primary group and enforce the new gidNumber */
        if (class_available('mixedGroup')) {
          $tabObject = objects::create('ogroup');
        } else {
          $tabObject  = objects::create('group');
        }
        if ($this->fromTemplate) {
          /* When creating a user through a template we bypass ACLs */
          $tabObject->setIgnoreAcls(TRUE);
        }

        $baseObject = $tabObject->getBaseObject();

        $baseObject->cn           = $groupcn;
        $baseObject->description  = sprintf(_('Group of user %s'), $this->getUid());
        $baseObject->base         = $this->parent->getBaseObject()->base;
        if (class_available('mixedGroup')) {
          // fake attrs as this user may not exists yet
          $attrs = array(
            'objectClass' => array('inetOrgPerson','organizationalPerson','person','posixAccount','shadowAccount'),
            'cn' => $this->getUid(),
            'uid' => $this->getUid(),
          );
          $baseObject->attributesAccess['member']->addValue($this->dn, $attrs);
          $tabObject->gen_tabs();
          $posixTab = $tabObject->by_object['mixedGroup'];
        } else {
          $posixTab = $baseObject;
        }
        $posixTab->force_id   = 1;
        $posixTab->gidNumber  = $this->gidNumber;

        $errors = $tabObject->save();
        if (!empty($errors)) {
          array_unshift(
            $errors,
            sprintf(_('Could not create automatic primary group (using gidNumber "%s"), because of the following errors'), $this->gidNumber)
          );
          return $errors;
        } else {
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
            sprintf('Primary group "%s" created, using gidNumber "%s".', $tabObject->dn, $this->gidNumber), '');
        }
      } else {
        $attrs = $ldap->fetch();
        $this->gidNumber = $attrs['gidNumber'][0];
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
          'Found and used: <i>'.$attrs['dn'].'</i>',
          sprintf('Primary group "%s" exists, gidNumber is "%s".', $this->getUid(), $this->gidNumber));
      }
    } else {
      /* Primary group was selected by user */
      $this->gidNumber = $this->primaryGroup;
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
        sprintf('Primary group "%s" for user "%s" manually selected.',
        $this->gidNumber, $this->getUid()), '');
    }

    if (!$this->is_template) {
      if ($this->mustchangepassword) {
        $this->shadowLastChange =
          floor(date('U') / EpochDaysDateAttribute::$secondsPerDay) - $this->shadowMax - 1;
      } elseif (
          ($this->is_account && !$this->initially_was_account) ||
          $this->parent->getBaseObject()->attributesAccess['userPassword']->hasChanged()
        ) {
        $this->shadowLastChange = floor(date('U') / EpochDaysDateAttribute::$secondsPerDay);
      }
    }

    $errors = parent::prepare_save();
    if (!empty($errors)) {
      return $errors;
    }

    if ($this->trustMode == 'fullaccess') {
      $this->attrs['host'] = array('*');
    }

    /* Trust accounts */
    if (($this->trustMode != '') && !in_array('hostObject', $this->attrs['objectClass'])) {
      $this->attrs['objectClass'][] = 'hostObject';
    } elseif (($this->trustMode == '') && (($key = array_search('hostObject', $this->attrs['objectClass'])) !== FALSE)) {
      unset($this->attrs['objectClass'][$key]);
    }

    if ($this->is_template) {
      $this->attrs['posixGroups'] = $this->groupMembership;
      if ($this->force_ids) {
        if (($this->uidNumber == '%askme%') || ($this->gidNumber == '%askme%')) {
          $this->attrs['force_ids'] = '%askme%';
        } else {
          $this->attrs['force_ids'] = 'TRUE';
        }
      } else {
        $this->attrs['force_ids']   = 'FALSE';
      }
      $this->attrs['mustchangepassword'] = ($this->mustchangepassword ? 'TRUE' : 'FALSE');
    }

    return $errors;
  }

  protected function shouldSave()
  {
    if (parent::shouldSave()) {
      return TRUE;
    }
    if (array_differs($this->groupMembership, $this->savedGroupMembership)) {
      return TRUE;
    }
    return FALSE;
  }

  function save()
  {
    $errors = parent::save();

    foreach ($this->locks as $lock) {
      del_lock($lock);
    }
    $this->locks = array();

    return $errors;
  }

  protected function ldap_save()
  {
    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    if (!$this->is_template && !class_available('mixedGroup')) {
      /* Take care about groupMembership values: add to groups */
      $groupMembership = $this->attributesAccess['groupMembership']->getValue();
      foreach ($groupMembership as $value) {
        if (!in_array($value, $this->savedGroupMembership)) {
          try {
            $g = objects::open($value, 'group');
            $g->getBaseObject()->addUser($this->dn, $this->getUid());
            $msg = $g->save();
            if (empty($msg)) {
              $this->savedGroupMembership[] = $value;
            } else {
              $errors = array_merge($errors, $msg);
            }
          } catch (FusionDirectoryException $e) {
            $errors[] = 'Exception: '.$e->getMessage();
          }
        }
      }

      /* Remove groups not listed in groupMembership */
      foreach ($this->savedGroupMembership as $key => $value) {
        if (!in_array($value, $groupMembership)) {
          try {
            $g = objects::open($value, 'group');
            $g->getBaseObject()->removeUser($this->getUid());
            $msg = $g->save();
            if (empty($msg)) {
              unset($this->savedGroupMembership[$key]);
            } else {
              $errors = array_merge($errors, $msg);
            }
          } catch (FusionDirectoryException $e) {
            $errors[] = 'Exception: '.$e->getMessage();
          }
        }
      }
    }

    return $errors;
  }

  protected function ldap_remove()
  {
    /* Remove and write to LDAP */
    $errors = parent::ldap_remove();
    if (!empty($errors)) {
      return $errors;
    }

    if (!$this->is_template) {
      /* Delete group only if cn is uid and there are no other members inside */
      if (class_available('mixedGroup')) {
        $type = 'ogroup';
      } else {
        $type = 'group';
      }
      $groups = objects::ls($type, array('cn' => 1, 'memberUid' => '*'), NULL, '(&(objectClass=posixGroup)(gidNumber='.ldap_escape_f($this->gidNumber).')(cn='.ldap_escape_f($this->getUid()).'))');
      foreach ($groups as $dn => $group) {
        if (empty($group['memberUid']) || ((count($group['memberUid']) == 1) && ($group['memberUid'][0] == $this->getUid()))) {
          $tabobject = objects::open($dn, $type);
          $tabobject->delete();
        }
      }
    }

    return array();
  }

  /* Adapt from template, using 'dn' */
  function adapt_from_template($attrs, $skip = array())
  {
    /* Include global link_info */

    parent::adapt_from_template($attrs, $skip);

    $this->savedGroupMembership = $this->groupMembership;
    if (isset($this->attrs['posixGroups'])) {
      unset($this->attrs['posixGroups']['count']);
      $this->groupMembership = $this->attrs['posixGroups'];
    }

    if (isset($this->attrs['force_ids'])) {
      $this->force_ids = ($this->attrs['force_ids'][0] != 'FALSE');
    }

    if (isset($this->attrs['mustchangepassword'])) {
      $this->mustchangepassword = ($this->attrs['mustchangepassword'][0] != 'FALSE');
    }

    $this->primaryGroup = $this->gidNumber;

    if ((count($this->host) == 1) && ($this->host[0] == '*')) {
      $this->trustMode = 'fullaccess';
    } elseif (count($this->host) > 0) {
      $this->trustMode = 'byhost';
    }

    $this->fromTemplate = TRUE;
  }

  function foreignKeyUpdate ($field, $oldvalue, $newvalue, $source)
  {
    $ret = parent::foreignKeyUpdate($field, $oldvalue, $newvalue, $source);
    if ($field == 'gidNumber') {
      if ($newvalue === NULL) {
        $this->primaryGroup = 0;
      } else {
        $this->primaryGroup = $this->gidNumber;
      }
    }
    return $ret;
  }

  /*!
   * \brief Get next id
   *
   * \param string $attrib attribute to use to find the next id.
   *
   * \param String $dn Dn we want an id for.
   *
   * \return Return the next id or NULL if failed
   */
  static function getNextId($attrib, $dn)
  {
    global $config;

    if ($config->get_cfg_value('nextIdHook') != '') {
      return static::getNextIdHook($attrib, $dn);
    }

    switch ($config->get_cfg_value('idAllocationMethod', 'traditional')) {
      case 'pool':
        return static::getNextIdPool($attrib);
      default:
        msg_dialog::display(_('Warning'), sprintf(_('Unknown ID allocation method "%s"!'), $config->get_cfg_value('idAllocationMethod', 'traditional')), WARNING_DIALOG);
      case 'traditional':
        return static::getNextIdTraditional($attrib);
    }
  }

  /*!
   * \brief Get next id and add a lock to prevent other user to use it before us
   */
  static function getNextIdLock($attrib, $dn)
  {
    /* Calculate new id's. We need to place a lock before calling getNextId
       to get real unique values.
     */
    $wait = 10;
    while ($user = get_lock($attrib)) {
      sleep (1);

      /* Oups - timed out */
      if ($wait-- == 0) {
        msg_dialog::display(
          _('Warning'),
          sprintf(_('Timeout while waiting for lock. Ignoring lock from %s!'), $user),
          WARNING_DIALOG
        );
        break;
      }
    }
    add_lock($attrib, $dn);
    return static::getNextId($attrib, $dn);
  }

  /*!
   * \brief Get next id from the sambaUnixIdPool
   *
   * \param string $attrib attribute to use to find the next id.
   *
   * \return Return the next id or NULL if failed
   */
  protected static function getNextIdPool($attrib)
  {
    global $config;

    /* Fill informational values */
    $min = $config->get_cfg_value("${attrib}PoolMin", 10000);
    $max = $config->get_cfg_value("${attrib}PoolMax", 40000);

    /* Sanity check */
    if ($min >= $max) {
      msg_dialog::display(_('Error'), _('Cannot allocate a free ID:').' '.sprintf(_('%sPoolMin >= %sPoolMax!'), $attrib), ERROR_DIALOG);
      return NULL;
    }

    /* ID to skip */
    $ldap = $config->get_ldap_link();
    $id   = NULL;

    /* Try to allocate the ID several times before failing */
    $tries = 3;
    while ($tries--) {

      /* Look for ID map entry */
      $ldap->cd ($config->current['BASE']);
      $ldap->search ("(&(objectClass=sambaUnixIdPool)($attrib=*))", array("$attrib"));

      /* If it does not exist, create one with these defaults */
      if ($ldap->count() == 0) {
        /* Fill informational values */
        $minUserId  = $config->get_cfg_value('uidPoolMin',  10000);
        $minGroupId = $config->get_cfg_value('gidPoolMin',  10000);

        /* Add as default */
        $attrs = array(
          'objectClass' => array('organizationalUnit', 'sambaUnixIdPool'),
          'ou'          => 'idmap',
          'uidNumber'   => $minUserId,
          'gidNumber'   => $minGroupId,
        );
        $ldap->cd('ou=idmap,'.$config->current['BASE']);
        $ldap->add($attrs);
        if (!$ldap->success()) {
          msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), 'ou=idmap,'.$config->current['BASE'], LDAP_ADD, ERROR_DIALOG));
          return NULL;
        }
        $tries++;
        continue;
      }
      /* Bail out if it's not unique */
      if ($ldap->count() != 1) {
        msg_dialog::display(_('Error'), _('Cannot allocate a free ID:').' '._('sambaUnixIdPool is not unique!'), ERROR_DIALOG);
        return NULL;
      }

      /* Store old attrib and generate new */
      $attrs    = $ldap->fetch();
      $dn       = $attrs['dn'];
      $oldAttr  = $attrs[$attrib][0];
      $newAttr  = $oldAttr + 1;

      /* Sanity check */
      if ($newAttr >= $max) {
        msg_dialog::display(_('Error'), _('Cannot allocate a free ID:').' '._('no ID available!'), ERROR_DIALOG);
        return NULL;
      }
      if ($newAttr < $min) {
        msg_dialog::display(_('Error'), _('Cannot allocate a free ID:').' '._('no ID available!'), ERROR_DIALOG);
        return NULL;
      }

      $ldap->cd($dn);
      $ldap->modify(array($attrib => $newAttr));
      if (!$ldap->success()) {
        msg_dialog::display(_('Error'), _('Cannot allocate a free ID:').' '.$ldap->get_error(), ERROR_DIALOG);
        return NULL;
      } else {
        return $oldAttr;
      }
    }

    /* Bail out if we had problems getting the next id */
    if (!$tries) {
      msg_dialog::display(_('Error'), _('Cannot allocate a free ID:').' '._('maximum tries exceeded!'), ERROR_DIALOG);
    }

    return $id;
  }

  /*!
   * \brief Get next id in a traditional unix way
   *
   * \param string $attrib attribute to use to find the next id.
   *
   * \param String $dn Dn we want an id for.
   *
   * \return Return the next id
   */
  protected static function getNextIdTraditional($attrib)
  {
    global $config;

    $ids = array();
    $ldap = $config->get_ldap_link();

    $ldap->cd ($config->current['BASE']);
    if (preg_match('/gidNumber/i', $attrib)) {
      $oc = 'posixGroup';
    } else {
      $oc = 'posixAccount';
    }
    $ldap->search ("(&(objectClass=$oc)($attrib=*))", array("$attrib"));

    /* Get list of ids */
    while ($attrs = $ldap->fetch()) {
      $ids[] = (int)$attrs["$attrib"][0];
    }

    /* Add the nobody id */
    $ids[] = 65534;

    /* get the ranges */
    $tmp = array('0' => 1000);
    if (preg_match('/posixAccount/', $oc) && $config->get_cfg_value('uidNumberBase') != '') {
      $tmp = explode('-', $config->get_cfg_value('uidNumberBase'));
    } elseif ($config->get_cfg_value('gidNumberBase') != '') {
      $tmp = explode('-', $config->get_cfg_value('gidNumberBase'));
    }

    /* Set hwm to max if not set - for backward compatibility */
    $base = $tmp[0];
    if (isset($tmp[1])) {
      $hwm = $tmp[1];
    } else {
      $hwm = pow(2, 32);
    }
    /* Find out next free id near to UID_BASE */
    for ($id = $base; $id++; $id < $hwm) {
      if (!in_array($id, $ids)) {
        return $id;
      }
    }

    /* Should not happen */
    if ($id == $hwm) {
      msg_dialog::display(_('Error'), _('Cannot allocate a free ID!'), ERROR_DIALOG);
      exit;
    }
  }

  /*!
   * \brief Get idNumber from external hook
   *
   * \param array $attrib
   *
   * \param string $dn The DN
   */
  protected static function getNextIdHook($attrib, $dn)
  {
    global $config;

    if ($config->get_cfg_value('nextIdHook') != '') {
      /* Call hook script - if present */
      $command = $config->get_cfg_value('nextIdHook');

      $command .= ' '.escapeshellarg($dn).' '.escapeshellarg($attrib);
      @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, 'Execute');
      exec($command, $output, $returnCode);

      $str = implode("\n", $output);
      if ($returnCode != 0) {
        /* Failure */
        @DEBUG(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, 'Execution failed code: '.$returnCode);
        msg_dialog::display(
          _('Warning'),
          sprintf(
            _("%s\nResult: %s\nUsing default base!"),
            msgPool::cmdexecfailed('nextIdHook', $command),
            $str
          ),
          WARNING_DIALOG
        );
        return $config->get_cfg_value('uidNumberBase');
      } elseif (isset($output[0]) && is_numeric($output[0])) {
        /* Success */
        @DEBUG(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, 'Result: '.$str);
        return $output[0];
      } else {
        /* Invalid output */
        msg_dialog::display(
          _('Warning'),
          sprintf(
            _("%s\nResult: %s\nUsing default base!"),
            _('"nextIdHook" did not return a valid output!'),
            $str
          ),
          WARNING_DIALOG
        );
        return $config->get_cfg_value('uidNumberBase');
      }
    }

    msg_dialog::display(_('Warning'), _('"nextIdHook" is not available. Using default base!'), WARNING_DIALOG);
    return $config->get_cfg_value('uidNumberBase');
  }
}
?>
