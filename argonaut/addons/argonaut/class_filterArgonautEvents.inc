<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2016  FusionDirectory

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

class filterArgonautEvents extends  filterLDAP
{
  static function query($parent, $base, $scope, $filter, $attributes, $category, $objectStorage = "")
  {
    global $config;

    $o_queue = new supportDaemon();
    $events  = argonautEventTypes::get_event_types();

    /* Get tags that will be used in queue searches */
    $event_tags = array("none");
    foreach ($events as $action => $evt) {
      $event_tags[] = $action;
    }

    $entries = $o_queue->get_queued_entries($event_tags, 0, 9999999, "id");
    if ($o_queue->is_error()) {
      msg_dialog::display(_("Error"), sprintf(_("Cannot load queue entries: %s"), "<br><br>".$o_queue->get_error()), ERROR_DIALOG);
      return array();
    }

    /* Assign entries by id.
     */
    $data = array();
    $ui   = get_userinfo();


    $acl_base = $config->current['BASE'];
    $acl_is_writeable   = preg_match('/w/', $ui->get_permissions($acl_base, "argonautQueue/argonautQueue", "0"));
    $acl_is_removeable  = preg_match('/d/', $ui->get_permissions($acl_base, "argonautQueue/argonautQueue", "0"));
    $acl_is_readable    = preg_match('/r/', $ui->get_permissions($acl_base, "argonautQueue/argonautQueue", "0"));

    foreach ($entries as $entry) {

      // Detect entry status
      $entry['objectClass'][] = "FAKE_OC__gotoEvent";

      /* If WAITING add priority action
       */
      if (in_array($entry['STATUS'], array("waiting")) && $acl_is_writeable) {
        $entry['objectClass'][] = "FAKE_OC__prioUp";
        $entry['objectClass'][] = "FAKE_OC__prioDown";
        $entry['objectClass'][] = "FAKE_OC__prioPause";
      }

      /* If PAUSED add resume action
       */
      if (in_array($entry['STATUS'], array("paused")) && $acl_is_writeable) {
        $entry['objectClass'][] = "FAKE_OC__prioResume";
      }

      /* If PAUSED or WAITING add execution action
       */
      if (in_array($entry['STATUS'], array("paused","waiting")) && $acl_is_writeable) {
        $entry['objectClass'][] = "FAKE_OC__processNow";
      }

      /* Add logview button, currently ever.
       */
      if ($acl_is_readable) {
        $entry['objectClass'][] = "FAKE_OC__viewLogs";
      }

      /* If PAUSED or WAITING add edit action
       */
      if (in_array($entry['STATUS'], array("waiting")) && $acl_is_writeable) {
        $entry['objectClass'][] = "FAKE_OC__edit";
      }


      /* If PROCESSING add abort action
       */
      if (in_array($entry['STATUS'], array("processing")) && preg_match("/install/", $entry['HEADERTAG']) && $acl_is_writeable) {
        $entry['objectClass'][] = "FAKE_OC__abort";
      }

      /* If ERROR add retry action
       */
      if (in_array($entry['STATUS'], array("error")) && $acl_is_writeable) {
        $entry['objectClass'][] = "FAKE_OC__retry";
      }

      /* If WAITING or ERROR add remove action
       */
      if ( $acl_is_removeable && in_array($entry['STATUS'], array("waiting","error","processed"))) {
        $entry['objectClass'][] = "FAKE_OC__remove";
      }
      if ($acl_is_writeable && in_array($entry['STATUS'], array("processing")) && !preg_match("/install/", $entry['HEADERTAG'])) {
        $entry['objectClass'][] = "FAKE_OC__remove";
      }

      $item = array();
      $item['count']  = 0;
      $item['EVENT']  = $entry;
      $item['dn']     = $entry['MACADDRESS'].",".$entry['TIMESTAMP'];

      foreach ($entry as $name => $value) {
        $item[] = $name;
        if (!is_array($value)) {
          $item[$name] = array('count' => 1, $value);
        } else {
          $item[$name] = $value;
          $item[$name]['count'] = count($value);
        }
        $item['count'] ++;
      }
      $data[] = $item;
    }

    return $data;
  }
}
?>
