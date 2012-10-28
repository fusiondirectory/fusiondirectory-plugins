<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* Remove classes and variables from session */
if ($cleanup) {
  session::un_set('board');
} else {
  $plInfo     = board::plInfo();
  $plIcon     = (isset($plInfo['plIcon'])?$plInfo['plIcon']:"plugin.png");
  $plHeadline = $plInfo['plShortName'];
  $plCategory = (isset($plInfo['plCategory'])?$plInfo['plCategory']:"board");
  if (is_array($plCategory)) {
    $c = key($plCategory);
    if (is_numeric($c)) {
      $c = $plCategory[0];
    }
    $plCategory = $c."/";
  }

  /* Create board object on demand */
  if (!session::is_set('board')) {
    $board = new boardtabs($config, $config->data['TABS']['BOARDTABS'], NULL, $plCategory);
    session::set('board', $board);
  }
  $board = session::get('board');

  /* Execute formular */
  $display = $board->execute();
  $display .= "<input type=\"hidden\" name=\"ignore\">\n";

  /* Page header*/
  $display = print_header(get_template_path($plIcon), $plHeadline).$display;

  /* Store changes  in session */
  session::set('board', $board);
}
?>
