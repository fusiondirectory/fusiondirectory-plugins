<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
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

class PackageConfigDialog extends GenericDialog
{
  protected $dialogClass = 'faiPackageConfiguration';

  protected $post_cancel = 'CancelObjectConfig';
  protected $post_finish = 'SaveObjectConfig';

  function __construct ($simplePlugin, $attribute, $key, $pkg_config)
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass($simplePlugin->dn, $key, $simplePlugin->FAIdebianRelease, $pkg_config);
  }

  protected function handleFinish ()
  {
    $this->dialog->update();

    $msgs = $this->dialog->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    }

    $this->attribute->packageConfigured($this->dialog->save());
    unset($this->dialog);
  }
}

class faiPackage extends faiSimplePluginClass
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Package'),
      'plDescription' => _('FAI Package list'),
      'plObjectClass' => ['FAIclass','FAIpackageList','FAIrepository'],
      'plFilter'      => '(objectClass=FAIpackageList)',
      'plObjectType'  => [
        'faiPackage' => [
          'name'        => _('FAI Package'),
          'aclCategory' => 'fai',
          'ou'          => get_ou('faiPackageRDN'),
          'icon'        => 'geticon.php?context=applications&icon=fai-packages&size=16'
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
      'repo' => [
        'name'  => _('Repository'),
        'attrs' => [
          new DisplayLDAPAttribute(
            _('Release'), _('Debian release concerned'),
            'FAIdebianRelease', FALSE
          ),
          new DisplayLDAPArrayAttribute(
            _('Sections'), _('Sections concerned'),
            'FAIdebianSection', FALSE
          ),
          new SelectAttribute(
            _('Install method'), _('Install method to use for this package list'),
            'FAIinstallMethod', TRUE,
            ['install', 'ninstall', 'remove',
              'dselect-upgrade', 'taskinst', 'taskrm',
              'hold', 'clean', 'aptitude', 'aptitude-r',
              'pending', 'dpkgc', 'yumi'],
            'aptitude'
          ),
        ]
      ],
      'packages' => [
        'name'  => _('Packages'),
        'class' => ['fullwidth'],
        'attrs' => [
          new PackagesAttribute(
            '', _('Packages in this class'),
            'FAIpackage', TRUE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique('one');

    if ($this->FAIdebianRelease === '') {
      /* Assemble release name */
      $this->updateRelease();
    }
  }

  function updateRelease ()
  {
    /* Assemble release name */
    $tmp = preg_replace('/'.preg_quote(get_ou('faiBaseRDN'), '/').'.*$/i', '', $this->base);
    preg_match_all('/ou=([^=,]+),/', $tmp, $m);
    /* m[1][0] is release and m[1][1] is distribution */
    $this->FAIdebianRelease = $m[1][0];

    /* Assemble sections */
    $repos = $this->getServerInfos();
    if (isset($repos[$this->FAIdebianRelease])) {
      $this->FAIdebianSection = $repos[$this->FAIdebianRelease];
      $this->FAIdebianSection = array_unique($this->FAIdebianSection);
    }
  }

  protected function shouldSave (): bool
  {
    /* We need ldap_save to always run */
    return TRUE;
  }

  protected function ldap_save (): array
  {
    global $config;
    $errors = parent::ldap_save();
    $this->attributesAccess['FAIpackage']->postLdapSave($config->get_ldap_link());
    return $errors;
  }

  function getServerInfos ()
  {
    $servs    = serviceRepository::getServers();
    $ret      = [];
    foreach ($servs as $serv) {
      if (isset($ret[$serv['FAI_RELEASE']])) {
        $ret[$serv['FAI_RELEASE']] = array_merge($ret[$serv['FAI_RELEASE']], $serv['SECTIONS']);
      } else {
        $ret[$serv['FAI_RELEASE']] = $serv['SECTIONS'];
      }
    }
    return $ret;
  }
}
