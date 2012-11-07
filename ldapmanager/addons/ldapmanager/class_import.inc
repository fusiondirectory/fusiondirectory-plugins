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

class ldifimport extends plugin
{
  /* attribute list for save action */
  var $attributes     = array();
  var $objectclasses  = array();
  var $ui;
  var $view_logged    = FALSE;


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Import"),
      "plDescription" => _("Import of ldif data into the ldap tree"),
      "plSelfModify"  => FALSE,
      "plSection"     => array("addons"),
      "plCategory"    => array("ldapmanager"),
      "plObjectType"  => array("ldapmanager"),

      "plProvidedAcls"  => array()
    );
  }

  function ldifimport (&$config, $dn = NULL)
  {
    /* Include config object */
    $this->config = &$config;

    $this->ui = get_userinfo();
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Log view */
    if (!$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view", "all/".get_class($this), $this->dn);
    }

    /* Set government mode */
    $smarty = get_smarty();

    // Get the LDAP link, to generate the Export
    $ldap = $this->config->get_ldap_link();

    $smarty->assign("LDIFError", FALSE);
    $smarty->assign("type", FALSE);

    /* Get acls */
    $acl = $this->ui->get_permissions($this->config->current['BASE'], "all/all");

    /* Import requested check file and acls */
    if ((isset($_FILES['userfile']['name'])) && (isset($_POST['fileup']))) {

      /* Check acls */
      if (!preg_match("/w/", $acl)) {
          msg_dialog::display(_("Permission error"), _("You need full write access to the LDAP tree to import data!"), ERROR_DIALOG);
      } else {
        $smarty->assign("type", "importfile");
        $handle = NULL;

        // Check if there is a file submitted
        if (!$_FILES['userfile']['size'] > 0 ) {
          msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file is empty")), ERROR_DIALOG);
          $smarty->assign("LDIFError", TRUE);
        } elseif (!file_exists($_FILES['userfile']['tmp_name'])) { // Is there a tmp file, which we can use ?
          msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not found")), ERROR_DIALOG);
          $smarty->assign("LDIFError", TRUE);
        } elseif (!$handle = @fopen($_FILES['userfile']['tmp_name'], "r")) { // Can we open the tmp file, for reading
          msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not readable")), ERROR_DIALOG);
          $smarty->assign("LDIFError", TRUE);
        } else {
          // Everything just fine :)
          $str = "";

          // Reading content
          while (!feof($handle)) {
              $str .= fread($handle, 1024);
          }
          @fclose($handle);

          // Should we use Overwrite ?
          $overwrite = !empty($_POST['overwrite']);
          $cleanup = !empty($_POST['cleanup']);

          $ErrorStr = "";
          $check    = $ldap->import_complete_ldif($str, $ErrorStr, $overwrite, $cleanup);

          if ($check == INSERT_OK) {
              $smarty->assign("LDIFError", FALSE);
          } else {
              $smarty->assign("LDIFError", TRUE);
          }

          switch ($check) {
            case INSERT_OK:
                break;

            case ALREADY_EXISTING_ENTRY:
              msg_dialog::display(_("LDAP error"), $ErrorStr, ERROR_DIALOG);
                break;

            case UNKNOWN_TOKEN_IN_LDIF_FILE:
              msg_dialog::display(_("LDAP error"), $ErrorStr, ERROR_DIALOG);
                break;

            default:
              msg_dialog::display(_("Internal error"), sprintf(_("Undefined flag '%s'!"), $check), ERROR_DIALOG);
                break;
          }
        }
      }
    }
    return $smarty->fetch (get_template_path('contentimport.tpl', TRUE));
  }
}
?>
