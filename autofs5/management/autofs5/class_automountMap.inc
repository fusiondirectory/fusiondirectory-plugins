<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2020  FusionDirectory

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

class automountMap extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Autofs map'),
      'plDescription'   => _('Autofs map'),
      'plObjectClass'   => ['automountMap'],
      'plObjectType'    => ['automountMap' => [
        'name'        => _('Autofs map'),
        'ou'          => get_ou('autofs5RDN'),
        'icon'        => 'geticon.php?context=applications&icon=autofs-map&size=16',
        'mainAttr'    => 'ou',
      ]],
      'plSearchAttrs' => ['ou'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Map'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('autofs5RDN')),
          new HostNameAttribute(
            _('Name'), _('Name of the map'),
            'ou', TRUE
          ),
        ],
      ],
      'entries' => [
        'name'  => _('Entries'),
        'class' => ['fullwidth'],
        'attrs' => [
          new SubNodesAttribute(
            _('Entries'), _('Entries in the automount table'),
            'automountEntries', 'automount',
            [
              new StringAttribute(
                _('Path'), _('Absolute (direct) or relative (indirect) path for this entry'),
                'cn', TRUE,
                '', '',
                '/^[a-z0-9\.\-\/]*$/i'
              ),
              new TextAreaAttribute(
                _('Options'), _("The entry of this directory for the automount daemon.\n For instance 'auto.u' or '-fstype=nfs domaine.tld:/mount/directory'"),
                'automountInformation', TRUE
              ),
              new StringAttribute(
                _('Description'), _('Description of this entry'),
                'description', FALSE
              ),
            ]
          ),
        ]
      ]
    ];
  }

  public function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['automountEntries']->setLinearRendering(FALSE);
  }

  protected function shouldSave (): bool
  {
    return (parent::shouldSave() || $this->attributesAccess['automountEntries']->hasChanged());
  }

  protected function ldap_save (): array
  {
    global $config;

    $errors = parent::ldap_save();

    $ldap = $config->get_ldap_link();

    $this->attributesAccess['automountEntries']->postLdapSave($ldap);

    return $errors;
  }
}
