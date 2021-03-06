<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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
  static $choices_for = [];

  static function get_prefixed_choices_for (string $shortname, $processLabel = 'supann::truncate_label')
  {
    $dir  = @opendir(SUPANN_DIR);
    $m    = [];
    $res  = [];

    while ($file = readdir($dir)) {
      if (preg_match('/^'.$shortname.'_(.+)$/', $file, $m)) {
        $prefix = $m[1];
        $res[$prefix] =& static::get_choices_for($file, '', $processLabel);
      }
    }
    closedir($dir);
    ksort($res);
    return $res;
  }

  static function &get_choices_for (string $type, string $prefix = '', $processLabel = 'supann::truncate_label')
  {
    if (isset(static::$choices_for[$prefix.$type])) {
      return static::$choices_for[$prefix.$type];
    }
    $entity_codes   = [];
    $entity_labels  = [];
    if (file_exists(SUPANN_DIR.'/'.$type)) {
      $entiteList = fopen(SUPANN_DIR.'/'.$type, 'r');
      if ($entiteList === FALSE) {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Cannot read file: "%s"'),
            SUPANN_DIR.'/'.$type
          ))
        );
        $error->display();
        return [[], []];
      }
      while (($line = fgets($entiteList)) !== FALSE) {
        $line = trim($line);
        if (!preg_match('/^#/', $line) && !empty($line)) {
          $entite_line = explode(';', $line, 2);

          $entity_codes[]   = $prefix.$entite_line[0];
          $entity_labels[]  = $processLabel($entite_line[1]);
        }
      }
      fclose($entiteList);
    }
    array_multisort($entity_labels, $entity_codes);
    static::$choices_for[$prefix.$type] = [$entity_codes, $entity_labels];
    return static::$choices_for[$prefix.$type];
  }

  /*! \brief Returns the $len first chars and "…" after if text is longer */
  static function truncate_label ($str, $len = 50)
  {
    if (mb_strlen($str) > $len) {
      return mb_substr($str, 0, $len).'…';
    } else {
      return $str;
    }
  }
}
