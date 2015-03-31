<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

/* Remove classes and variables from session
 */
if ($cleanup) {
  session::un_set('addressbook');
} else {
  /* Create phonelist object on demand */
  if (!session::is_set('addressbook')) {
    $addressbook = new addressbook ($config);
    $addressbook->set_acl_base($ui->dn);
    $addressbook->set_acl_category("addressbook");
    session::set('addressbook',$addressbook);
    session::un_set('show_info');
  }
  $addressbook= session::get('addressbook');

  /* Execute formular */
  $display = $addressbook->execute();
  $display .= "<input type=\"hidden\" name=\"ignore\">\n";

  /* Page header*/
  $display = print_header(get_template_path('plugins/addressbook/images/icon.png'),
                          _("Addressbook")).$display;

  /* Store changes  in session */
  session::set('addressbook',$addressbook);
}
?>
