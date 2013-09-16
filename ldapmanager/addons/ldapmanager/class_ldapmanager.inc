<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class ldapmanager extends simplePlugin
{
  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('LDIF'),
      'plDescription' => _('Export/Import the ldap tree to/from LDIF format'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('ldapmanager' => array(
        'name'        => _('LDAP Manager'),
        'aclCategory' => 'ldapmanager'
      )),

      'plProvidedAcls'  => array()
    );
  }

  static function getAttributesInfo ()
  {
    global $config;
    return array(
      'export' => array(
        'name'  => _('Export'),
        'attrs' => array(
          new CompositeAttribute (
            _('DN of a single entry to export as ldif'),
            'single_export',
            array(
              new StringAttribute (
                '', '',
                'single_dn', FALSE
              ),
              new ButtonAttribute (
                '', '',
                'single_submit', FALSE,
                _('Export'), ''
              )
            ),
            '', '%s%s', '',
            _('Export single entry')
          ),
          new StringAttribute (
            _('Filter'), _('Filter to use for selecting objects to export'),
            'export_filter', FALSE,
            '(objectClass=*)'
          ),
          new CompositeAttribute (
            _('Download a complete snapshot of the running LDAP directory for this base as ldif'),
            'complete_export',
            array(
              new BaseSelectorAttribute ('', '', ''),
              new ButtonAttribute (
                '', '',
                'complete_submit', FALSE,
                _('Export'), ''
              )
            ),
            '', '%s%s', '',
            _('Export complete LDIF for')
          )
        )
      ),
      'import' => array(
        'name'  => _('Import LDIF'),
        'attrs' => array(
          new BooleanAttribute (
            _('Overwrite existing entries'), _('Remove fields that are not in the LDIF from the LDAP entries if they were existing.'),
            'overwrite', FALSE,
            FALSE, 'import'
          ),
          new CompositeAttribute (
            _('Import an LDIF file into your LDAP. Remember that FusionDirectory will not check your LDIFs for FusionDirectory conformance.'),
            'import',
            array(
              new FileAttribute (
                '', '',
                'import_file', FALSE
              ),
              new ButtonAttribute (
                '', '',
                'import_submit', FALSE,
                _('Import'), ''
              )
            ),
            '', '%s%s', '',
            _('Import LDIF file')
          )
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->ui = get_userinfo();

    $this->attributesAccess['single_export']->setInLdap(FALSE);
    $this->attributesAccess['single_export']->setLinearRendering(TRUE);
    $this->attributesAccess['complete_export']->setInLdap(FALSE);
    $this->attributesAccess['complete_export']->setLinearRendering(TRUE);
    $this->attributesAccess['import']->setInLdap(FALSE);
    $this->attributesAccess['import']->setLinearRendering(TRUE);
    $this->attributesAccess['overwrite']->setInLdap(FALSE);
    $this->attributesAccess['export_filter']->setInLdap(FALSE);
  }

  function save_object()
  {
    parent::save_object();
    if (isset($_POST[get_class($this).'_posted'])) {
      $this->save_export();
      $this->save_import();
    }
  }

  function save_export()
  {
    $dn = '';
    if (isset($_POST['single_submit'])) {
      $dn     = $this->single_export;
      $acl    = $this->attributesAccess['single_export']->getAcl();
      $scope  = 'base';
      $name   = 'entryExport.ldif';
      if (!$ldap->dn_exists($dn)) {
        msg_dialog::display(_('LDAP error'),
            sprintf(_('No such object %s!'), LDAP::fix($dn)),
            ERROR_DIALOG);
        $dn = '';
      }
    } elseif (isset($_POST['complete_submit'])) {
      $dn     = $this->complete_export;
      $acl    = $this->attributesAccess['complete_export']->getAcl();
      $scope  = 'sub';
      $name   = 'fullExport.ldif';
    }

    if (!empty($dn)) { // An LDIF export was asked
      // Check permissions
      if (!$this->acl_is_writeable($acl, $this->acl_skip_write())) {
        msg_dialog::display(_('Permission error'),
            sprintf(_('You have no permission to export %s!'), bold(LDAP::fix($dn))),
            ERROR_DIALOG);
      } else {
        $ldap = $this->config->get_ldap_link();
        $data = $ldap->generateLdif(LDAP::fix($dn), $this->export_filter, $scope);
        if ($data === NULL) {
          msg_dialog::display(_('Error'),
              sprintf(_('Failed to generate ldap export, error was "%s"!'), $ldap->get_error()),
              ERROR_DIALOG);
        } else {
          send_binary_content($data, $name);
        }
      }
    }
  }

  function save_import()
  {
    if (isset($_POST['import_submit'])) {
      if ($this->acl_is_writeable($this->attributesAccess['import']->getAcl(), $this->acl_skip_write())) {
        $ldap   = $this->config->get_ldap_link();
        try {
          $ldap->import_complete_ldif($this->import, !$this->overwrite, FALSE);
        } catch(Exception $e) {
          msg_dialog::display(_('LDAP error'), $e->getMessage(), ERROR_DIALOG);
        }
      }
    }
  }

  function save()
  {
  }

  function get_allowed_bases()
  {
    return $this->config->idepartments;
  }
}

?>
