<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

/*!
 * \brief This class handles the argonaut queue queries
 */
class argonautQueueFilter extends managementFilter
{
  function __construct (management $parent)
  {
    global $config;

    $this->parent = $parent;

    $this->pid = preg_replace('/[^0-9]/', '', microtime(TRUE));

    $this->types = [];

    $this->filterElements = [];

    $this->fixedScope = TRUE;
  }

  function update (string $base)
  {
  }

  function render (): string
  {
    return '';
  }

  function query (array $attrs, string $base): array
  {
    global $config, $ui;

    $events  = argonautEventTypes::get_event_types();

    /* Get tags that will be used in queue searches */
    $event_tags = ['none'];
    foreach ($events as $action => $evt) {
      $event_tags[] = $action;
    }

    $argonautEntries = $this->parent->o_queue->get_queued_entries($event_tags, 0, 9999999, 'id');
    if ($this->parent->o_queue->is_error()) {
      $error = new FusionDirectoryError(msgPool::siError($this->parent->o_queue->get_error()));
      $error->display();
      return [[], []];
    }

    $entries  = [];
    $row      = 0;

    foreach ($argonautEntries as $entry) {
      if (isset($entry['TARGETDN'])) {
        $acl_base = $entry['TARGETDN'];
      } else {
        $acl_base = $config->current['BASE'];
      }

      if (strpos($ui->get_permissions($acl_base, 'argonautQueue/argonautQueue', '0'), 'r') === FALSE) {
        /* ACL read check */
        continue;
      }

      $dn           = $entry['MACADDRESS'].','.$entry['TIMESTAMP'];
      $entries[$dn] = new ListingEntry($this->parent->listing, 'argonautTask', $dn, $entry, $row++);
      /* Set ACL base so that the ACLs checks works correctly ($dn is not a dn) */
      $entries[$dn]->aclBase = $acl_base;
    }

    return [$entries, ['ARGONAUTTASK' => count($entries)]];
  }
}
