<?php
/*
 * This file is part of the autofs plugin for fusiondirectory
 * Copyright (C) 2011 Gaston TJEBBES - Majerti <g.t at majerti.fr>
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Autofs plugin
 * main file
 */

/* Remove locks created by this plugin */
if ($remove_lock) {
  if (session::is_set('autofsManagement')) {
    $macl = session::get('autofsManagement');
    $macl->remove_lock();
  }
}

/* Remove this plugin from session */
if ($cleanup) {
  session::un_set('autofsManagement');
} else {
  /* Reset requested? */
  if (isset($_GET['reset']) && $_GET['reset'] == 1) {
    session::un_set('autofsManagement');
  }

  /* Create usermanagement object on demand */
  if (!session::is_set('autofsManagement')) {
    $autofsManagement = new autofsManagement($config, $ui);
    session::set('autofsManagement', $autofsManagement);
  }
  $autofsManagement = session::get('autofsManagement');
  $display          = $autofsManagement->execute();

  /* Show and save dialog */
  session::set('autofsManagement', $autofsManagement);
}
