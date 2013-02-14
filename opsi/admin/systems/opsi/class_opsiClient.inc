<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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

  static function plInfo ()
  {
    return array(
      'plShortName'   => _('OPSI client'),
      'plDescription' => _('Edit OPSI client settings'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('winstation','ogroups'),
      'plObjectType'  => array('winstation','ogroup'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('OPSI Client'),
        'attrs' => array(
          new SelectAttribute(
            _('OPSI Server'), _('The OPSI Server this client is connected to'),
            'fdOpsiServerDn', TRUE
          )
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search('(objectClass=opsiServer)', array('cn'));
    $serversdn    = array();
    $serverslabel = array();
    while ($attrs = $ldap->fetch()) {
      $serversdn[]    = $attrs['dn'];
      $serverslabel[] = $attrs['cn'][0];
    }
    $this->attributesAccess['fdOpsiServerDn']->setChoices($serversdn, $serverslabel);
  }

  function save()
  {
    parent::save();

    if ($this->is_account) {
      $s_daemon   = new supportDaemon();
      $macAddress = $this->parent->getBaseObject()->netConfigDNS->macAddress;
      if ($s_daemon->is_available()) {
        $res = $s_daemon->append_call("OPSI.update_or_insert", array($macAddress), array());
        if ($s_daemon->is_error()) {
          msg_dialog::display(
            _("Could not update OPSI informations"),
            msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
          );
        }
      }
    }
  }

  function remove_from_parent()
  {
    $s_daemon   = new supportDaemon();
    $macAddress = $this->parent->getBaseObject()->netConfigDNS->macAddress;
    if ($s_daemon->is_available()) {
      $res = $s_daemon->append_call("OPSI.delete", array($macAddress), array());
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          _("Could not update OPSI informations"),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
      }
    }

    parent::remove_from_parent();
  }
}

?>
