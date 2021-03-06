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

class argonautFuseConfig extends simpleService
{
  protected $plugin;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Argonaut Fuse'),
      'plDescription'   => _('Argonaut Fuse settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=argonaut-fuse&size=16',
      'plObjectClass'   => ['argonautFuseConfig'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    return  [
      'main' => [
        'name'  => _('Basic settings'),
        'attrs' => [
          new StringAttribute(
            _('Default mode'),
            '',
            'argonautFuseDefaultMode',
            TRUE,
            'install'
          ),
          new StringAttribute(
            _('Log directory'),
            _('Directory in which argonaut-fuse will store its log'),
            'argonautFuseLogDir',
            TRUE,
            '/var/log/argonaut'
          ),
        ]
      ],
      'tftp' => [
        'name'  => _('TFTP'),
        'attrs' => [
          new StringAttribute(
            _('Pxelinux cfg path'),
            _('Path where argonaut-fuse should store pxelinux.cfg'),
            'argonautFusePxelinuxCfg',
            TRUE,
            '/srv/tftp/pxelinux.cfg'
          ),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $parent = NULL)
  {
    global $config;
    parent::__construct($dn, $parent);

    /* Load modules */
    $this->plugin = [];
    foreach ($config->data['TABS']['FUSEMODULETABS'] as $plug) {
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $plug['CLASS'], "Loading Fuse module");
      if (!plugin_available($plug['CLASS'])) {
        @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $plug['CLASS'], "Fuse module not available");
        continue;
      }
      $name = $plug['CLASS'];
      $this->plugin[$name] = new $name($dn, $parent, $parent);

      $this->plugin[$name]->set_acl_category($this->acl_category);
    }
  }

  public function readPost ()
  {
    parent::readPost();
    if (isset($_POST[get_class($this).'_posted'])) {
      foreach ($this->plugin as $plugin) {
        $plugin->readPost();
      }
    }
  }

  public function update (): bool
  {
    parent::update();
    foreach ($this->plugin as $plugin) {
      $plugin->update();
    }
    return TRUE;
  }

  public function render (): string
  {
    $str = '<div style="width:100%; text-align:right; clear:both; float:none;">'.
           '  <input type="submit" name="SaveService" value="'.msgPool::saveButton().'"/>&nbsp;'.
           '  <input type="submit" formnovalidate="formnovalidate" name="CancelService" value="'.msgPool::cancelButton().'"/>'.
           '</div>';

    $display = simplePlugin::render();

    foreach ($this->plugin as $plugin) {
      $display .= '<p class="seperator plugbottom">&nbsp;</p><div></div>';
      $display .= $plugin->render();
    }

    return $display.$str;
  }

  function check (): array
  {
    $errors = parent::check();

    foreach ($this->plugin as $plugin) {
      if ($plugin->is_account) {
        $errors = array_merge($errors, $plugin->check());
      }
    }

    return $errors;
  }

  function set_acl_category (string $cat)
  {
    parent::set_acl_category($cat);
    foreach ($this->plugin as $plugin) {
      $plugin->set_acl_category($cat);
    }
  }

  /* Save to LDAP */
  function save (): array
  {
    $errors = parent::save();
    if (!empty($errors)) {
      return $errors;
    }

    /* Save objects */
    foreach ($this->plugin as $plugin) {
      $plugin->dn = $this->dn;

      if ($plugin->is_account) {
        $errors = $plugin->save();
      } else {
        $errors = $plugin->remove(FALSE);
      }
      if (!empty($errors)) {
        return $errors;
      }
    }

    return [];
  }

  function remove (bool $fulldelete = FALSE): array
  {
    $errors = [];

    /* Remove objects */
    foreach ($this->plugin as $plugin) {
      $plugin->dn = $this->dn;
      $result = $plugin->remove($fulldelete);
      if (!empty($result)) {
        $errors = array_merge($errors, $result);
      }
    }

    if (!empty($errors)) {
      return $errors;
    }

    return parent::remove($fulldelete);
  }
}
