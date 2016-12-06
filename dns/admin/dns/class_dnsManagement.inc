<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016  FusionDirectory

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

class dnsManagement extends simpleManagement
{
  protected $objectTypes  = array('dnsZone', 'dnsView', 'dnsAcl');

  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = TRUE;
  protected $baseMode               = TRUE;

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'zoneName', 'relativeDomainName', 'sOARecord');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS'),
      'plDescription' => _('DNS Management'),
      'plIcon'        => 'geticon.php?context=applications&icon=dns&size=48',
      'plSection'     => array('systems' => array('name' => _('Systems'), 'priority' => 10)),
      'plPriority'    => 1,
      'plManages'     => array('dnsZone', 'dnsView', 'dnsAcl'),

      'plProvidedAcls'  => array()
    );
  }

  function __construct()
  {
    parent::__construct();
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();
    if (class_available('supportDaemon')) {
      $this->registerAction('zonerefresh', 'ldap2zoneRefresh');
      array_splice(
        $this->headpage->xmlData['actionmenu']['action'],
        2,
        0,
        array(
          array(
            'name'  => 'zonerefresh',
            'type'  => 'entry',
            'image' => 'geticon.php?context=actions&icon=view-refresh&size=16',
            'label' => 'Refresh Zone',
            'acl'   => 'dnsZone/dnsZone[w]',
          )
        )
      );
      array_splice(
        $this->headpage->xmlData['actiontriggers']['action'],
        2,
        0,
        array(
          array(
            'name'  => 'zonerefresh',
            'type'  => 'entry',
            'image' => 'geticon.php?context=actions&icon=view-refresh&size=16',
            'label' => 'Refresh Zone',
            'acl'   => '%acl[w]',
          )
        )
      );
    }
  }

  function ldap2zoneRefresh ($action, $targets)
  {
    if (empty($targets)) {
      return;
    }
    foreach ($targets as $zoneDn) {
      $entry = $this->getHeadpage()->getEntry($zoneDn);
      list ($fqdn) = explode(' ', $entry['sOARecord'][0]);
      $zoneName = $entry['zoneName'][0];
      $serverCn = preg_replace('/\.'.preg_quote($zoneName, '/').'$/', '', $fqdn);
      $servers = objects::ls('server', array('cn' => 1, 'macAddress' => 1), NULL, '(&(cn='.ldap_escape_f($serverCn).')(fdDNSZoneDn='.ldap_escape_f($zoneDn).'))');
      if (count($servers) > 1) {
        msg_dialog::display(_('Could not run ldap2zone'), _('More than one server matches the SOA'), ERROR_DIALOG);
      } elseif (count($servers) == 0) {
        msg_dialog::display(_('Could not run ldap2zone'), _('Could not find the primary server'), ERROR_DIALOG);
      } else {
        $serverTabs = objects::open(key($servers), 'server');
        if ($serverTabs->by_object['argonautClient']->is_account) {
          $s_daemon = new supportDaemon();
          if ($s_daemon->is_error()) {
            msg_dialog::display(_('Could not run ldap2zone'), msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
            );
          } else {
            $target = array($serverTabs->getBaseObject()->macAddress);
            $res    = $s_daemon->append_call('Ldap2Zone.start', $target, array('args' => array($zoneName)));
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
}
?>
