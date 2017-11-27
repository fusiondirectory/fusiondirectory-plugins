<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2017  FusionDirectory

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
  var $products;

  static function plInfo ()
  {
    return array(
      'plShortName'   => _('OPSI client'),
      'plDescription' => _('Edit OPSI client settings'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('workstation', 'ogroup-dynamic'),
      'plDepends'     => array('sambaSystemTab'),
      'plConflicts'   => array('debconfStartup','faiStartup'),

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
      ),
      'products' => array(
        'name'  => _('Softwares'),
        'attrs' => array(
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('The localboot products to setup on this host'),
              'fdOpsiLocalbootProduct',
              array(
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_product', TRUE
                ),
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_action', TRUE,
                  array('setup', 'always', 'once', 'custom', 'userlogin', 'update')
                )
              ),
              '|', '', _('Localboot products')
            ),
            FALSE, // non-ordered
            array(),
            TRUE // edition
          ),
        )
      ),
      'infos' => array(
        'name'  => _('Information'),
        'attrs' => array(
          new DisplayAttribute(
            _('Last seen'), _('Last time this OPSI client was seen by the OPSI server'),
            'lastSeen', FALSE
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    if ($object instanceof ogroup) {
      $this->initialMembers = array();
      if (isset($object->saved_attributes['member'])) {
        $this->initialMembers = $object->saved_attributes['member'];
        if (!is_array($this->initialMembers)) {
          $this->initialMembers = array($this->initialMembers);
        }
      }
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->setSubmitForm('product_changed');
    $this->attributesAccess['lastSeen']->setAllowHTML(TRUE);
  }

  function save()
  {
    global $config;
    if ($this->initially_was_account && $this->editing_group() && !$this->is_template) {
      $oldMembers = array_diff($this->initialMembers, $this->parent->getBaseObject()->member);
      $macAddresses = array();
      $ldap = $config->get_ldap_link();
      foreach ($oldMembers as $member) {
        $ldap->cat($member, array('macAddress'), '(&(!(objectClass=opsiClient))(macAddress=*))');
        if ($attrs = $ldap->fetch()) {
          $macAddresses[] = $attrs['macAddress'][0];
        }
      }
      if (!empty($macAddresses)) {
        $ldap->cat($this->fdOpsiServerDn, array('macAddress'));
        if ($attrs = $ldap->fetch()) {
          $serverMac = $attrs['macAddress'][0];
        } else {
          msg_dialog::display(
            _('Could not get macAddress for OPSI server!'),
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

    if ($this->is_account && !$this->is_template) {
      $this->update_or_insert();
    }
  }

  function getMacAddresses()
  {
    global $config;
    $macAddresses = array();
    if ($this->editing_group()) {
      $ldap = $config->get_ldap_link();
      foreach ($this->parent->getBaseObject()->member as $member) {
        $ldap->cat($member, array('macAddress'), '(macAddress=*)');
        if ($attrs = $ldap->fetch()) {
          $macAddresses[] = $attrs['macAddress'][0];
        } else {
          if ($ldap->success()) {
            $error = _('This member was not found or has no macAddress');
          } else {
            $error = $ldap->get_error();
          }
          msg_dialog::display(
            sprintf(_('Could not get macAddress for "%s"'), $member),
            msgPool::ldaperror($error), ERROR_DIALOG
          );
        }
      }
    } else {
      $macAddresses = $this->parent->getBaseObject()->macAddress;
      if (!is_array($macAddresses)) {
        $macAddresses = array($macAddresses);
      }
    }
    return $macAddresses;
  }

  function update_or_insert()
  {
    global $config;
    $s_daemon     = new supportDaemon();
    $macAddresses = $this->getMacAddresses();
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
    if ($this->member_of_group && !$this->is_template) {
      // If the winstation is member of an ogroup which has OPSI activated, we must update_or_insert information
      // We have to call this in here because save won't be called as is_account is FALSE
      $this->update_or_insert();
    }

    if (!$this->initially_was_account) {
      return;
    }

    if (!$this->is_template) {
      $macAddresses = array();
      if ($this->editing_group()) {
        $ldap = $config->get_ldap_link();
        foreach ($this->initialMembers as $member) {
          $ldap->cat($member, array('macAddress'), '(&(!(objectClass=opsiClient))(macAddress=*))');
          if ($attrs = $ldap->fetch()) {
            $macAddresses[] = $attrs['macAddress'][0];
          }
        }
      } elseif (!$this->member_of_group) {
        $macAddresses = array($this->parent->getBaseObject()->macAddress);
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

    $s_daemon = new supportDaemon();
    if (!$s_daemon->is_available()) {
      msg_dialog::display(
        _('Could not contact argonaut server'),
        msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
      );
      return;
    }
    $ldap->cat($this->fdOpsiServerDn);
    if ($attrs = $ldap->fetch()) {
      if (isset($attrs['macAddress'][0])) {
        $macAddress = $attrs['macAddress'][0];
      } else {
        msg_dialog::display(
          _('No mac address'),
          sprintf(_('Server %s has no mac address configured in the LDAP'), $this->fdOpsiServerDn), ERROR_DIALOG
        );
        return FALSE;
      }
    } else {
      msg_dialog::display(
        sprintf(_('Could not find %s in the LDAP'), $this->fdOpsiServerDn),
        msgPool::ldaperror($ldap->get_error()), ERROR_DIALOG
      );
      return FALSE;
    }
    $infos = $s_daemon->append_call('OPSI.host_getObjects', $macAddress, array('args' => array(array('lastSeen', 'hardwareAddress'), array('hardwareAddress' => $this->getMacAddresses()))));
    if ($s_daemon->is_error()) {
      $this->lastSeen = sprintf(_('Failed to contact OPSI server: %s'), $s_daemon->get_error());
    } elseif (count($infos) == 0) {
      $this->lastSeen = _('Never');
    } else {
      $text = '';
      foreach ($infos as $info) {
        $text .= htmlentities(sprintf('%s - %s', $info['id'], $info['lastSeen']), ENT_COMPAT, 'UTF-8')."<br/>\n";
      }
      $this->lastSeen = $text;
    }
    if (!isset($this->products[$this->fdOpsiServerDn])) {
      $opsi_args  = array('id','name','setupScript','alwaysScript','onceScript','customScript','userLoginScript','updateScript','productVersion','packageVersion');
      $localboots = $s_daemon->append_call('OPSI.get_localboots', $macAddress, array('args' => array($opsi_args)));
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          _('Failed to contact OPSI server'),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
        return;
      }
      $this->products[$this->fdOpsiServerDn]['localboots'] = array();
      foreach ($localboots as $localboot) {
        $this->products[$this->fdOpsiServerDn]['localboots'][$localboot['id']] = $localboot;
      }
    }
    if ($s_daemon->is_error()) {
      msg_dialog::display(
        _('Failed to contact OPSI server'),
        msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
      );
      return;
    }
    $choices = array();
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

    $actions = array();
    foreach (array('setup', 'always', 'once', 'custom', 'userLogin', 'update') as $action) {
      if (!empty($this->products[$this->fdOpsiServerDn]['localboots'][$localboot][$action.'Script'])) {
        $actions[] = strtolower($action);
      }
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[1]->setChoices($actions);
  }
}

?>
