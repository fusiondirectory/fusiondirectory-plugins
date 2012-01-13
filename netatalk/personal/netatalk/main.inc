<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2006  Gina Haeussge <osd@foosel.net>
  Copyright (C) 2006  Bernd Zeimetz <bernd@zeimetz.de>
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

/* Remove locks created by this plugin */
$lock_msg="";
if ($remove_lock || isset ($_POST['edit_cancel'])){
  if(session::is_set('netatalk') && session::is_set('edit')){
    del_lock($ui->dn);
  }
}

/* Remove this plugin from session */
if ( $cleanup ){
  session::un_set('netatalk');
  session::un_set('edit');

}else{

  /* Reset requested? */
  if (isset ($_POST['edit_cancel'])) {
    session::un_set('edit');
    session::un_set('netatalk');
  }

  /* Create netatalk object on demand */
  if (!session::is_set('netatalk') || (isset ($_GET['reset']) && $_GET['reset'] == 1)) {
    $netatalk = new netatalk($config, $ui->dn);
    $netatalk->enable_CSN_check();
    $netatalk->set_acl_base($ui->dn);
    $netatalk->set_acl_category("users");
    session::set('netatalk',$netatalk);
  }
  $netatalk  = session::get('netatalk');

  /* save changes back to object */
  if (session::is_set('edit')) {
    $netatalk->save_object();
  }

  /* Enter edit mode? */
  if (isset ($_POST['edit'])) {

    /* Check locking */
    if (($username = get_lock($ui->dn)) != "") {
      session::set('back_plugin',$plug);
      $lock_msg =  gen_locked_message ($username, $ui->dn);
    }

    /* Lock the current entry */
    add_lock($ui->dn, $ui->dn);
    session::set('edit',TRUE);
  }

  /* save changes to LDAP and disable edit mode */
  if (isset ($_POST['edit_finish'])) {

    /* Perform checks */
    $message = $netatalk->check();

    /* No errors, save object */
    if (count($message) == 0) {
      $netatalk->save();
      del_lock($ui->dn);
      session::un_set('edit');

      /* Remove object */
      session::un_set('netatalk');
    } else {
      /* Errors found, show message */
      msg_dialog::displayChecks($message);
    }
  }

  /* Execute formular */
  if(!empty($lock_msg)){
    $display = $lock_msg;
  }else{
    $display.= $netatalk->execute ();
  }


  /* Store changes  in session */
  if (session::is_set('edit')) {
    session::set('netatalk',$netatalk);
  }

  $info = "";
  /* Show page footer depending on the mode */
  if ($netatalk->is_account) {
    $display .= "<p class=\"plugbottom\">";

    /* Are we in edit mode? */
    if (session::is_set('edit')) {
      $display .= "<input type=submit name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
      $display .= "&nbsp;";
      $display .= "<input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
      $info = "<img alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/locked.png')."\"> ".$ui->dn."&nbsp;";
    } else {
      $info = "<img alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/unlocked.png')."\"> ".$ui->dn."&nbsp;";
      if(preg_match("/r/",$ui->get_permissions($ui->dn,"users/netatalk"))){
        $info .= "<img alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/on.png')."\"> ".msgPool::clickEditToChange();
        $display .= "<input type=submit name=\"edit\" value=\"".msgPool::editButton()."\">\n";
      }

      $display .= "<input type=\"hidden\" name=\"ignore\">\n";
    }
    $display .= "</p>\n";
  }

  /* Page header*/
  $display = print_header(get_template_path('plugins/netatalk/images/plugin.png'), _("Netatalk settings"), $info).$display;

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
