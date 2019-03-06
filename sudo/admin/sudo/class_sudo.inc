<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
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

/*! \brief Sudo generic class. Allow setting User/Host/Command/Runas
           for a sudo role object.
 */
class sudo extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Sudo'),
      'plDescription' => _('Sudo role'),
      'plIcon'        => 'geticon.php?context=applications&icon=sudo&size=48',
      'plObjectClass' => ['sudoRole'],
      'plObjectType'  => ['sudo' => [
        'name'      => _('Sudo role'),
        'icon'      => 'geticon.php?context=applications&icon=sudo&size=16',
        'ou'        => get_ou('sudoRDN'),
        'tabClass'  => 'sudotabs',
      ]],
      'plForeignKeys'  => [
        'sudoUser'   => [
          ['user', 'uid', '(sudoUser=%oldvalue%)'],
          ['posixGroup', 'cn', '(sudoUser=%|%%oldvalue%)'],
          ['mixedGroup', 'cn', '(sudoUser=%|%%oldvalue%)'],
        ],
        'sudoHost'   => [
          ['serverGeneric', 'cn'],
          ['workstationGeneric', 'cn'],
          ['terminalGeneric', 'cn'],
        ],
      ],
      'plSearchAttrs' => ['description', 'sudoUser', 'sudoHost'],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return  [
      'section1' => [
        'name'  => _('Role settings'),
        'class' => ['fullwidth'],
        'attrs' => [
          new DefaultRoleAttribute(FALSE),
          new BaseSelectorAttribute(get_ou('sudoRDN')),
          new StringAttribute(
            _('Name'), _('Name of the role'),
            'cn', TRUE,
            ''
          ),
          new StringAttribute(
            _('Description'), _('Description for the new sudo role'),
            'description', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              _('Commands'),
              _('A Unix command with optional command line arguments, potentially including globbing characters (aka wild cards)'),
              'sudoCommand', FALSE
            )
          ),
          new SetAttribute(
            new StringAttribute(
              _('Run as (user)'),
              _('User(s) impersonated by sudo'),
              'sudoRunAsUser', FALSE
            ),
            ['ALL']
          ),
          new SetAttribute(
            new StringAttribute(
              _('Run as (group)'),
              _('Group(s) impersonated by sudo'),
              'sudoRunAsGroup', FALSE
            ),
            ['ALL']
          ),
          new SudoSystemsAttribute(
            _('Systems'), _('A host name, IP address or IP network'),
            'sudoHost', FALSE,
            ['ALL']
          ),
          new SudoUsersGroupsAttribute(
            _('Users and groups'),
            _("A user name, user ID (prefixed with '#'), Unix group (prefixed with '%')"),
            'sudoUser', FALSE
          ),
          new IntAttribute(
            _('Priority'), _('This rule priority compared to others'),
            'sudoOrder', FALSE,
            0, FALSE, 0
          ),
          new GeneralizedTimeDateAttribute(
            _('Valid starting from'), _('Start of time interval for which the entry is valid (leave empty to disable)'),
            'sudoNotBefore', FALSE,
            ''
          ),
          new GeneralizedTimeDateAttribute(
            _('Valid until'), _('End of time interval for which the entry is valid (leave empty to disable)'),
            'sudoNotAfter', FALSE,
            ''
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['isDefaultRole']->setValue($this->is_default());
    $this->attributesAccess['isDefaultRole']->setManagedAttributes(
      [
        'erase' => [
          TRUE => [
            'base',
            'sudoUser',
            'sudoCommand',
            'sudoHost',
            'sudoRunAsUser',
            'sudoRunAsGroup',
            'sudoOrder',
          ]
        ],
        'disable' => [
          TRUE => [
            'cn'
          ]
        ]
      ]
    );
  }


  public function set_default ($state)
  {
    $this->attributesAccess['isDefaultRole']->setValue($state);
  }

  public function is_default ()
  {
    return preg_match("/^defaults$/i", $this->attributesAccess['cn']->getValue());
  }

  function foreignKeyCheck (string $field, $fieldvalue, array $source)
  {
    if (($field == 'sudoUser') && (($source['CLASS'] == 'posixGroup') || ($source['CLASS'] == 'mixedGroup'))) {
      return parent::foreignKeyCheck($field, '%'.$fieldvalue, $source);
    } else {
      return parent::foreignKeyCheck($field, $fieldvalue, $source);
    }
  }

  function foreignKeyUpdate (string $field, $oldvalue, $newvalue, array $source)
  {
    if (($field == 'sudoUser') && (($source['CLASS'] == 'posixGroup') || ($source['CLASS'] == 'mixedGroup'))) {
      return parent::foreignKeyUpdate($field, '%'.$oldvalue, '%'.$newvalue, $source);
    } else {
      return parent::foreignKeyUpdate($field, $oldvalue, $newvalue, $source);
    }
  }
}
