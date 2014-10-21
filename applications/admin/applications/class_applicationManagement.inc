<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2014  FusionDirectory

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
      'plSection'     => 'admin',
      'plPriority'    => 21,
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
      $apps = objects::ls(
        'webApplication',
        array(
          'cn'                          => '1',
          'description'                 => '1',
          'labeledURI'                  => '1',
          'fdApplicationTitle'          => '1',
          'fdApplicationImageLocation'  => '1',
          'fdApplicationImage'          => '1',
        )
      );
    } else {
      $roles = objects::ls(
        'role',
        array('fdApplicationAllowed' => '*'),
        NULL,
        '(&(objectClass=fdApplicationRights)(roleOccupant='.$ui->dn.'))'
      );
      $ldap = $config->get_ldap_link();
      foreach ($roles as $role) {
        foreach ($role['fdApplicationRights'] as $appdn) {
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
      $entries[] = array(
        'NAME'        => $app['cn'],
        'TITLE'       => $app['fdApplicationTitle'],
        'DESCRIPTION' => $app['description'],
        'LINK'        => $app['labeledURI'],
        'ICONPATH'    => $app['fdApplicationImageLocation'],
      );
    }
    return array(
      array(
        'webapps' => array('name' => _('Web applications'))
      ),
      array(
        'webapps' => $entries
      )
    );
  }
}
?>
