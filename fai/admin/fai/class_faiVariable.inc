<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016  FusionDirectory

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

class faiVariable extends faiSimplePluginClass
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Variable'),
      'plDescription' => _('FAI variable'),
      'plObjectClass' => ['FAIclass','FAIvariable'],
      'plFilter'      => '(objectClass=FAIvariable)',
      'plObjectType'  => [
        'faiVariable' => [
          'name'        => _('FAI variable'),
          'aclCategory' => 'fai',
          'ou'          => get_ou('faiVariableRDN'),
          'icon'        => 'geticon.php?context=applications&icon=fai-variable&size=16'
        ]
      ],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new HostNameAttribute(
            _('Class name'), _('Variables class name'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('Short description of the class'),
            'description', FALSE
          ),
        ]
      ],
      'vars' => [
        'name'  => _('Variables'),
        'class' => ['fullwidth'],
        'attrs' => [
          new SubNodesAttribute(
            '', _('Variables in this class'),
            'variables', ['FAIvariableEntry','FAIclass','top'],
            [
              new StringAttribute(
                _('Name'), _('The name of the variable'),
                'cn', TRUE
              ),
              new StringAttribute(
                _('Description'), _('A short description of the variable'),
                'description', FALSE
              ),
              new StringAttribute(
                _('Content'), _('The content of the variable'),
                'FAIvariableContent', TRUE
              ),
            ],
            FALSE, /* no order */
            [],
            TRUE /* edit enabled */
          ),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique('one');
    $this->attributesAccess['variables']->setLinearRendering(FALSE);
  }

  protected function shouldSave (): bool
  {
    /* We need ldap_save to always run */
    return TRUE;
  }

  function ldap_save (): array
  {
    global $config;
    $errors = parent::ldap_save();
    $this->attributesAccess['variables']->postLdapSave($config->get_ldap_link());
    return $errors;
  }
}
