<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016 FusionDirectory

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

class faiScript extends faiSimplePluginClass
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Script'),
      'plDescription' => _('FAI script'),
      'plObjectClass' => ['FAIclass','FAIscript'],
      'plFilter'      => '(objectClass=FAIscript)',
      'plObjectType'  => [
        'faiScript' => [
          'name'        => _('FAI script'),
          'aclCategory' => 'fai',
          'ou'          => get_ou('faiScriptRDN'),
          'icon'        => 'geticon.php?context=applications&icon=fai-script&size=16'
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
        'name'  => _('Scripts'),
        'class' => ['fullwidth'],
        'attrs' => [
          new SubNodesAttribute(
            '', _('Variables in this class'),
            'scripts', ['FAIscriptEntry','FAIclass','top'],
            [
              new StringAttribute(
                _('Name'), _('The name of the variable'),
                'cn', TRUE
              ),
              new StringAttribute(
                _('Description'), _('A short description of the variable'),
                'description', FALSE
              ),
              new IntAttribute(
                _('Priority'), _('Priority of this script (smaller is done first)'),
                'FAIpriority', TRUE,
                0, 99, 1
              ),
              new FileTextAreaAttribute(
                _('Script'), _('The script itself'),
                'FAIscript', TRUE,
                '.sh'
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
    $this->attributesAccess['scripts']->setLinearRendering(FALSE);
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
    $this->attributesAccess['scripts']->postLdapSave($config->get_ldap_link());
    return $errors;
  }
}
