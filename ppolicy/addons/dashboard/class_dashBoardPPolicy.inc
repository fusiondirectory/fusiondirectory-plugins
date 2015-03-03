<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2015  FusionDirectory project
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

class dashboardPpolicy extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Ppolicy'),
      'plDescription' => _('Statistics about ppolicy expired users'),
      'plObjectType'  => array('dashboard'),

      'plProvidedAcls'  => array()
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'expired_accounts' => array(
        'name'  => _('Expired accounts'),
        'attrs' => array(new FakeAttribute('expired')),
        'template' => get_template_path('users_accounts.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->expired  = $this->expired_accounts_info();
  }

  function expired_accounts_info ()
  {
    /* getting the date in TIMESTAMP UNIX format */
    $today        = floor(time() / 86400); // 24 * 60 * 60

    /* Fetch global value from fusiondirectory.conf */
    $next_expired_days = $this->config->get_cfg_value('dashboardExpiredAccountsDays', 15);
    $next_expired_date = ($today + $next_expired_days);

    /* search all locked accounts */
    $locked_users = objects::ls('user', array(
      'uid'                   => 'raw',
      'cn'                    => 'raw',
      'mail'                  => 'raw',
      'telephoneNumber'       => 'raw',
      'manager'               => 'raw',
      'pwdAccountLockedTime'  => 'raw',
    ), NULL, '(pwdAccountLockedTime=*)');

    foreach ($locked_users as &$user) {
      $user = dashboardUsers::get_user_infos($user);
    }
    unset($user);

    return array(
      'accounts'            => $locked_users,
      'accounts_next_days'  => array(),
      'next_days'           => $next_expired_days,
    );
  }
}
?>
