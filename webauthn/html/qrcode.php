<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019 FusionDirectory

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

// We have to setup the proper path to the required file.
$root_path = dirname($_SERVER['DOCUMENT_ROOT']); //FD root folder
$subdirectory = "/include";
$required_path = $root_path . $subdirectory; // FD/include/

/* Require PHPQRCODE */
require_once($required_path.'/variables_webauthn.inc');

$data = urldecode($_GET['data']);

QRcode::png($data);
