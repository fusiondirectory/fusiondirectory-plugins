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

class Game
{
  private $config;
  private $missions;
  private $mission;

  private function __construct($config, $missions)
  {
    $this->config   =& $config;
    $this->missions = $missions;
    $this->mission  = 0;
  }

  private function _run()
  {
    $smarty = get_smarty();
    $i = $this->mission;
    $end    = FALSE;
    $smarty->assign('success', FALSE);
    while (($i < count($this->missions)) && $this->missions[$i]->validate()) {
      $i++;
      $this->mission = $i;
      $smarty->assign('success', TRUE);
      if ($i < count($this->missions)) {
        $this->missions[$i]->init();
      }
    }
    if ($i >= count($this->missions)) {
      self::stop();
    } else {
      $smarty->assign('mission_number', $i);
      $smarty->assign('mission', $this->missions[$i]->compute_infos());
    }
    return $smarty->fetch(get_template_path('game.tpl', TRUE, dirname(__FILE__)));
  }

  static function run()
  {
    global $config;
    if (isset($_GET['start_game'])) {
      $instance = new Game($config, self::beatles_scenario());
      session::global_set('game_instance', $instance);
    }
    if (session::global_is_set('game_instance')) {
      $instance = session::global_get('game_instance');
      return $instance->_run();
    }
  }

  static function stop()
  {
    session::global_un_set('game_instance');
  }

  static function beatles_scenario() {
    return array(
      new Mission(_('John Lennon has arrived'),
        array(
          'Game::john_created' => _('Create a user for John Lennon with login <b>john</b>')
        ), ''
      ),
      new Mission(_('The Beatles are here'),
        array(
          'Game::template_created'  => _('Create a user template named <b>beatle</b>, using <i>http://%l|givenName%.com</i> as homepage and <i>%l|sn%</i> (lower case first name) as password'),
          'Game::paul_created'      => _('Create a user for Paul McCartney using beatle template (login paul)'),
          'Game::george_created'    => _('Create a user for George Harrison using beatle template (login george)'),
          'Game::ringo_created'     => _('Create a user for Ringo Starr using beatle template (login ringo)'),
          'Game::beatles_created'   => _('Create a POSIX group for The Beatles named <b>beatles</b> and add the 4 beatles to this group'),
        ), ''
      ),
      /* new Mission(_('The Beatles hired a manager'),
        array(
          'Game::manager_created'     => _('Create a user for the manager with login <b>manager</b>'),
          'Game::manager_rights'      => _('Give this user full rights on users'),
          'Game::manager_login'       => _('Log out and log in using the manager account'),
          'Game::beatles_management'  => _('Set the birth date of John to 1940-10-09'),
        )
      ),*/
      new Mission(_('The Beatles bought computers'),
        array(
          'Game::submarine_created' => _('Create a server named <b>yellow-submarine</b> with IP 192.168.1.129 and MAC 00:11:22:33:44:01'),
          'Game::dns_created'       => _('Add a DNS service for zone <b>beatles.fake</b> on this server'),
          'Game::dhcp_created'      => _('Add a DHCP service for the network <b>192.168.1.128/26</b> on this server'),
          'Game::station_created'   => _('Create a workstation for John\'s laptop, with IP 192.168.1.130 and MAC 00:11:22:33:44:02'),
          'Game::windows_created'   => _('Create a winstation for George\'s laptop, as he is running windows, with IP 192.168.1.131 and MAC 00:11:22:33:44:03'),
        ), ''
      ),
    );
  }

  static function john_created()
  {
    return self::user_created('john');
  }

  static function paul_created()
  {
    return self::user_created('paul');
  }

  static function george_created()
  {
    return self::user_created('george');
  }

  static function ringo_created()
  {
    return self::user_created('ringo');
  }

  static function user_created($uid)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd ($config->current['BASE']);
    $ldap->search ('(&(objectClass=inetOrgPerson)(uid='.$uid.'))', array('uid'));
    if ($ldap->fetch()) {
      return TRUE;
    }
    return FALSE;
  }

  static function template_created()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd ($config->current['BASE']);
    $ldap->search ('(&(objectClass=fdTemplate)(cn=beatle))', array('fdTemplateField'));
    if ($attrs = $ldap->fetch()) {
      return (in_array('labeledURI:http://%l|givenName%.com', $attrs['fdTemplateField']));
    }
    return FALSE;
  }

  static function beatles_created()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd ($config->current['BASE']);
    $ldap->search ('(&(objectClass=posixGroup)(cn=beatles))', array('cn', 'memberUid'));
    if ($attrs = $ldap->fetch()) {
      unset($attrs['memberUid']['count']);
      return (count(array_diff($attrs['memberUid'], array('john', 'paul', 'george', 'ringo'))) == 0);
    }
    return FALSE;
  }

  static function submarine_created()
  {
    return self::system_created('goServer', 'yellow-submarine','192.168.1.129','00:11:22:33:44:01');
  }

  static function station_created()
  {
    return self::system_created('gotoWorkstation', '*','192.168.1.130','00:11:22:33:44:02');
  }

  static function windows_created()
  {
    return self::system_created('sambaSamAccount', '*','192.168.1.131','00:11:22:33:44:03');
  }

  static function system_created($class, $cn, $ip, $mac)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd (get_ou('systemRDN').$config->current['BASE']);
    $ldap->search ('(&(objectClass='.$class.')(cn='.$cn.')(ipHostNumber='.$ip.')(macAddress='.$mac.'))', array('cn'));
    if ($attrs = $ldap->fetch()) {
      return TRUE;
    }
    return FALSE;
  }

  static function dhcp_created ()
  {
    return self::system_created('dhcpServer', 'yellow-submarine','192.168.1.129','00:11:22:33:44:01');
  }

  static function dns_created ()
  {
    $cn = 'yellow-submarine';
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd (get_ou('systemRDN').$config->current['BASE']);
    $ldap->search ('(&(objectClass=goServer)(cn='.$cn.'))', array('cn'));
    if (($attrs = $ldap->fetch()) && (class_available('serviceDNS'))) {
      $dns = new serviceDNS($config, $attrs['dn']);
      if ($dns->is_this_account($attrs)) {
        return isset($dns->Zones['beatles.fake.']);
      } else {
        return FALSE;
      }
    }
    return FALSE;
  }
}

?>
