<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2015-2019  FusionDirectory

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

class dnsManagement extends management
{
  public $neededAttrs = array('sOARecord' => 1);

  /* Default columns */
  public static $columns = array(
    array('ObjectTypeColumn',     array()),
    array('LinkColumn',           array('attributes' => 'nameAttr',             'label' => 'Name')),
    array('ActionsColumn',        array('label' => 'Actions')),
  );

  static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS'),
      'plTitle'       => _('DNS Management'),
      'plDescription' => _('Manage DNS zones and views'),
      'plIcon'        => 'geticon.php?context=applications&icon=dns&size=48',
      'plSection'     => array('systems' => array('name' => _('Systems'), 'priority' => 10)),
      'plPriority'    => 1,
      'plManages'     => array('dnsZone', 'dnsView', 'dnsAcl'),
    );
  }

  protected function configureActions()
  {
    parent::configureActions();

    if (class_available('supportDaemon')) {
      $this->registerAction(
        new Action(
          'zonerefresh',
          _('Refresh Zone'),
          'geticon.php?context=actions&icon=view-refresh&size=16',
          '+',
          'ldap2zoneRefresh',
          array('w'),
          TRUE,
          TRUE,
          array('dnsZone')
        )
      );
      $this->actions['zonerefresh']->setSeparator(TRUE);
    }
  }

  function ldap2zoneRefresh ($action)
  {
    foreach ($action['targets'] as $zoneDn) {
      $entry = $this->listing->getEntry($zoneDn);
      if (strcasecmp($entry->type, 'dnsZone') != 0) {
        /* Only refresh zones */
        throw new FusionDirectoryException(sprintf(_('Invalid target "%s" passed for action "%s"'), $zoneDn, _('Refresh Zone')));
      }
      list ($fqdn) = explode(' ', $entry['sOARecord']);
      $servers = static::findServerByFQDN($fqdn, $zoneDn);
      if (count($servers) > 1) {
        msg_dialog::display(_('Could not run ldap2zone'), _('More than one server matches the SOA'), ERROR_DIALOG);
      } elseif (count($servers) == 0) {
        msg_dialog::display(_('Could not run ldap2zone'), sprintf(_('Could not find the primary server "%s"'), $fqdn), ERROR_DIALOG);
      } else {
        $serverTabs = objects::open(key($servers), 'server');
        if ($serverTabs->by_object['argonautClient']->is_account) {
          $s_daemon = new supportDaemon();
          if ($s_daemon->is_error()) {
            msg_dialog::display(_('Could not run ldap2zone'), msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
            );
          } else {
            $target   = $serverTabs->getBaseObject()->macAddress;
            if (is_array($target)) {
              $target = $target[0];
            }
            $zoneName = $entry['zoneName'];
            $s_daemon->append_call('Ldap2Zone.start', $target, array('args' => array($zoneName)));
            if ($s_daemon->is_error()) {
              msg_dialog::display(_('Could not get run ldap2zone'), msgPool::siError($s_daemon->get_error()), ERROR_DIALOG);
            } else {
              msg_dialog::display(_('Ldap2zone'), sprintf(_('Ldap2Zone called for zone "%s"'), $zoneName), INFO_DIALOG);
            }
          }
        } else {
          msg_dialog::display(_('Error'), _('Argonaut client needs to be activated to use ldap2zone remotely'), ERROR_DIALOG);
        }
      }
    }
  }

  public static function findServerByFQDN($fqdn, $zoneDn = NULL)
  {
    global $config;
    list ($serverCn, $serverZone) = explode('.', $fqdn, 2);
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ips = array();
    $ldap->search('(&(|(aRecord=*)(aAAARecord=*))(relativeDomainName='.$serverCn.')(zoneName='.$serverZone.'))', array('aRecord', 'aAAARecord'));
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['aRecord'])) {
        unset($attrs['aRecord']['count']);
        $ips = array_merge($ips, $attrs['aRecord']);
      }
      if (isset($attrs['aAAARecord'])) {
        unset($attrs['aAAARecord']['count']);
        $ips = array_merge($ips, $attrs['aAAARecord']);
      }
    }
    if (!empty($ips)) {
      $filter = '(|(ipHostNumber='.implode(')(ipHostNumber=', $ips).'))';
      if ($zoneDn !== NULL) {
        $filter = '(&'.$filter.'(fdDNSZoneDn='.ldap_escape_f($zoneDn).'))';
      }
      return objects::ls('server', NULL, NULL, $filter);
    }
    return array();
  }
}
