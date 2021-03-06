<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

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

class ldapDump extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('LDAP'),
      'plDescription' => _('LDAP Dump'),
      'plObjectType'  => ['special'],
      'plPriority'    => 99,

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'      => _('LDAP Dump'),
        'class'     => ['fullwidth'],
        'attrs'     => [
          new FakeAttribute('dump')
        ],
        'template'  => get_template_path('ldapdump.tpl', TRUE, dirname(__FILE__))
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->dump = NULL;
  }

  function aclHasPermissions (): bool
  {
    return TRUE;
  }

  function aclGetPermissions ($attribute = '0', string $base = NULL, bool $skipWrite = FALSE): string
  {
    global $ui;
    /* You need complete rights on the category to see anything in this tab */
    return $ui->get_complete_category_acls($this->getAclBase(), $this->parent->acl_category);
  }

  public function render (): string
  {
    global $config;
    if ($this->dump === NULL) {
      if (preg_match('/r/', $this->aclGetPermissions())) {
        $ldap = $config->get_ldap_link();
        $ldap->cat($this->dn, ['*','createTimestamp','modifyTimestamp']);
        if ($attrs = $ldap->fetch()) {
          $this->dump = [$this->dumpLdapNode($attrs, $config->current['BASE'])];
        } else {
          $this->dump = [];
        }
      } else {
        $this->dump = FALSE;
      }
    }
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    return parent::render();
  }

  function dumpLdapNode ($attrs, $parent_dn = '')
  {
    global $config;
    $node = [
      'dn'        => $attrs['dn'],
      'name'      => preg_replace('/,'.preg_quote($parent_dn).'$/', '', $attrs['dn']),
      'attrs'     => [],
      'subnodes'  => [],
    ];
    for ($i = 0; $i < $attrs['count']; $i++) {
      $key                  = $attrs[$i];
      if (in_array($key, ['createTimestamp', 'modifyTimestamp'])) {
        continue;
      }
      $node['attrs'][$key]  = $attrs[$key];
      unset($node['attrs'][$key]['count']);
    }
    $timestamps = [];
    if (isset($attrs['createTimestamp'][0])) {
      $date = LdapGeneralizedTime::fromString($attrs['createTimestamp'][0]);
      $date->setTimezone(timezone::getDefaultTimeZone());
      $timestamps['createTimestamp'] = $date->format('Y-m-d H:i:s');
    }
    if (isset($attrs['modifyTimestamp'][0])) {
      $date = LdapGeneralizedTime::fromString($attrs['modifyTimestamp'][0]);
      $date->setTimezone(timezone::getDefaultTimeZone());
      $timestamps['modifyTimestamp'] = $date->format('Y-m-d H:i:s');
    }
    $node['attrs'] = $timestamps + $node['attrs'];

    $ldap = $config->get_ldap_link();
    $ldap->cd($attrs['dn']);
    $ldap->search('(objectClass=*)', ['*','createTimestamp','modifyTimestamp'], 'one');
    while ($attrs = $ldap->fetch()) {
      $node['subnodes'][$attrs['dn']] = $this->dumpLdapNode($attrs, $node['dn']);
    }

    return $node;
  }

  function check (): array
  {
    return [];
  }

  function save (): array
  {
    return [];
  }

  function remove (bool $fulldelete = FALSE): array
  {
    return [];
  }
}
