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

class pgpServerInfo extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('GPG server info'),
      'plDescription' => _('GPG server info'),
      'plSection'     => 'conf',
      'plPriority'    => 5,
      'plIcon'        => 'geticon.php?context=applications&icon=gpg&size=48',
      'plObjectClass' => ['pgpServerInfo'],
      'plObjectType'  => [
        'pgpServerInfo' => [
          'name'        => _('GPG server info'),
          'icon'        => 'geticon.php?context=applications&icon=gpg&size=16',
          'aclCategory' => 'configuration',
        ]
      ],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    global $config;
    return [
      // Attributes are grouped by section
      'main' => [
        'name'  => _('GPG server info'),
        'attrs' => [
          new HiddenAttribute('cn', TRUE, 'pgpServerInfo'),
          new StringAttribute(
            _('Software'), _('Which software this GPG key server is running'),
            'pgpSoftware', FALSE,
            'OpenLDAP 2.4.33'
          ),
          new StringAttribute(
            _('GPG version'), _('Which version of GPG this key server is running'),
            'pgpVersion', FALSE,
            'gpg (GnuPG) 1.4.13'
          ),
          new CompositeAttribute(
            _('Branch in which keys are stored'),
            'pgpBaseKeySpaceDN',
            [
              new StringAttribute(
                _('Keys RDN'), _('Branch under base in which keys are stored'),
                'BaseKeyBranchDN', TRUE,
                'ou=PGP Keys,'
              )
            ],
            '/^(.*)'.$config->current['BASE'].'$/',
            '%s'.$config->current['BASE']
          ),
        ]
      ],
    ];
  }

  protected function ldap_save (): array
  {
    global $config;
    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    /* Create the new branch if non-existing */
    $ldap = $config->get_ldap_link();
    $ldap->cat($this->pgpBaseKeySpaceDN);
    if (!$ldap->count()) {
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->pgpBaseKeySpaceDN, "Creating branch");
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees($this->pgpBaseKeySpaceDN);
      if (!$ldap->success()) {
        $errors[] = msgPool::ldaperror($ldap->get_error(), $this->pgpBaseKeySpaceDN, LDAP_MOD, get_class());
      }
    }
    /* Delete the old branch if empty */
    if ($this->attributesAccess['pgpBaseKeySpaceDN']->hasChanged()) {
      $dn = $this->attributesAccess['pgpBaseKeySpaceDN']->getInitialValue();
      $ldap->cat($dn);
      if ($ldap->count()) {
        /* Old branch exists */
        $ldap->cd($dn);
        $ldap->search('(objectClass=*)');
        if (!$ldap->count()) {
          /* Old branch is empty */
          @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Deleting old empty branch");
          $ldap->rmdir($dn);
        } else {
          @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Old branch is not empty");
        }
      }
    }
    return $errors;
  }

  static function mainInc ($classname = NULL, $entry_dn = NULL, $tabs = FALSE, $edit_mode = TRUE, $objectType = FALSE)
  {
    global $config;

    if ($classname === NULL) {
      $classname = get_called_class();
    }

    if ($entry_dn === NULL) {
      $entry_dn = 'cn=pgpServerInfo,'.$config->current['BASE'];
    }

    parent::mainInc($classname, $entry_dn, $tabs, $edit_mode, $objectType);
  }
}
