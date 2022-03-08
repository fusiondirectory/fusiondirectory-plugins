<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2022  FusionDirectory

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

class posixGroup extends simplePlugin
{
  protected $locks = [];

  static function plInfo (): array
  {
    if (class_available('mixedGroup')) {
      return [];
    }
    return [
      'plShortName'   => _('Group'),
      'plDescription' => _('POSIX group information'),
      'plObjectClass' => ['posixGroup'],
      'plObjectType'  => ['group' => [
        'name'        => _('POSIX group'),
        'description' => _('POSIX user group'),
        'ou'          => get_ou('groupRDN'),
        'icon'        => 'geticon.php?context=types&icon=user-group&size=16',
        'mainAttr'    => 'cn',
      ]],
      'plForeignKeys'  => [
        'memberUid' => ['user','uid']
      ],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;

    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('groupRDN')),
          new StringAttribute(
            _('Name'), _('Name of this group'),
            'cn', TRUE,
            '', '', (strict_uid_mode() ? '/^[a-z0-9_-]+$/' : '/^[a-z0-9_.-]+$/i')
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of this group'),
            'description', FALSE
          ),
          new BooleanAttribute(
            _('Force GID'), _('Force GID value for this group'),
            'force_id', FALSE
          ),
          new IntAttribute(
            _('GID'), _('GID value for this group'),
            'gidNumber', FALSE,
            $config->get_cfg_value('minId', 0), FALSE, ''
          )
        ]
      ],
      'members' => [
        'name'  => _('Group members'),
        'attrs' => [
          new UsersAttribute(
            '', _('Group members'),
            'memberUid', FALSE,
            [], 'uid'
          )
        ]
      ],
      'system_trust' => [
        'name'  => _('System trust'),
        'icon'  => 'geticon.php?context=status&icon=locked&size=16',
        'attrs' => [
          new SelectAttribute(
            _('Trust mode'), _('Type of authorization for those hosts'),
            'trustMode', FALSE,
            ['', 'fullaccess', 'byhost'],
            '',
            [_('disabled'), _('full access'), _('allow access to these hosts')]
          ),
          new SystemsAttribute(
            '', _('Only allow this group to connect to this list of hosts'),
            'host', FALSE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    //This allows if trust mode is disable to remove any hosts from the list, removing from ldap as well.
    $this->attributesAccess['trustMode']->setManagedAttributes(
      [
        'multiplevalues' => ['notbyhost' => ['','fullaccess']],
        'erase' => [
          'notbyhost' => ['host']
        ]
      ]
    );

    //As trustMode is not registered in ldap, only host, this is a way to put back the option to the proper value
    if ((count($this->host) == 1) && ($this->host[0] == '*')) {
      $this->trustMode = 'fullaccess';
    } elseif (count($this->host) > 0) {
      $this->trustMode = 'byhost';
    }

    $this->attributesAccess['gidNumber']->setUnique('whole');
    $this->attributesAccess['force_id']->setManagedAttributes(
      [
        'disable' => [
          FALSE => [
            'gidNumber',
          ]
        ]
      ]
    );
  }

  function resetCopyInfos ()
  {
    parent::resetCopyInfos();

    $this->force_id = FALSE;
    $this->attributesAccess['gidNumber']->setInitialValue("");
    $this->gidNumber = "";
  }

  function prepare_save (): array
  {
    if (!$this->force_id) {
      // Handle gidNumber automatic generation if required
      if ($this->is_template) {
        $this->gidNumber = '';
      } else {
        if ($this->attributesAccess['gidNumber']->getInitialValue() != '') {
          $this->gidNumber = $this->attributesAccess['gidNumber']->getInitialValue();
        } else {
          $this->gidNumber = posixAccount::getNextIdLock('gidNumber', $this->dn);
          $this->locks[] = 'gidNumber';
        }
      }
    }

    $errors = parent::prepare_save();
    if (!empty($errors)) {
      return $errors;
    }

    if ($this->trustMode == 'fullaccess') {
      $this->attrs['host'] = ['*'];
    }

    //Register hosts selected within LDAP
    if (($this->trustMode != "") && !in_array('hostObject', $this->attrs['objectClass'])) {
      $this->attrs['objectClass'][] = 'hostObject';
    } elseif (($this->trustMode == "") && (($key = array_search('hostObject', $this->attrs['objectClass'])) !== FALSE)) {
      unset($this->attrs['objectClass'][$key]);
    }

    return $errors;
  }

  function save (): array
  {
    //Important to get attrs from ldap fdtemplate and to not register them within ldap afterwards
    $this->attributesAccess['trustMode']->setInLdap($this->is_template);
    $this->attributesAccess['force_id']->setInLdap($this->is_template);

    $errors = parent::save();

    foreach ($this->locks as $lock) {
      Lock::deleteByObject($lock);
    }
    $this->locks = [];

    return $errors;
  }

  //Below function allows force_id and trustMode to be retrieved from ldap fdTemplate
  function adapt_from_template (array $attrs, array $skip = [])
  {
    parent::adapt_from_template($attrs, $skip);

    //Important to not register within ldap afterwards
    $this->attributesAccess['trustMode']->setInLdap(FALSE);
    $this->attributesAccess['force_id']->setInLdap(FALSE);

    $this->fromTemplate = TRUE;
  }

  function addUser ($dn, $uid)
  {
    $this->attributesAccess['memberUid']->addValue($dn,
      [
        'dn'          => $dn,
        'objectClass' => ['inetOrgPerson'],
        'uid'         => [$uid],
        'cn'          => [$uid]
      ]
    );
  }

  function removeUser ($uid)
  {
    $this->attributesAccess['memberUid']->searchAndRemove($uid);
  }
}
