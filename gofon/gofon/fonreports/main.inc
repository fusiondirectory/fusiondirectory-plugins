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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/* Remove classes and variables from session */
$lock_msg ="";
if ( $cleanup ){
  session::un_set('fonreport');
}else{

	/* Create fonreport object on demand */
	if (!session::is_set('fonreport')){
		session::set('fonreport',new fonreport ($config, $ui));
	}
	$fonreport = session::get('fonreport');

	/* Execute formular */
	$fonreport->save_object();
	$display= $fonreport->execute ();
	$display.= "<input type=\"hidden\" name=\"ignore\">\n";

	/* Page header*/
	$display= print_header(get_template_path('plugins/gofon/images/phonereport.png'), _("Phone reports")).$display;

	/* Store changes  in session */
	session::set('fonreport',$fonreport);
}
?>
