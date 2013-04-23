<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

require_once("MDB2.php");
/* Basic setup, remove eventually registered sessions */
require_once ("php_setup.inc");
require_once ("functions.inc");
error_reporting (0);
session::start();
session::set('errorsAlreadyPosted', array());

/* Logged in? Simple security check */
if (!session::is_set('ui')) {
  new log("security", "faxreport/faxreport", "", array(), "Error: getfax.php called without session");
  header ("Location: index.php");
  exit;
}
$ui = session::is_set("ui");

/* User object present? */
if (!session::is_set('fuserfilter')) {
  new log("security", "faxreport/faxreport", "", array(), "getfax.php called without propper session data");
  header ("Location: index.php");
  exit;
}

/* Get loaded servers */
foreach (array("FAX_SERVER", "FAX_LOGIN", "FAX_PASSWORD") as $val) {
  if (session::is_set($val)) {
    $$val = session::get($val);
  }
}

/* Load fax entry */
$config = session::get('config');
$cfg = $config->data['SERVERS']['FAX'][0]; // FIXME : should support multiple FAX servers
$cfg['DB'] = 'gofax';
$link = databaseManagement::connectDatabase($cfg);
if (PEAR::isError($link)) {
  die(_("Could not connect to database server!")." ".$link->getMessage());
}

/* Permission to view? */
$query = "SELECT id,uid FROM faxlog WHERE id = '".validate(stripcslashes($_GET['id']))."'";
$result = $link->query($query);
if (PEAR::isError($result)) {
  die(_("Database query failed!")." ".$result->getMessage());
}
$line = $result->fetchRow(MDB2_FETCHMODE_ASSOC);
if (!preg_match ("/'".$line["uid"]."'/", session::get('fuserfilter'))) {
  die ("No permissions to view fax!");
}

$query  = "SELECT id,fax_data FROM faxdata WHERE id = '".validate(stripcslashes($_GET['id']))."'";
$data   = $link->queryOne($query, 'string', "fax_data");
if (PEAR::isError($data)) {
  die(_("Database query failed!")." ".$data->getMessage());
}

/* Load pic */
$link->disconnect();

if (!isset($_GET['download'])) {

  /* display picture */
  header("Content-type: image/png");

  $im = new Imagick();

  /* Loading image */
  if (!$im->readImage($data)) {
    new log("view", "faxreport/faxreport", "", array(), "Cannot load fax image");
  }

  /* Resizing image to 420x594 and blur */
  if (!$im->resizeImage(420, 594, Imagick::FILTER_GAUSSIAN, 1)) {
    msg_dialog::display(_("Error"), _("cannot resize fax image"), ERROR_DIALOG);
  }

  /* Converting image to PNG */
  if (!$im->setImageFormat('PNG')) {
    msg_dialog::display(_("Error"), _("cannot convert fax image to png"), ERROR_DIALOG);
  }

  /* Creating binary Code for the Image */
  if (!$data = $im->getImageBlob()) {
    msg_dialog::display(_("Error"), _("Reading fax image image failed"), ERROR_DIALOG);
  }

} else {

  /* force download dialog */
  header("Content-type: application/tiff\n");
  if (preg_match('/MSIE 5.5/', $HTTP_USER_AGENT) ||
      preg_match('/MSIE 6.0/', $HTTP_USER_AGENT)) {
    header('Content-Disposition: filename="fax.tif"');
  } else {
    header('Content-Disposition: attachment; filename="fax.tif"');
  }
  header("Content-transfer-encoding: binary\n");
  header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
  header("Last-Modified: ".gmdate("D, d M Y H:i:s")." GMT");
  header("Cache-Control: no-cache");
  header("Pragma: no-cache");
  header("Cache-Control: post-check=0, pre-check=0");

}

/* print the tiff image and close the connection */
echo "$data";

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
