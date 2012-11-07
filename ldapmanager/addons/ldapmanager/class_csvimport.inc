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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110 - 1301, USA.
*/

class csvimport extends plugin
{
  /* Array with csv informations*/
  var $csvinfo = array();

  /* attribute list for save action */
  var $attributes     = array();
  var $objectclasses  = array();
  var $view_logged    = FALSE;

  var $separator = ";";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("CVS import"),
      "plDescription" => _("Import of cvs data into the ldap tree"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("ldapmanager"),
      "plObjectType"  => array("ldapmanager"),

      "plProvidedAcls"  => array()
    );
  }

  function csvimport (&$config, $dn = NULL)
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

    /* initiate smarty */
    $smarty = get_smarty();
    $smarty->assign("selectedattrs", array(0 => ""));
    $smarty->assign("data", array(0 => ""));
    $smarty->assign("head", array(0 => ""));
    $smarty->assign("sorted", 0);
    $smarty->assign("fileup", 0);

    /* Get the LDAP link, to generate the Export */
    $ldap = $this->config->get_ldap_link();

    $arrtemplates = array();
    $tempvar = 0;

    /* Array to fill in Formfields */
    $arrtemplates[$tempvar] = "None";
    if (!is_array($this->csvinfo)) {
      $this->csvinfo = array();
    }

    /* Set Usertemplate information and get all Attribute from userclass */
    unset ($this->csvinfo['arrtemplates']);
    unset ($this->csvinfo['arrtempls']);

    /* Generate Template Array, Attribute Array */
    if (!isset($this->csvinfo['arrtempls'])) {

      /* Generating attributes */
      $this->csvinfo['attr'] = array();
      $this->usertab = new usertabs($this->config, $this->config->data['TABS']['USERTABS'], 'new');
      foreach ($this->usertab->by_object as $pname => $plugin) { // for each user tab
        $attrs = @get_class_vars($pname);
        $attrs = $attrs['attributes'];            // get saved attributes
        foreach ($attrs as &$attr) {
          $attr = $pname.":".$attr;
        }
        $this->csvinfo['attr']  = array_merge($this->csvinfo['attr'], $attrs);
      }

      /* Attach the choice none to discard a csv column */
      $this->csvinfo['attr'] = array_merge(array("none","userPassword"), $this->csvinfo['attr']);

      /* Search all Templates  */
      $this->csvinfo['templates'] = $ldap->search("(objectClass = gosaUserTemplate)", array("*"));

      /* Array to handle selected Form fiels */
      $this->csvinfo['arrtempls'][$tempvar]['name'] = "";
      $this->csvinfo['arrtempls'][$tempvar]['dn']   = "";

      $this->csvinfo['arrtemplates'][] = "None";

      while ($temp = $ldap->fetch($this->csvinfo['templates'])) {
        $tempvar++;
        $this->csvinfo['arrtemplates'][$tempvar] = $temp['sn'][0]." - ".$this->config->idepartments[preg_replace("/^[^,] +,".preg_quote(get_people_ou(), '/')."/i", "", $temp['dn'])];
        $this->csvinfo['arrtempls']   [$tempvar]['name'] = $temp['sn'][0];
        $this->csvinfo['arrtempls']   [$tempvar]['dn']   = $temp['dn'];
      }
    }

    $arr_temp  = array_flip($this->csvinfo['attr']);
    $this->csvinfo['arr_selected'] = array($arr_temp['user:uid'],$arr_temp['user:sn'],$arr_temp['user:givenName'],$arr_temp['userPassword']);

    $smarty->assign("templates", $this->csvinfo['arrtemplates']);
    $smarty->assign("attrs", $this->csvinfo['attr']);

    /* Check permissions for import */
    $acl = $this->ui->get_permissions($this->config->current['BASE'], "all/all");
    if (!preg_match("/w/", $acl)) {
      if (isset($_POST['userfile']) || isset($_POST['sorted']) || isset($_POST['fileup'])) {
        msg_dialog::display(_("Permission error"), _("You've no permission to import CSV files."), ERROR_DIALOG);
      }
      return ($smarty->fetch (get_template_path('contentcsv.tpl', TRUE)));
    }

    /* If the given data from the csv file are sorted by the attributes */
    if (isset($_POST['sorted'])) {

      /* Template Handling */
      $smarty->assign("fileup", TRUE);
      $smarty->assign("sorted", TRUE);

      for ($i = 0; $i < $this->csvinfo['rows']; $i ++) {
        /* Preparing given Data */
        $this->csvinfo['sorteddata'][$i] = array();

        /* Go through every column */
        for ($a = 0; $a <= ($this->csvinfo['cols']); $a++) {
          /* Append a status col at last, to show possibly errors */
          if ($a == ($this->csvinfo['cols'])) {
            $this->csvinfo['sorteddata'][$i]["status"] = "-";
          } else {
            $cc = ($i * ($this->csvinfo['cols'])) + $a;

            /* If theres a Attribut given for this col */
            if (!empty($_POST[('row'.($a))])) {
              $this->csvinfo['sorteddata'][$i][$this->csvinfo['attr'][$_POST[('row'.($a))]]] = $this->csvinfo['data'][($cc)];
            }
          }
        }
      }

      /* aleast one entry */
      $rowcnt = 0;
      if (isset($this->csvinfo['sorteddata'])) {
        $rowcnt++;

        /* row count */
        $tbl_rows   = $this->csvinfo['count'] / $this->csvinfo['cols'];

        /* No error occurred */
        $failing = FALSE;

        /* For every row */
        for ($i = 0; $i < $this->csvinfo['rows']; $i ++) {

          /* Get element */
          $dat = $this->csvinfo['sorteddata'][$i];

          /* If not all needed vars set .... */
          /* givenName is used as first name and sn as second name */
          if (!isset($dat['user:givenName']) || empty($dat['user:givenName']) || !isset($dat['user:sn']) || empty($dat['user:sn'])) {

            /* Output Error */
            msg_dialog::display(_("Error"), _("Need at least 'sn' and 'givenName' to create users!"), ERROR_DIALOG);
            $this->csvinfo['sorteddata'][$i]['status'] = "<b>"._("failed")."</b>";
            $failing = $i + 1;
            continue;
          }

          /* Automatically generate 'uid' while it wasn't given yet.
           */
          if (!isset($dat['user:uid']) || empty($dat['user:uid'])) {

            // Generate uids and take the first result.
            $attributes = array();
            foreach ($dat as $key => $val) {
              $key_exp = explode(":", $key);
              if ($key_exp[0] == "user") {
                $attributes[$key_exp[1]] = $val;
              }
            }
            $uids = gen_uids ($this->config->get_cfg_value("idGenerator"), $attributes);
            if (!count($uids) || empty($uids[0])) {
              $msgs = sprintf(_("Failed to generate 'uid' for entry '%s'!"), ($i + 1));
              $this->csvinfo['sorteddata'][$i]['status'] = "<b>"._("failed")."</b>&nbsp;". $msgs;
              $failing = $i + 1;
              $this->csvinfo['sorteddata'][$i]['user:uid'] = "";
              $dat['user:uid'] = $this->csvinfo['sorteddata'][$i]['user:uid'];
              continue;
            } else {
              $this->csvinfo['sorteddata'][$i]['user:uid'] = array_pop($uids);
              $dat['user:uid'] = $this->csvinfo['sorteddata'][$i]['user:uid'];
            }
          }

          /* Register usertab to trigger edit dialog */
          $this->usertab = new usertabs($this->config, $this->config->data['TABS']['USERTABS'], 'new');
          $keys = array_keys($dat);

          /* Set all submitted vars  */
          if (is_array($keys)) {
            foreach ($keys as $key) {
              if ($key != "status") {
                $key_exp = explode(":", $key);
                $plugin = $key_exp[0];
                $var = $key_exp[1];
                if (!isset($plugin) || !isset($var)) {
                  continue;
                }
                if (in_array($var, $this->usertab->by_object[$plugin]->attributes)) {
                  $this->usertab->by_object[$plugin]->$var        = $dat[$key]; // set the variable
                  $this->usertab->by_object[$plugin]->is_account  = TRUE; // activate the plugin
                }
              }
            }
          }
          // Adapt template values.
          if ($this->csvinfo['tplid'] != 0) {
            $tpl_dn = $this->csvinfo['arrtempls'][$this->csvinfo['tplid']]['dn'];
            $this->usertab->adapt_from_template($tpl_dn, array("uid","cn","givenName","sn"));
            $tpl_base  = preg_replace("/^[^,] +,".preg_quote(get_people_ou(), '/i')."/", '', $tpl_dn);
            $this->usertab->by_object['user']->base = $tpl_base;
          }

          // Setting user Password
          if ((isset($dat['userPassword'])) && (!empty($dat['userPassword']))) {
            $password = $dat['userPassword'];
          } else {
            $rand_str = "";
            $feed     = "0123456789abcdefghijklmnopqrstuvwxyz";
            for ($e = 0; $e < 8; $e ++) {
              $rand_str .= substr($feed, rand(0, strlen($feed) - 1), 1);
            }
            $password = $rand_str;
          }

          /* Insert in the LDAP tree */
          if (count($this->usertab->check())) {
            msg_dialog::displayChecks($this->usertab->check());
            $this->csvinfo['sorteddata'][$i]['status'] = "<b>"._("failed")."</b>";
            $failing = $i + 1;
            break;
          } else {
            $this->csvinfo['sorteddata'][$i]['status'] = _("ok");
            $this->usertab->save();
            change_password($this->usertab->dn, $password);
          }
        }

        $pointsbefore = FALSE;
        $pointsafter  = FALSE;

        /* Get Attributs for Tableheadline  */
        $this->csvinfo['colsstr'] = array_keys($this->csvinfo['sorteddata'][0]);

        /*Create array with data for the template */
        if ($failing == FALSE) {

          $dataouts = array();
          for ($i = 1; $i <= $this->csvinfo['shownrowscount']; $i ++) {
            if (is_array($this->csvinfo['sorteddata'][($i - 1)])) {
              $dataouts[] = $this->csvinfo['sorteddata'][($i - 1)];
            }
          }

          if ($this->csvinfo['rows'] > $this->csvinfo['shownrowscount']) {
            $pointsafter = TRUE;
          }
        } else {
          $pointsbefore = TRUE;
          $pointsafter  = TRUE;

          $begin = $failing - 3;

          if ($begin < 0) {
            $begin        = 0;
            $pointsbefore = FALSE;
          }

          $end = $failing + 2;

          if ($end > $this->csvinfo['rows']) {
            $end          = $this->csvinfo['rows'] + 1;
            $pointsafter  = FALSE;
          }
          $this->csvinfo['shownrowscount'] = $end - $begin;

          if ($this->csvinfo['shownrowscount'] > $this->csvinfo['rows']) {
            $this->csvinfo['shownrowscount'] = $this->csvinfo['rows'];
          }

          $dataouts = array();
          for ($i = $begin; ($i <= $end) && (isset($this->csvinfo['sorteddata'][($i)])); $i ++) {
            if (is_array($this->csvinfo['sorteddata'][($i)])) {
                $dataouts[] = $this->csvinfo['sorteddata'][($i)];
            }
          }
        }
        $tmparr2  = $this->csvinfo['colsstr'];

        /* Error? */
        if ($failing) {
          msg_dialog::display(_("Error"), sprintf(_("Cannot insert entry '%s'!"), $failing), ERROR_DIALOG);
        }
        $smarty->assign("error", $failing);

        /* Set vars to smarty */
        $smarty->assign("cols", count($tmparr2));
        $smarty->assign("anz", $this->csvinfo['count']);
        $smarty->assign("rows", $this->csvinfo['shownrowscount']);


        $smarty->assign("head", $tmparr2);
        $smarty->assign("data", $dataouts);

        /* Set other vars  */
        $smarty->assign("i", 1);
        $smarty->assign("ie", 0);
        $smarty->assign("tplid", $this->csvinfo['tplid']);

        $smarty->assign("pointsafter", $pointsafter);
        $smarty->assign("pointsbefore", $pointsbefore);
      } else {
        /* Set Template ID */
        $tplid = $_POST['tplid'];


        $smarty->assign("tpl", $arrtemplates[$tplid]);
        msg_dialog::display(_("Information"), _("Nothing to import!"), INFO_DIALOG);
      }

      /* If there is a file uploaded */
    } else {
      /* Check if there is a file uploaded */
      if (!empty($_FILES['userfile']['name'])) {

        $handle = NULL;

        if ((!isset($_FILES['userfile']['name'])) || (!isset($_POST['fileup']))) {
          msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not found")), ERROR_DIALOG);
          $smarty->assign("LDIFError", TRUE);
        } elseif (!$_FILES['userfile']['size'] > 0) {
          msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file is empty")), ERROR_DIALOG);
          $smarty->assign("LDIFError", TRUE);
        } elseif (!file_exists($_FILES['userfile']['tmp_name'])) { /* Is there is a tmp file, which we can use ? */
          msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not found")), ERROR_DIALOG);
          $smarty->assign("LDIFError", TRUE);
        } elseif (!$handle = @fopen($_FILES['userfile']['tmp_name'], "r")) {
          msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not readable")), ERROR_DIALOG);
          $smarty->assign("LDIFError", TRUE);
        } else {
          $smarty->assign("fileup", TRUE);
          $str = "";


          /* Reading content */
          while (!feof($handle)) {
            $str .= fread($handle, 1024);
          }

          $lines  = preg_split("/\n/", $str);
          $anz    = 0;
          $rest   = 0;
          $data   = array();

          /* check column count */
          if (is_array($lines)) {
            foreach ($lines as $line) {
              /* continue if theres a comment */
              if (substr(trim($line), 0, 1) == "#") {
                continue;
              }

              $line   = str_replace ("\t", "", $line);
              $cells  = explode($this->separator, $line);

              if (count($cells) > $anz ) {
                $anz = count($cells);
              }
            }
          }

          /* At least one entry */
          if ($anz > 1) {

            /* Generate array with output info  */
            if (is_array($lines)) {
              foreach ($lines as $line) {
                $rest = 0;
                $cnt  = 0;

                /* dont use comments or empty lines */
                if ((substr(trim($line), 0, 1) == "#") || (empty($line))) {
                  continue;
                }

                /* replace \t to , */
                $line = str_replace ("\t", "", $line);

                /* remove " */
                $line = str_replace ("\"", "", $line);

                /* get all elements  */
                $cells  = explode($this->separator, $line);

                /* attach all elements to data array */
                if (is_array($cells)) {
                  foreach ($cells as $cell) {
                    if (!empty($cell)) {
                      $cnt ++;
                      $data[] = trim($cell);
                    }
                  }
                }

                /* cell count less than anz, attach some empty fields */
                if (($cnt != $anz) && (!empty($cnt))) {
                  $rest = $anz - $cnt;
                  for ($i = 0; $i < $rest; $i++) {
                    $data[] = " ";
                  }
                }
              }
            }

            unset($this->csvinfo['sorteddata']);
            unset($this->csvinfo['colsstr']);
            unset($this->csvinfo['sorteddata']);

            $this->csvinfo['cols']  = $anz;
            $this->csvinfo['data']  = array();
            $this->csvinfo['data']  = $data;
            $this->csvinfo['count'] = count($this->csvinfo['data']);

            if ($this->csvinfo['count'] > (6 * $this->csvinfo['cols'])) {
              /* only show 6 rows in ouptuttablei */
              $datouttemp = array_chunk($this->csvinfo['data'], (6 * $this->csvinfo['cols']));
              $this->csvinfo['dataout'] = $datouttemp[0];
              $this->csvinfo['shownrowscount'] = 6;
            } else {
              $this->csvinfo['shownrowscount']  = (count($this->csvinfo['data'])) / $this->csvinfo['cols'];
              $this->csvinfo['dataout']         = $this->csvinfo['data'];
            }

            $this->csvinfo['tplid']       = $_POST['template'];
            $this->csvinfo['templatestr'] = $this->csvinfo['arrtemplates' ][$this->csvinfo['tplid']];
            $this->csvinfo['count']       = count($this->csvinfo['data']);
            $this->csvinfo['rows']        = (count($this->csvinfo['data']) / $anz);

            $i = 0;
            $tmp = $tmp2 = array();
            if (is_array($this->csvinfo['dataout'])) {
              foreach ($this->csvinfo['dataout'] as $dat) {
                $tmp[] = $dat;
                $i++;
                if ($i >= $this->csvinfo['cols']) {
                  $i      = 0;
                  $tmp2[] = $tmp;
                  $tmp    = array();
                }
              }
            }

            /* Set Templateid  */
            $smarty->assign("tplid", $this->csvinfo['tplid']);

            /* Set Template  */
            $smarty->assign("tpl", $this->csvinfo['templatestr']);

            /* Temp var 1 */
            $smarty->assign("ia", 1);

            /* Temp var 2 */
            $smarty->assign("i", 0);

            /* Num rows  */
            $smarty->assign("rows", $this->csvinfo['shownrowscount']);


            for ($i  = 0; $i < $anz; $i++) {
              $this->csvinfo['arr_selected'][] = "0";
            }

            $smarty->assign("selectedattrs", $this->csvinfo['arr_selected']);

            /* Entrie count5 */
            $smarty->assign("anz", $this->csvinfo['cols']);

            /* Array with data */
            $smarty->assign("data", $tmp2);

            @fclose($handle);
          } else {
            $smarty->assign("tpl", $this->csvinfo['attr'][$_POST['template']]);
            $smarty->assign("LDIFError", TRUE);
            $smarty->assign("fileup", FALSE);
            msg_dialog::display(_("Error"), _("Cannot find CSV data in the selected file!"), ERROR_DIALOG);
          }
        }
      }
    }

    /* Show main page */
    return $smarty->fetch (get_template_path('contentcsv.tpl', TRUE));
  }

}

?>
