<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2011 Alejandro Escanero Blanco (aescanero@gmail.com)
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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class netgroup extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Netgroup'),
      'plDescription' => _('NIS Netgroup settings'),
      'plObjectClass' => ['nisNetgroup'],
      'plObjectType'  => ['netgroup' => [
        'name'    => _('NIS Netgroup'),
        'ou'      => get_ou('netgroupRDN'),
        'icon'    => 'geticon.php?context=applications&icon=netgroups&size=16'
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Information'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('netgroupRDN')),
          new HostNameAttribute(
            _('Name'), _('Name of this NIS netgroup'), 'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('Description of this NIS netgroup'), 'description'
          ),
        ]
      ],
      'users' => [
        'name'  => _('User members'),
        'icon'  => 'geticon.php?context=types&icon=user&size=16',
        'attrs' => [
          new HiddenArrayAttribute('nisNetgroupTriple'),
          new UsersAttribute(
            '', _('NIS netgroup members'), 'memberUsers',
            FALSE, [], 'dn', 'uid',
            [
              ['TabFilterElement',    []],
              ['FixedFilterElement',  ['(objectClass=posixAccount)']],
            ]
          ),
        ]
      ],
      'systems' => [
        'name'  => _('System members'),
        'icon'  => 'geticon.php?context=devices&icon=server&size=16',
        'attrs' => [
          new SystemsAttribute('', _('NIS netgroup members'), 'memberSystems', FALSE),
        ]
      ],
      'netgroups' => [
        'name'  => _('Netgroup members'),
        'icon'  => 'geticon.php?context=applications&icon=netgroups&size=16',
        'attrs' => [
          new ObjectsAttribute(
            '', _('NIS netgroup members'),
            'memberNisNetgroup', FALSE,
            ['netgroup'], [],
            'cn', 'cn'
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique('one');
    $this->attributesAccess['memberUsers']->setInLdap(FALSE);
    $this->attributesAccess['memberSystems']->setInLdap(FALSE);
    $triples      = $this->attributesAccess['nisNetgroupTriple']->getValue();
    $usersfilter  = '(|';
    $cns          = [];
    foreach ($triples as $triple) {
      if (preg_match('/^\(([^,]*),([^,]*),[^)]*\)$/', $triple, $matches)) {
        if ($matches[1] != "") {
          $cns[] = $matches[1];
        } elseif ($matches[2] != "") {
          $usersfilter .= '(uid='.$matches[2].')';
        }
      }
    }
    $usersfilter .= ')';

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=posixAccount)'.$usersfilter.')', ['uid']);
    $dns = [];
    while ($attrs = $ldap->fetch()) {
      $dns[] = $attrs['dn'];
    }
    $this->memberUsers    = $dns;
    $this->memberSystems  = $cns;
  }

  function prepare_save (): array
  {
    $memberUsers    = $this->attributesAccess['memberUsers']->getDisplayValues();
    $memberSystems  = $this->memberSystems;
    $triples = [];

    foreach ($memberUsers as $uid) {
      $triples[] = '(,'.$uid.',)';
    }

    foreach ($memberSystems as $cn) {
      $triples[] = '('.$cn.',,)';
    }

    $this->nisNetgroupTriple = $triples;

    return parent::prepare_save();
  }
}
