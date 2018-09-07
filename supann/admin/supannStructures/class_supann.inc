<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2018  FusionDirectory

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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supann
{
  static $choices_for = array();

  static function get_prefixed_choices_for($shortname)
  {
    $dir  = @opendir(SUPANN_DIR);
    $m    = array();
    $res  = array();

    while ($file = readdir($dir)) {
      if (preg_match('/^'.$shortname.'_(.+)$/', $file, $m)) {
        $prefix = $m[1];
        $res[$prefix] =& static::get_choices_for($file);
      }
    }
    closedir($dir);
    ksort($res);
    return $res;
  }

  static function &get_choices_for($type, $prefix = '')
  {
    if (isset(static::$choices_for[$prefix.$type])) {
      return static::$choices_for[$prefix.$type];
    }
    $entity_codes   = array();
    $entity_labels  = array();
    if (file_exists(SUPANN_DIR.'/'.$type)) {
      $entiteList = fopen(SUPANN_DIR.'/'.$type, 'r');
      if ($entiteList === FALSE) {
        msg_dialog::display(
          _("File error"),
          sprintf(_("Cannot read file: '%s'"), SUPANN_DIR.'/'.$type),
          ERROR_DIALOG
        );
        return array(array(), array());
      }
      while (($line = fgets($entiteList)) !== FALSE) {
        if (!preg_match ('/^#/', $line)) {
          $entite_line = preg_split('/;/', trim($line));

          $entity_codes[]   = $prefix.$entite_line[0];
          $entity_labels[]  = static::truncate_label($entite_line[1]);
        }
      }
      fclose($entiteList);
    }
    array_multisort($entity_labels, $entity_codes);
    static::$choices_for[$prefix.$type] = array($entity_codes, $entity_labels);
    return static::$choices_for[$prefix.$type];
  }

  /* return the 64 first chars and "…" after if text is longer */
  static function truncate_label($str, $len = 50)
  {
    if (mb_strlen($str) > $len) {
      return mb_substr($str, 0, $len).'…';
    } else {
      return $str;
    }
  }
}
