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

class IEManager extends simplePlugin
{
  /* Return plugin information for acl handling */
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Export Manager'),
      'plDescription' => _('Export the ldap tree to/from LDIF format'),
      'plObjectType'  => ['special'],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'export' => [
        'name'  => _('Export'),
        'attrs' => [
          new CompositeAttribute(
            _('DN of a single entry to export as ldif'),
            'single_export',
            [
              new ButtonAttribute(
                '', '',
                'single_submit',
                _('Single Export')
              )
            ],
            '', '%s%s', '',
            _('Export single entry')
          ),
          new HiddenAttribute(
            'export_filter', FALSE,
            '(objectClass=*)',
            'complete_export'
          ),
          new CompositeAttribute(
            _('Download a complete snapshot of the running LDAP directory for this base as ldif'),
            'complete_export',
            [
              new ButtonAttribute(
                '', '',
                'complete_submit',
                _('Complete Export ')
              )
            ],
            '', '%s%s', '',
            _('Export complete LDIF for')
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['single_export']->setInLdap(FALSE);
    $this->attributesAccess['single_export']->setLinearRendering(TRUE);
    $this->attributesAccess['complete_export']->setInLdap(FALSE);
    $this->attributesAccess['complete_export']->setLinearRendering(TRUE);
  }

  function handle_single_submit ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    var_dump($ldap);
    $dn   = $this->dn;
    $acl  = $this->attributesAccess['single_export']->getAcl();
    if (!$ldap->dn_exists($dn)) {
      $error = new FusionDirectoryError(htmlescape(sprintf(_('No such object %s!'), $dn)));
      $error->display();
      $dn = '';
    }
    $this->save_export($dn, $acl, 'base', 'entryExport.ldif');
  }

  function handle_complete_submit ()
  {
    $acl    = $this->attributesAccess['complete_export']->getAcl();
    $this->save_export($this->dn, $acl, 'sub', 'fullExport.ldif');
  }

  function save_export ($dn, $acl, $scope, $name)
  {
    global $config;
    // An LDIF export was asked
    if (!empty($dn)) {
      // Check permissions
      if (!$this->acl_is_writeable($acl, $this->acl_skip_write())) {
        $error = new FusionDirectoryPermissionError(htmlescape(sprintf(_('You have no permission to export %s!'), $dn)));
        $error->display();
      } else {
        $ldap = $config->get_ldap_link();
        try {
          $data = $ldap->generateLdif($dn, $this->export_filter, $scope, 0, intval($this->wrap));
          send_binary_content($data, $name);
        } catch (LDIFExportException $e) {
          $error = new FusionDirectoryError(
            htmlescape(sprintf(
              _('Failed to generate ldap export, error was "%s"!'),
              $e->getMessage()
            ))
          );
          $error->display();
        }
      }
    }
  }

  function save (): array
  {
    return [];
  }

  function remove (bool $fulldelete = FALSE): array
  {
    return [];
  }

  function get_allowed_bases (): array
  {
    global $config;
    return $config->getDepartmentTree();
  }
}
