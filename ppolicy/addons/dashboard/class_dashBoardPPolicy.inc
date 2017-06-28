<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2014-2015  Dhatim
  Copyright (C) 2014-2016  FusionDirectory project

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
      'locked_accounts' => array(
        'name'  => _('Locked accounts'),
        'attrs' => array(new FakeAttribute('locked')),
        'template' => get_template_path('ppolicy_locked_accounts.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->expiredAccountsColumns = array(
      'user' => array(
        'uid'             => _('Login'),
        'cn'              => _('Name'),
        'telephoneNumber' => _('Phone number'),
        'expirationDate'  => _('Expiration date'),
      ),
      'manager' => array(
        'manager_cn'      => _('Name'),
        'manager_mail'    => _('Email'),
        'manager_phone'   => _('Phone number'),
      )
    );

    $this->compute_accounts_info();
  }

  function compute_accounts_info ()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $defaultMaxAge = NULL;
    $ppolicydn = $config->get_cfg_value('ppolicyDefaultCn', '');
    if (!empty($ppolicydn)) {
      $ppolicydn = 'cn='.$ppolicydn.','.get_ou('ppolicyRDN').$config->current['BASE'];
      $ldap->cat($ppolicydn, array('pwdMaxAge'));
      $policy = $ldap->fetch();
      if (!$policy) {
        msg_dialog::display(
          _('Configuration error'),
          sprintf(_('Default ppolicy "%s" could not be found in the LDAP!'), $ppolicydn),
          ERROR_DIALOG
        );
      } elseif (isset($policy['pwdMaxAge'][0])) {
        $defaultMaxAge = $policy['pwdMaxAge'][0];
      }
    }

    /* Fetch global value from configuration */
    $next_expired_days  = $config->get_cfg_value('dashboardExpiredAccountsDays', 15);
    /* Convert it to seconds */
    $next_expired_days_seconds = $next_expired_days * 24 * 60 * 60;
    /* Ppolicy stores all dates in UTC */
    $now = new DateTime('now', timezone::utc());

    /* search all expired accounts */
    $users = objects::ls('user', array(
        'dn'                    => 'raw',
        'uid'                   => 'raw',
        'cn'                    => 'raw',
        'mail'                  => 'raw',
        'telephoneNumber'       => 'raw',
        'manager'               => 'raw',
        'pwdChangedTime'        => 1,
        'pwdPolicySubentry'     => 1,
        'pwdAccountLockedTime'  => 1,
      ), NULL, '(|(pwdAccountLockedTime=*)(pwdChangedTime=*))');
    $locked_users           = array();
    $expired_accounts       = array();
    $next_expired_accounts  = array();
    // ppolicies cache
    $maxAges = array();
    foreach ($users as $user) {
      if (isset($user['pwdAccountLockedTime'])) {
        $locked_user  = dashboardUsers::get_user_infos($user);
        $lockedTime   = LdapGeneralizedTime::fromString($user['pwdAccountLockedTime']);
        $lockedTime->setTimezone(timezone::getDefaultTimeZone());
        $locked_user['pwdAccountLockedTime']  = $lockedTime->format('Y-m-d H:i:s');
        $locked_users[]                       = $locked_user;
      }
      if (!isset($user['pwdChangedTime'])) {
        continue;
      }
      $maxAge = NULL;
      if (isset($user['pwdPolicySubentry'])) {
        if (isset($maxAges[$user['pwdPolicySubentry']])) {
          $maxAge = $maxAges[$user['pwdPolicySubentry']];
        } else {
          $ldap->cat($user['pwdPolicySubentry'], array('pwdMaxAge'));
          $policy = $ldap->fetch();
          if (!$policy) {
            msg_dialog::display(
              _('Configuration error'),
              sprintf(_('Ppolicy "%s" set for user "%s" could not be found in the LDAP!'), $ppolicydn, $user['dn']),
              ERROR_DIALOG
            );
            continue;
          }
          if (isset($policy['pwdMaxAge'][0])) {
            $maxAge = $policy['pwdMaxAge'][0];
          }
          $maxAges[$user['pwdPolicySubentry']] = $maxAge;
        }
      } elseif ($defaultMaxAge !== NULL) {
        $maxAge = $defaultMaxAge;
      }
      if (($maxAge === NULL) || ($maxAge <= 0)) {
        /* No max age, it can’t be expired */
        continue;
      }
      /* Build expiration date from pwdChangedTime and max age */
      $expDate = LdapGeneralizedTime::fromString($user['pwdChangedTime']);
      $expDate->setTimezone(timezone::utc());
      $expDate->add(new DateInterval('PT'.$maxAge.'S'));
      if ($expDate->getTimeStamp() < $now->getTimeStamp()) {
        $expired_account = dashboardUsers::get_user_infos($user);
        $expired_account['expirationDate'] = $expDate->format('d.m.Y');
        $expired_accounts[] = $expired_account;
      } elseif ($expDate->getTimeStamp() < ($now->getTimeStamp() + $next_expired_days_seconds)) {
        $expired_account = dashboardUsers::get_user_infos($user);
        $expired_account['expirationDate'] = $expDate->format('d.m.Y');
        $next_expired_accounts[] = $expired_account;
      }
    }

    $this->expired = array(
      'columns'             => $this->expiredAccountsColumns,
      'accounts'            => $expired_accounts,
      'accounts_next_days'  => $next_expired_accounts,
      'next_days'           => $next_expired_days,
    );
    $this->locked = array(
      'accounts'  => $locked_users,
    );
  }
}
?>
