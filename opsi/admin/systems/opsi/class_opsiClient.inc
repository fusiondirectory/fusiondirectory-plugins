<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class opsiClient extends simplePlugin
{
  var $objectclasses  = array('opsiClient');
  var $displayHeader  = TRUE;

  var $inheritance = array('gosaGroupOfNames' => 'member');

  var $initialMembers;

  static function plInfo ()
  {
    return array(
      'plShortName'   => _('OPSI client'),
      'plDescription' => _('Edit OPSI client settings'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('winstation', 'ogroup-dynamic'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('OPSI Client'),
        'attrs' => array(
          new SubmittingOPSIServerAttribute(
            _('OPSI Server'), _('The OPSI Server this client is connected to'),
            'fdOpsiServerDn', TRUE
          ),
          new SelectAttribute(
            _('OPSI Profile'), _('The OPSI Profile to apply to this client'),
            'fdOpsiProfileDn', FALSE
          ),
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    if ($object instanceof ogroup) {
      $this->initialMembers = $object->saved_attributes['member'];
      if (!is_array($this->initialMembers)) {
        $this->initialMembers = array($this->initialMembers);
      }
    }
  }

  function save()
  {
    global $config;
    if ($this->initially_was_account && $this->editing_group()) {
      $oldMembers = array_diff($this->initialMembers, $this->parent->getBaseObject()->member);
      $macAddresses = array();
      $ldap = $config->get_ldap_link();
      foreach ($oldMembers as $member) {
        $ldap->cat($member, array('macAddress'), '(&(!(objectClass=opsiClient))(macAddress=*))');
        if (!$ldap->success()) {
          msg_dialog::display(
            _("Could not get macAddress for $member"),
            msgPool::ldaperror($ldap->get_error()), ERROR_DIALOG
          );
        } elseif ($attrs = $ldap->fetch()) {
          $macAddresses[] = $attrs['macAddress'][0];
        }
      }
      if (!empty($macAddresses)) {
        $ldap->cat($this->fdOpsiServerDn, array('macAddress'));
        if ($attrs = $ldap->fetch()) {
          $serverMac = $attrs['macAddress'][0];
        } else {
          msg_dialog::display(
            _("Could not get macAddress for OPSI server !"),
            msgPool::ldaperror($ldap->get_error()), ERROR_DIALOG
          );
        }
        $s_daemon   = new supportDaemon();
        if ($s_daemon->is_available()) {
          $s_daemon->append_call('OPSI.delete', array($serverMac), array('args' => array($macAddresses)));
          if ($s_daemon->is_error()) {
            msg_dialog::display(
              _('Could not remove OPSI information'),
              msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
            );
          }
        }
      }
    }

    $errors = parent::save();
    if (!empty($errors)) {
      return $errors;
    }

    if ($this->is_account) {
      $this->update_or_insert();
    }
  }

  function update_or_insert()
  {
    global $config;
    $s_daemon   = new supportDaemon();
    $macAddresses = array();
    if ($this->editing_group()) {
      $ldap = $config->get_ldap_link();
      foreach ($this->parent->getBaseObject()->member as $member) {
        $ldap->cat($member, array('macAddress'), '(macAddress=*)');
        if ($attrs = $ldap->fetch()) {
          $macAddresses[] = $attrs['macAddress'][0];
        } else {
          if ($ldap->success()) {
            $error = "$member was not found!";
          } else {
            $error = $ldap->get_error();
          }
          msg_dialog::display(
            _("Could not get macAddress for $member"),
            msgPool::ldaperror($error), ERROR_DIALOG
          );
        }
      }
    } else {
      $macAddresses = array($this->parent->getBaseObject()->netConfigDNS->macAddress);
    }
    if (!empty($macAddresses) && $s_daemon->is_available()) {
      $s_daemon->append_call('OPSI.update_or_insert', $macAddresses, array());
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          _('Could not update OPSI information'),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
      }
    }
  }

  function remove_from_parent()
  {
    global $config;
    if ($this->member_of_group) {
      // If the winstation is member of an ogroup which has OPSI activated, we must update_or_insert information
      // We have to call this in here because save won't be called as is_account is FALSE
      $this->update_or_insert();
    }

    if (!$this->initially_was_account) {
      return;
    }

    $macAddresses = array();
    if ($this->editing_group()) {
      $ldap = $config->get_ldap_link();
      foreach ($this->initialMembers as $member) {
        $ldap->cat($member, array('macAddress'), '(&(!(objectClass=opsiClient))(macAddress=*))');
        if (!$ldap->success()) {
          msg_dialog::display(
            _("Could not get macAddress for $member"),
            msgPool::ldaperror($ldap->get_error()), ERROR_DIALOG
          );
        } elseif ($attrs = $ldap->fetch()) {
          $macAddresses[] = $attrs['macAddress'][0];
        }
      }
    } elseif (!$this->member_of_group) {
      $macAddresses = array($this->parent->getBaseObject()->netConfigDNS->macAddress);
    }

    if (!empty($macAddresses)) {
      $s_daemon   = new supportDaemon();
      if ($s_daemon->is_available()) {
        $s_daemon->append_call('OPSI.delete', $macAddresses, array());
        if ($s_daemon->is_error()) {
          msg_dialog::display(
            _('Could not remove OPSI information'),
            msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
          );
        }
      }
    }

    return parent::remove_from_parent();
  }

  public function server_changed ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->search('(&(objectClass=opsiProfile)(fdOpsiServerDn='.$this->fdOpsiServerDn.'))', array('cn'));
    $profilesdn    = array();
    $profileslabel = array();
    while ($attrs = $ldap->fetch()) {
      $profilesdn[]    = $attrs['dn'];
      $profileslabel[] = $attrs['cn'][0];
    }
    $this->attributesAccess['fdOpsiProfileDn']->setChoices($profilesdn, $profileslabel);
  }
}

?>
