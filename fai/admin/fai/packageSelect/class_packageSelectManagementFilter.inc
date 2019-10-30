<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

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

class packageSelectManagementFilter extends managementFilter
{
  function __construct (management $parent, bool $fixedScope = NULL, array $filterElementDefinitions = [])
  {
    $filterElementDefinitions[] = ['PackageSelectCustomFilterElement', []];

    parent::__construct($parent, $fixedScope, $filterElementDefinitions);
  }

  function query (array $attrs, string $base): array
  {
    $objectTypeCount  = [];
    $entries          = [];
    $row              = 0;
    $q                = new supportDaemon();
    $filter           = [$this->search];
    $attrs            = ['distribution', 'package', 'version', 'section', 'timestamp'];
    $release          = $this->parent->getRelease();
    foreach ($this->parent->objectTypes as $type) {
      if (!$this->types['filter_type_'.$type]['show']) {
        continue;
      }

      $customs = [];
      $this->filterElements[0]->getFilters($type, $customs);

      $ret        = $q->FAI_get_packages($release, $attrs, $filter, 0, 200);
      if ($q->is_error()) {
        msg_dialog::display(_('Infrastructure service'), msgPool::siError($q->get_error()), ERROR_DIALOG);
      } else {
        for ($i = 0;(count($ret) < 200) && ($i < count($customs));$i++) {
          $ret2 = $q->FAI_get_packages($customs[$i], $attrs, $filter, 0, 200 - count($ret));
          if ($q->is_error()) {
            msg_dialog::display(_('Infrastructure service'), msgPool::siError($q->get_error()), ERROR_DIALOG);
            return [];
          }
          $ret = array_merge($ret, $ret2);
        }
        $objectTypeCount[$type] = count($ret);
        foreach ($ret as $attr) {
          if (in_array($attr['PACKAGE'], $this->parent->blackList['PACKAGE'])) {
            continue;
          }
          $item = [];
          foreach ($attr as $key => $value) {
            $item[$key] = [$value];
          }
          $dn   = 'dn='.$attr['PACKAGE'].','.$attr['DISTRIBUTION'].','.$base;
          $entries[$dn] = new ListingEntry($this->parent->listing, $type, $dn, $item, $row++);
          $entries[$dn]->aclBase = $base;
        }
      }
    }

    return [$entries, $objectTypeCount];
  }
}
