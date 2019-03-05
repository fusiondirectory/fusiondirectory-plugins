<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2010 Antoine Gallavardin
  Copyright (C) 2011-2018 FusionDirectory project

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

class opsiImport extends systemImport
{
  protected $types = array('workstation');

  /* Return plugin information for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI import'),
      'plDescription' => _('Import windows stations from OPSI into FD'),
      'plIcon'        => 'geticon.php?context=applications&icon=opsi&size=48',
      'plSelfModify'  => FALSE,
      'plObjectType'  => array(
        'opsiImport' => array(
          'name'      => _('OPSI import'),
          'tabClass'  => 'simpleTabs_noSpecial',
          'mainAttr'  => FALSE,
        )
      ),
      'plSection'     => 'conf',
      'plPriority'    => 15,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo($filter = 'objectClass=opsiServer')
  {
    $attributesInfo = parent::getAttributesInfo($filter);
    /* Add our checkbox just before the import button */
    array_splice(
      $attributesInfo['import']['attrs'],
      -1,
      0,
      array(
        new BooleanAttribute(
          _('Import localboot products'), _('Import localboot product list configured in OPSI in the FD OPSI tab'),
          'importLocalboots', FALSE,
          FALSE
        )
      )
    );
    return $attributesInfo;
  }

  protected function getExtraTabs($server_dn, $host)
  {
    $extraTabs = array(
      'sambaSystemTab' => array(
        'is_account'      => TRUE,
      ),
      'opsiClient' => array(
        'is_account'      => TRUE,
        'fdOpsiServerDn'  => $server_dn,
      )
    );

    if ($this->importLocalboots) {
      if (!empty($host['localboots'])) {
        $extraTabs['opsiClient']['fdOpsiLocalbootProduct'] = $host['localboots'];
      }
    }

    return $extraTabs;
  }

  protected function getHosts ($server_dn)
  {
    global $config;
    $s_daemon = new supportDaemon();
    if (!$s_daemon->is_available()) {
      msg_dialog::display(
        _('Argonaut server is not available'), $s_daemon->get_error(), ERROR_DIALOG
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
          _('No mac address'),
          sprintf(_('Server "%s" has no mac address configured in the LDAP'), $server_dn),
          ERROR_DIALOG
        );
        return FALSE;
      }
    } else {
      msg_dialog::display(
        sprintf(_('Could not find "%s" in the LDAP'), $server_dn),
        msgPool::ldaperror($ldap->get_error(), $server_dn), ERROR_DIALOG
      );
      return FALSE;
    }
    $hosts = $s_daemon->append_call('OPSI.host_getObjects', $macAddress, array('args' => array(array(), array('type' => 'OpsiClient'))));
    if ($s_daemon->is_error()) {
      msg_dialog::display(
        _('Could not get OPSI information'),
        msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
      );
      return FALSE;
    }

    if ($this->importLocalboots) {
      foreach ($hosts as &$host) {
        $res = $s_daemon->append_call(
          'OPSI.productOnClient_getObjects',
          $macAddress,
          array('args' => array(
            array(),
            array(
              'clientId'    => $host['id'],
              'type'        => 'ProductOnClient',
              'productType' => 'LocalbootProduct',
            )
          ))
        );
        if ($s_daemon->is_error()) {
          msg_dialog::display(
            _('Could not get OPSI information'),
            msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
          );
          return FALSE;
        }
        $host['localboots'] = array();
        foreach ($res as $productOnClient) {
          if ($productOnClient['productId'] == 'opsi-winst') {
            /* opsi-winst cannot be installed and has no actions */
            continue;
          }
          if ($productOnClient['actionRequest'] != 'none') {
            $host['localboots'][] = $productOnClient['productId'].'|'.$productOnClient['actionRequest'];
          } elseif ($productOnClient['installationStatus'] == 'installed') {
            $host['localboots'][] = $productOnClient['productId'].'|setup';
          }
        }
      }
      unset($host);
    }

    return $hosts;
  }
}
?>
