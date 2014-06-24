<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

/* Remove locks created by this plugin
*/
if ($remove_lock || isset($_POST['edit_cancel'])){
  if(session::is_set('gofaxAccount') && session::is_set('edit')){
    del_lock($ui->dn);
  }
}

/* Remove this plugin from session
*/
if ( $cleanup ){
  session::un_set('gofaxAccount');
  session::un_set('edit');
}else{

  /* Reset requested? */
  if (isset($_POST['edit_cancel'])){
    session::un_set ('edit');
    session::un_set ('gofaxAccount');
  }

  /* Create gofaxAccount object on demand */
  if (!session::is_set('gofaxAccount') || (isset($_GET['reset']) && $_GET['reset'] == 1)){
    $gofaxAccount= new gofaxAccount ($config, $ui->dn);
    $gofaxAccount->set_acl_base($ui->dn);
    $gofaxAccount->set_acl_category("user");
    session::set('gofaxAccount',$gofaxAccount);
  }
  $gofaxAccount = session::get('gofaxAccount');

  /* save changes back to object */
  if (session::is_set('edit')){
    $gofaxAccount->save_object ();
  }

  /* Enter edit mode? */
  if (isset($_POST['edit'])){

    /* Check locking */
    if (($username= get_lock($ui->dn)) != ""){
      session::set('back_plugin',$plug);
      session::set('LOCK_VARS_TO_USE',array("/^edit$/","/^plug$/"));
      $lock_msg =  gen_locked_message ($username, $ui->dn);

    }else{

      /* Lock the current entry */
      add_lock ($ui->dn, $ui->dn);
      session::set('edit',TRUE);
    }
  }

  /* save changes to LDAP and disable edit mode */
  if (isset($_POST['edit_finish'])){

                /* Perform checks */
                $message= $gofaxAccount->check ();

                /* No errors, save object */
                if (count ($message) == 0){
                        $gofaxAccount->save ();
                        del_lock ($ui->dn);
                        session::un_set ('edit');

                } else {
                        /* Errors found, show message */
                        msg_dialog::displayChecks($message);
                }
        }

  /* Execute form */
  if(isset($lock_msg)){
    $display.= $lock_msg;
  }else{
    $display.= $gofaxAccount->execute ();
  }

  $info= "";

  /* Store changes  in session */
  if (session::is_set('edit')){
    session::set('gofaxAccount',$gofaxAccount);
  }

  /* Show page footer depending on the mode */
  if (!$gofaxAccount->dialog &&
    !$gofaxAccount->out_blocklist_dialog &&
    !$gofaxAccount->in_blocklist_dialog &&
    $gofaxAccount->is_account &&
    empty($lock_msg)){

    $display.= "<p class=\"plugbottom\">\n";

    /* Are we in edit mode? */
    if (session::is_set('edit')){
      $display.= "<input type=submit name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
      $display.= "&nbsp;\n";
      $display.= "<input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
      $info= "<img alt='' align=\"middle\" src=\"geticon.php?context=status&icon=object-locked&size=16\"> ".$ui->dn."&nbsp;";
    } else {
      $info= "<img alt=\"\" align=\"middle\" src=\"geticon.php?context=status&icon=object-unlocked&size=16\"> ".$ui->dn."&nbsp;";

      if(preg_match("/r/",$ui->get_permissions($ui->dn,"user/gofaxAccount"))){
        $info.= "<img alt=\"\" align=\"middle\" src=\"geticon.php?context=status&icon=dialog-information&size=16\">"._("Click the 'Edit' button below to change informations in this dialog");
        $display.= "<input type=submit name=\"edit\" value=\"".msgPool::editButton()."\">";
      }
      $display.= "<input type=\"hidden\" name=\"ignore\">\n";
    }
    $display.= "</p>";
  }

  /* Page header*/
  $display= print_header(get_template_path('plugins/fax/images/icon.png'), _("FAX settings"), $info).$display;

}
?>
