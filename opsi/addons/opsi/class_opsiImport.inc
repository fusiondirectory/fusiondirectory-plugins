<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011-2013 FusionDirectory project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class opsiImport extends systemImport
{
  protected $defaultType = 'winstation';

  static function getAttributesInfo($filter = 'objectClass=opsiServer')
  {
    return parent::getAttributesInfo($filter);
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI import'),
      'plDescription' => _('Import windows stations from OPSI into FD'),
      'plIcon'        => 'plugins/opsi/images/icon.png',
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('opsiImport' => array('name' => _('OPSI import'), 'filter' => '')),
      'plSection'     => 'addons',
      'plPriority'    => 6,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  protected function getExtraTabs($server_dn)
  {
    return array(
      'opsiClient' => array(
        'is_account'      => TRUE,
        'fdOpsiServerDn'  => $server_dn,
      )
    );
  }

  protected function getHosts ($server_dn)
  {
    global $config;
    $s_daemon = new supportDaemon();
    if (!$s_daemon->is_available()) {
      msg_dialog::display(
        _("Argonaut server is not available"), $s_daemon->get_error(), ERROR_DIALOG
      );
      return FALSE;
    }
    $ldap = $config->get_ldap_link();
    $ldap->cat($server_dn);
    if ($attrs = $ldap->fetch()) {
      if (isset($attrs['macAddress'])) {
        $macAddress = $attrs['macAddress'][0];
      } else {
        msg_dialog::display(
          _("No mac address"),
          _("Server $server_dn has no mac address configured in the LDAP"), ERROR_DIALOG
        );
        return FALSE;
      }
    } else {
      msg_dialog::display(
        _("Could not find $server_dn in the LDAP"),
        msgPool::ldaperror($ldap->get_error(), $server_dn), ERROR_DIALOG
      );
      return FALSE;
    }
    $res = $s_daemon->append_call("OPSI.host_getObjects", array($macAddress), array('args' => array(array(), array('type' => 'OpsiClient'))));
    if ($s_daemon->is_error()) {
      msg_dialog::display(
        _("Could not update OPSI informations"),
        msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
      );
      return FALSE;
    }
    return $res;
  }
}
?>
