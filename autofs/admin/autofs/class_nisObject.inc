<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2019  FusionDirectory

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

class nisObject extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Directory'),
      'plDescription'   => _('Directory'),
      'plObjectClass'   => ['nisObject'],
      'plObjectType'    => ['nisObject' => [
        'name'        => _('Directory'),
        'ou'          => get_ou('autofsRDN'),
        'icon'        => 'geticon.php?context=applications&icon=autofs-nis-object&size=16',
        'mainAttr'    => 'cn',
      ]],
      'plForeignKeys'  => [
        'nisMapName'   => [
          ['nisMap', 'nisMapName'],
        ],
      ],
      'plSearchAttrs' => ['nisMapName', 'nisMapEntry'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Directory'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('autofsRDN')),
          new StringAttribute(
            _('Name'), _('Name of this directory'),
            'cn', TRUE,
            '', '',
            '/^[a-z0-9\.\-\/]*$/i', 'nfsPartage'
          ),
          new TextAreaAttribute(
            _('Automount entry'), _("The entry of this directory for the automount daemon.\n For instance 'auto.u' or '-fstype=nfs domaine.tld:/mount/directory'"),
            'nisMapEntry', TRUE
          ),
          new SelectAttribute(
            _('Mount point'), _('The mount point this directory will be placed in'),
            'nisMapName', TRUE,
            []
          )
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['nisMapName']->setChoices($this->getMountPoints());
  }

  function save_object ()
  {
    parent::save_object();
    $this->attributesAccess['nisMapName']->setChoices($this->getMountPoints());
  }

  function getMountPoints ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd(get_ou('autofsRDN').$this->base);
    $ldap->search('(objectClass=nisMap)', ['nisMapName']);
    $nisMaps = [];
    while ($attrs = $ldap->fetch()) {
      $nisMaps[] = $attrs['nisMapName'][0];
    }
    return $nisMaps;
  }

  /* We need to fix the base */
  function get_allowed_bases (): array
  {
    $this->base = preg_replace('/^nisMapName=[^,]+,/', '', $this->base);
    return parent::get_allowed_bases();
  }

  function compute_dn (): string
  {
    return 'cn='.ldap_escape($this->attributesAccess['cn']->getValue(), '', LDAP_ESCAPE_DN).','.
            'nisMapName='.ldap_escape($this->nisMapName, '', LDAP_ESCAPE_DN).','.
              get_ou('autofsRDN').$this->base;
  }
}
