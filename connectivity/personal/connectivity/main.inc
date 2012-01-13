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


/* Remove locks created by this plugin */
$lock_msg="";
if ($remove_lock || isset($_POST['edit_cancel'])){
  if(session::is_set('connectivity') && session::is_set('edit')){
    del_lock($ui->dn);
  }
}

/* Remove this plugin from session */
if ( $cleanup ){
  session::un_set('connectivity');
  session::un_set('edit');

}else{


  /* Reset requested? */
  if (isset($_POST['edit_cancel'])){
    session::un_set ('edit');
    session::un_set ('connectivity');
  }

  /* Create connectivity object on demand */
  if (!session::is_set('connectivity') || (isset($_GET['reset']) &&
        $_GET['reset'] == 1)){
    $connectivity= new connectivity ($config, $ui->dn);
    $connectivity->enable_CSN_check();
    $connectivity->set_acl_base($ui->dn);
    $connectivity->set_acl_category("users");

    /* Adjust acl's to mode */
    foreach ($connectivity->plugin_name as $name){
      $connectivity->plugin[$name]->set_acl_category( "users");
      $connectivity->plugin[$name]->set_acl_base($ui->dn);
    }
    session::set('connectivity',$connectivity);
  }
  $connectivity = session::get('connectivity');

  /* save changes back to object */
  if (session::is_set('edit')){
    $connectivity->save_object ();
  }

  /* Enter edit mode? */
  if (isset($_POST['edit']) || isset($_POST['delete_lock'])){

    /* Check locking */
    if ((($username= get_lock($ui->dn)) != "") && (!session::is_set('edit'))){
      session::set('back_plugin',$plug);
      $display = gen_locked_message ($username, $ui->dn);
    }else{

      /* Lock the current entry */
      add_lock ($ui->dn, $ui->dn);
      session::set('edit',TRUE);
    }
  }

  /* save changes to LDAP and disable edit mode */
  if (isset($_POST['edit_finish'])){

    /* Perform checks */
    $message= $connectivity->check ();

    /* No errors, save object */
    if (count ($message) == 0){
      $connectivity->save ();
      del_lock ($ui->dn);
      session::un_set ('edit');

      /* Remove from session */
      session::un_set ('connectivity');
    } else {
      /* Errors found, show message */
      msg_dialog::displayChecks($message);
    }
  }

  /* Execute formular */
  $info= "";
  $display= $connectivity->execute ();

  /* Store changes  in session */
  if (session::is_set('edit')){
    session::set('connectivity',$connectivity);
  }


  $display.="<div align='right'>";
  if (session::is_set('edit')){
    $display.= "<input type=submit name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
    $display.= "&nbsp;";
    $display.= "<input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
    $info= "<img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/locked.png').
      "\"> ".$ui->dn."&nbsp;";
  } else {
    $info= "<img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/unlocked.png').
      "\"> ".$ui->dn."&nbsp;";

    $info.= "<img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/on.png').
      "\">".msgPool::clickEditToChange();
    $display.= "<input type=submit name=\"edit\" value=\"".msgPool::editButton()."\">\n";
    $display.= "<input type=\"hidden\" name=\"ignore\">\n";
  }
  $display.="</div>";

  /* Page header*/
  $display= print_header(get_template_path($connectivity->plIcon),
      _("Connectivity"), $info).$display;

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
