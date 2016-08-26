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

class argonautEventTypes
{
  static protected function get_event_types_list()
  {
    die('get_event_types_list needs to be implemented in all subclass of argonautEventTypes');
  }

  /*! \brief  Returns a complete list of all available events.
    @return   Array   Containing info for all available events.
   */
  static public function get_event_types()
  {
    global $class_mapping;
    if (session::is_set('argonautEventTypes::get_event_types')) {
      return session::get('argonautEventTypes::get_event_types');
    } else {
      $ret = array();
      foreach (array_keys($class_mapping) as $class) {
        if (is_subclass_of($class, 'argonautEventTypes')) {
          $ret = array_merge($ret, $class::get_event_types_list());
        }
      }
      session::set('argonautEventTypes::get_event_types', $ret);
      return $ret;
    }
  }

  /*! \brief  Returns event information, like menu strings, images ...
    @return   Array Event informations.
   */
  static public function get_event_info($action)
  {
    $events = static::get_event_types();
    if (isset($events[$action])) {
      $infos = $events[$action];

      $infos['listimg'] = '<img src="'.$infos['img'].'" title="'.$infos['name'].
                          '"alt="'.$infos['name'].'" class="center"/>';
      return $infos;
    } else {
      return FALSE;
    }
  }
}

class argonautEventSystem extends argonautEventTypes
{
  static protected function get_event_types_list()
  {
    return array(
      'System.halt' => array(
        'name'  => _('Switch off'),
        'img'   => 'geticon.php?context=actions&icon=system-shutdown&size=16'
      )
    );
  }
}

class argonautEventDeployment extends argonautEventTypes
{
  static protected function get_event_types_list()
  {
    return array(
      'Deployment.reboot' => array(
        'name'  => _('Reboot'),
        'img'   => 'geticon.php?context=actions&icon=system-reboot&size=16'
      ),
      'Deployment.wake' => array(
        'name'  => _('Wake up'),
        'img'   => 'geticon.php?context=status&icon=task-running&size=16'
      ),
      'Deployment.update' => array(
        'name'  => _('Software update'),
        'img'   => 'geticon.php?context=actions&icon=system-update&size=16'
      ),
      'Deployment.reinstall' => array(
        'name'  => _('(Re)Install'),
        'img'   => 'geticon.php?context=actions&icon=system-reinstall&size=16'
      ),
    );
  }
}
