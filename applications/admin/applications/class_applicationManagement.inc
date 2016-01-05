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

class applicationManagement extends simpleManagement
{
  protected $objectTypes  = array('application', 'webApplication');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Applications'),
      'plDescription' => _('Application management'),
      'plIcon'        => 'geticon.php?context=categories&icon=applications-other&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 60,
      'plManages'     => array('application', 'webApplication'),

      'plMenuProvider'  => TRUE,
      'plProvidedAcls'  => array()
    );
  }

  static function getMenuEntries()
  {
    global $config, $ui;
    $mode = $config->get_cfg_value('WebappsMenu', 'none');
    if ($mode == 'none') {
      return array(array(), array());
    }
    $entries = array();
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
      $roles  = objects::ls(
        'role',
        array('fdApplicationAllowed' => '*'),
        NULL,
        '(&(objectClass=fdApplicationRights)(roleOccupant='.$ui->dn.'))'
      );
      $ldap = $config->get_ldap_link();
      foreach ($roles as $role) {
        foreach ($role['fdApplicationAllowed'] as $appdn) {
          if (!isset($apps[$appdn])) {
            $ldap->cat($appdn);
            if ($ldap->count()) {
              $apps[$appdn] = $ldap->fetch();
            }
          }
        }
      }
    }
    foreach ($apps as $app) {
      if (isset($app['fdApplicationImage'])) {
        $path = 'getbin.php?key=webapp_'.urlencode($app['cn'][0]);
        session::set('binarywebapp_'.$app['cn'][0].'type', 'image/png');
        session::set('binarywebapp_'.$app['cn'][0], $app['fdApplicationImage'][0]);
      } elseif (isset($app['fdApplicationImageLocation'])) {
        $path = $app['fdApplicationImageLocation'][0];
      } else {
        $path = 'plugins/applications/images/default_icon.png';
      }
      $entries[] = array(
        'NAME'        => $app['cn'][0],
        'TITLE'       => $app['fdApplicationTitle'][0],
        'DESCRIPTION' => $app['description'][0],
        'LINK'        => $app['labeledURI'][0],
        'ICONPATH'    => $path,
      );
    }
    return array(
      array(
        'webapps' => array('name' => _('Web applications'), 'priority' => 100)
      ),
      array(
        'webapps' => $entries
      )
    );
  }
}
?>
