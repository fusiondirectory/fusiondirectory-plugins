<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class filterFAIPackages extends filterLDAPBlacklist
{
  static function query ($parent, $base, $scope, $filter, $attributes, $category, $objectStorage = '')
  {
    $customs = array();
    foreach ($parent->customs as $custom) {
      if ($parent->elementValues[$custom] == 'true') {
        $customs[] = $custom;
      }
    }
    $filter     = preg_replace('/\*/', '', $filter);
    $pkgs       = array();
    $q          = new supportDaemon();
    $filter     = array("$filter");
    $attrs      = array('distribution', 'package', 'version', 'section', 'timestamp');
    $release    = session::get('packageSelect_Release');
    $ret        = $q->FAI_get_packages($release, $attrs, $filter, 0, 200);
    if ($q->is_error()) {
      msg_dialog::display(_('Infrastructure service'), msgPool::siError($q->get_error()), ERROR_DIALOG);
    } else {
      for ($i = 0;(count($ret) < 200) && ($i < count($customs));$i++) {
        $ret2 = $q->FAI_get_packages($customs[$i], $attrs, $filter, 0, 200 - count($ret));
        if ($q->is_error()) {
          msg_dialog::display(_('Infrastructure service'), msgPool::siError($q->get_error()), ERROR_DIALOG);
          return array();
        }
        $ret = array_merge($ret, $ret2);
      }
      foreach ($ret as $attr) {
        $attr['objectClass'] = 'FAKE_OC__FaiPackage';

        $item = array();
        $item['dn']     = 'dn='.$attr['PACKAGE'].','.$attr['DISTRIBUTION'].','.$base;
        $item['count']  = 0;
        foreach ($attr as $key => $value) {
          $item['count']++;
          $item[] = $key;
          $item[$key] = array('count' => 1, $value);
        }
        $pkgs[] = $item;
      }
    }

    return static::filterByBlacklist($pkgs);
  }

  function save ()
  {
    $act = $this->detectPostActions();
    $headpage = $this->getHeadpage();
    if (!isset($act['targets'])) {
      return array();
    }
    $ret = array();
    foreach ($act['targets'] as $dn) {
      $ret[] = $headpage->getEntry($dn);
    }
    return $ret;
  }
}
