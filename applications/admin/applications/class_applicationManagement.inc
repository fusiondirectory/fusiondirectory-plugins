<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2019  FusionDirectory

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

class applicationManagement extends management
{
  static function plInfo ()
  {
    return array(
      'plShortName'   => _('Applications'),
      'plTitle'       => _('Applications management'),
      'plDescription' => _('Manage applications and web applications'),
      'plIcon'        => 'geticon.php?context=categories&icon=applications-other&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 60,
      'plManages'     => array('application', 'webApplication'),

      'plMenuProvider'  => TRUE,
    );
  }

  static function getMenuEntries ()
  {
    global $config, $ui;
    $mode = $config->get_cfg_value('WebappsMenu', 'none');
    if ($mode == 'none') {
      return array(array(), array());
    }
    $ldap = $config->get_ldap_link();
    if ($mode == 'all') {
      /* using raw to have the same results than with allowed mode */
      $apps = objects::ls(
        'webApplication',
        array(
          'cn'                          => 'raw',
          'description'                 => 'raw',
          'labeledURI'                  => 'raw',
          'fdApplicationTitle'          => 'raw',
          'fdApplicationImageLocation'  => 'raw',
          'fdApplicationImage'          => 'raw',
        )
      );
    } else { /* allowed */
      $apps   = array();
      if (is_object($ui) && isset($ui->dn)) {
        $roles  = objects::ls(
          'role',
          array('fdApplicationAllowed' => '*'),
          NULL,
          '(&(objectClass=fdApplicationRights)(roleOccupant='.$ui->dn.'))'
        );
        foreach ($roles as $role) {
          foreach ($role['fdApplicationAllowed'] as $appdn) {
            if (!isset($apps[$appdn])) {
              $ldap->cat($appdn);
              if ($ldap->count()) {
                $attrs = $ldap->fetch();
                if (in_array_ics('fdWebApplication', $attrs['objectClass'])) {
                  $apps[$appdn] = $attrs;
                }
              }
            }
          }
        }
      }
    }
    ksort($apps);
    $sections = array(
      'webapps' => array('name' => _('Web applications'), 'priority' => 100)
    );
    $entries = array(
      'webapps' => array()
    );
    foreach ($apps as $dn => $app) {
      $base = preg_replace('/^[^,]+,'.preg_quote(get_ou('webappsRDN'), '/').'/', '', $dn);
      $section = 'webapps';
      if ($base != $config->current['BASE']) {
        $ldap->cat($base, array('dn', 'ou', 'description'), '(&(ou=*)(description=*))');
        if ($attrs = $ldap->fetch()) {
          $section = $attrs['ou'][0];
          if (!isset($sections[$section])) {
            $sections[$section] = array(
              'name'      => $attrs['description'][0],
              'priority'  => 101
            );
            $entries[$section] = array();
          }
        }
      }
      if (isset($app['fdApplicationImage'])) {
        $path = 'getbin.php?key=webapp_'.urlencode($app['cn'][0]);
        session::set('binarywebapp_'.$app['cn'][0].'type', 'image/png');
        session::set('binarywebapp_'.$app['cn'][0], $app['fdApplicationImage'][0]);
      } elseif (isset($app['fdApplicationImageLocation'])) {
        $path = $app['fdApplicationImageLocation'][0];
      } else {
        $path = 'plugins/applications/images/default_icon.png';
      }
      $name   = $app['cn'][0];
      $title  = (isset($app['fdApplicationTitle'][0]) ? $app['fdApplicationTitle'][0] : $name);
      $entries[$section][] = array(
        'NAME'        => $name,
        'TITLE'       => $title,
        'DESCRIPTION' => (isset($app['description'][0]) ? $app['description'][0] : $title),
        'LINK'        => $app['labeledURI'][0],
        'ICONPATH'    => $path,
      );
    }
    return array(
      $sections,
      $entries
    );
  }
}
