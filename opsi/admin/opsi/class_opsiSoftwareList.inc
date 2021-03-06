<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2019 FusionDirectory

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

class opsiSoftwareList extends simplePlugin
{
  var $macAddress = NULL;
  var $products;

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('OPSI list'),
      'plDescription' => _('OPSI software list'),
      'plIcon'        => 'geticon.php?context=applications&icon=opsi-software-list&size=16',
      'plObjectClass' => ['opsiSoftwareList'],
      'plObjectType'  => ['opsiSoftwareList' => [
        'name'    => _('OPSI software list'),
        'ou'      => get_ou('opsiRDN'),
        'icon'    => 'geticon.php?context=applications&icon=opsi-software-list&size=16',
      ]],
      'plForeignKeys'   => [
        'fdOpsiServerDn' => 'serverGeneric',
      ],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Infos'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('opsiRDN')),
          new SubmittingOPSIServerAttribute(
            _('OPSI server'), _('OPSI server to use for deployment'),
            'fdOpsiServerDn', TRUE
          ),
          new HostNameAttribute(
            _('Name'), _('Name of this OPSI profile'),
            'cn', TRUE
          ),
        ]
      ],
      'products' => [
        'name'  => _('Softwares'),
        'attrs' => [
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('The localboot products to install with this list'),
              'fdOpsiLocalbootProduct',
              [
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_product', TRUE
                ),
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_action', TRUE,
                  ['setup', 'always', 'once', 'custom', 'userlogin', 'update']
                )
              ],
              '|', '', _('Localboot products')
            ),
            FALSE, // non-ordered
            [],
            TRUE // edition
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique('one');
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->setSubmitForm('product_changed');
  }

  public function server_changed ($opsi_args = NULL)
  {
    global $config;
    if ($opsi_args == NULL) {
      $opsi_args = ['id','name','setupScript','alwaysScript','onceScript','customScript','userLoginScript','updateScript','productVersion','packageVersion'];
    }
    if (!isset($this->products[$this->fdOpsiServerDn])) {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
        $error->display();
        return;
      }
      try {
        $this->macAddress = opsiProfile::getOpsiServerMac($this->fdOpsiServerDn);
      } catch (FusionDirectoryError $error) {
        $error->display();
        return FALSE;
      }
      $localboots = $s_daemon->append_call('OPSI.get_localboots', $this->macAddress, ['args' => [$opsi_args]]);
      if ($s_daemon->is_error()) {
        $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
        $error->display();
        return;
      }
      $this->products[$this->fdOpsiServerDn]['localboots'] = [];
      foreach ($localboots as $localboot) {
        $this->products[$this->fdOpsiServerDn]['localboots'][$localboot['id']] = $localboot;
      }
    }
    $choices = [];
    foreach ($this->products[$this->fdOpsiServerDn]['localboots'] as $id => $infos) {
      $choices[$id] = sprintf(_('%s (%s-%s)'), $infos['id'], $infos['productVersion'], $infos['packageVersion']);
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->setChoices(
      array_keys($choices),
      array_values($choices)
    );
  }

  public function product_changed ()
  {
    $localboot = $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->getValue();

    $actions = [];
    foreach (['setup', 'always', 'once', 'custom', 'userLogin', 'update'] as $action) {
      if (!empty($this->products[$this->fdOpsiServerDn]['localboots'][$localboot][$action.'Script'])) {
        $actions[] = strtolower($action);
      }
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[1]->setChoices($actions);
  }
}
