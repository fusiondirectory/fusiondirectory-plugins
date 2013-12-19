<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class addressbook extends plugin
{
  /* Generic */
  var $ui;

  /* Phonelist attributes */
  var $telephone_list     = array();
  var $new_dn             = "";
  var $orig_cn            = "";
  var $storage_base       = "";
  var $orig_storage_base  = "";

  /* Filter attributes */
  var $start            = 0;
  var $search_for       = "*";
  var $search_base      = "";
  var $search_type      = "";
  var $range            = 20;

  /* Currently edited/added entry attributes */
  var $sn                       = "";
  var $cn                       = "";
  var $givenName                = "";
  var $mail                     = "";
  var $title                    = "";
  var $personalTitle            = "";
  var $initials                 = "";
  var $homePostalAddress        = "";
  var $homePhone                = "";
  var $mobile                   = "";
  var $o                        = "";
  var $postalAddress            = "";
  var $l                        = "";
  var $postalCode               = "";
  var $st                       = "";
  var $ou                       = "";
  var $telephoneNumber          = "";
  var $facsimileTelephoneNumber = "";
  var $pager                    = "";
  var $view_logged              = FALSE;

  /* attribute list for save action */
  var $attributes = array("sn", "givenName", "mail", "title",
      "initials", "homePostalAddress", "displayName",
      "homePhone", "mobile", "o", "postalAddress", "l",
      "postalCode", "st", "ou", "telephoneNumber",
      "facsimileTelephoneNumber", "pager");

  var $objectclasses = array("top", "person", "organizationalPerson", "inetOrgPerson");

  var $abobjectclass = "dc=addressbook";
  var $acl_targets_objects = array();

  function addressbook (&$config, $dn = NULL)
  {
    /* Include config object */
    $this->config = &$config;

    $this->base = $this->config->current['BASE'];

    /* Check if there is a special ldap-sub-tree specified, instead of dc=addressbook, */
    $aoc = $this->config->search("addressbook", "addressbookBaseDN",array('menu'));
    if ($aoc != "") {
      $this->abobjectclass  = $aoc;
    }

    /* Get global filter config */
    if (!session::is_set("phonefilter")) {
      $ui         = get_userinfo();
      $base       = get_base_from_people($ui->dn);
      $phonefilter= array(
          "search_base"       => $base,
          "organizational"    => "checked",
          "global"            => "checked",
          "search_for"        => "*",
          "object_type"       => "*");
      session::set("phonefilter", $phonefilter);
    }

    $this->ui = get_userinfo();
    $this->acl_targets_objects = $this->ui->get_acl_target_objects();
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    $smarty = get_smarty();

    /* Prevent empty variables for smarty */
    foreach ($this->attributes as $atr) {
      $smarty->assign($atr,"");
    }

    /* Save formular information */
    $phonefilter = session::get("phonefilter");
    foreach (array("search_for", "search_base", "object_type") as $type) {
      if (isset($_POST[$type])) {
        $phonefilter[$type] = $_POST[$type];
      }
      $this->$type = $phonefilter[$type];
    }
    if (isset($_POST['search_base'])) {
      foreach (array("organizational", "global") as $type) {
        if (isset($_POST[$type])) {
          $phonefilter[$type] = "checked";
        } else {
          $phonefilter[$type] = "";
        }
      }
    }

    /* Search string */
    $s = $phonefilter['search_for'];
    if ($s == "") {
      $s = "*";
    }
    if (isset($_GET['search'])) {
      $s = validate(mb_substr($_GET['search'], 0, 1, "UTF8"))."*";
      if ($s == "**") {
        $s = "*";
      }
      $this->search_for = $s;
      $phonefilter['search_for'] = $s;
    }
    session::set("phonefilter", $phonefilter);

    /* Assign create acl */
    $acl = $this->get_entry_acls($this->base);
    $smarty->assign("internal_createable", preg_match("/c/",$acl));

    $acl = $this->get_entry_acls($this->dn);
    $smarty->assign("internal_removeable", preg_match("/d/",$acl));
    $smarty->assign("internal_editable",   preg_match("/w/",$acl));

    /* Perform actions with CTI hook */
    if (isset($_GET['target'])
        && isset($_GET['dial'])
        && $this->config->get_cfg_value("ctiHook") != "" ) {

      $dialmode= $_GET['dial'];
      if ($dialmode == "telephoneNumber" ||
          $dialmode == "mobile" ||
          $dialmode == "homePhone") {

        /* Get target */
        $ldap = $this->config->get_ldap_link();
        $ldap->cat(base64_decode($_GET['target']), array('telephoneNumber', 'mobile', 'homePhone'));
        $attrs = $ldap->fetch();
        if (isset($attrs["$dialmode"])) {
          $target = $attrs[$dialmode][0];
        } else {
          $target = "";
        }

        /* Get source */
        $ui = get_userinfo();
        $ldap->cat($ui->dn, array('telephoneNumber'));
        $attrs = $ldap->fetch();
        if (isset($attrs["telephoneNumber"])) {
          $source = $attrs['telephoneNumber'][0];
        } else {
          $source = "";
        }

        /* Save to session */
        session::set('source',$source);
        session::set('target',$target);

        /* Perform call */
        if ($target != "" && $source != "") {
          $smarty->assign("phone_image", get_template_path('plugins/addressbook/images/phone.png'));
          $smarty->assign("dial_info", sprintf(_("Dial from %s to %s now?"), "<b style='font-size:22px; color:red'>".$source."</b>", "<b style='font-size:22px;color:red'>".$target."</b>"));
          return $smarty->fetch(get_template_path('dial.tpl', TRUE));
        } else {
          msg_dialog::display(_("Error"), _("You need to set your personal phone number in order to perform direct dials."), ERROR_DIALOG);
        }
      }

    }

    /* Finally dial */
    if (isset($_POST['dial']) && session::is_set('source') && session::is_set('target')) {
      exec ($this->config->get_cfg_value("ctiHook")." ".
            escapeshellarg(session::get('source'))." ".escapeshellarg(session::get('target')), $dummy, $retval);
      session::un_set('source');
      session::un_set('target');
    }


    /* Delete entry? */
    if (isset($_POST['delete_entry_confirm'])) {

      /* Some nice guy may send this as POST, so we've to check
         for the permissions again. */

      $acl = $this->get_entry_acls($this->dn);
      if (preg_match("/d/",$acl)) {

        /* Delete request is permitted, perform LDAP action */
        $ldap= $this->config->get_ldap_link();
        $ldap->rmdir ($this->dn);
        new log("remove","addressbook/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
        if (!$ldap->success()) {
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
        }
        new log("remove","addressbook/".get_class($this),$this->dn,array(),"Addressbook object'".$this->dn."' has been removed");

      } else {

        /* Normally this shouldn't be reached, send some extra
           logs to notify the administrator */
        msg_dialog::display(_("Permission"),permDelete($this->dn),ERROR_DIALOG);
        new log("remove","addressbook/".get_class($this),$this->dn,array(),"Warning: '".$this->ui->uid."' tried to trick address book deletion.");
      }

      /* Remove lock file after successfull deletion */
      del_lock ($this->dn);

      /* Clean up */
      if (session::is_set('saved_start')) {
        $_GET['start']= session::get('saved_start');
      }
      session::un_set('show_info');
      session::un_set('saved_start');
    }


    /* Delete entry? */
    if (isset($_POST['delete_cancel'])) {
      del_lock ($this->dn);
    }


    /* Save address entry? */
    if (isset($_POST['save'])) {
      $this->save_object();
      $this->storage_base= $_POST['storage_base'];

      /* Perform checks */
      $message= $this->check ();

      /* No errors, save object */
      if (count ($message) == 0) {
        $this->save();

        /* Clean up */
        if (session::is_set('saved_start')) {
          $_GET['start']= session::get('saved_start');
        }
        session::set('show_info',$this->dn);
        session::un_set('saved_start');
      } else {
        /* Errors found, show message */
        msg_dialog::displayChecks($message);
      }
    }


    /* Close info window */
    if (isset($_GET['close']) || isset($_POST['cancel'])) {
      if (session::is_set('saved_start')) {
        $_GET['start']= session::get('saved_start');
      }
      session::un_set('show_info');
      session::un_set('saved_start');
    }


    /* Start address book edit mode? */
    if (isset($_GET['global'])) {
      if (!session::is_set('saved_start') && isset($_GET['start'])) {
        session::set('saved_start',$_GET['start']);
      }
      switch ($_GET['global']) {
        case "add":
          $this->dn= "new";
        $this->orig_cn= "";

        /* Clean values */
        foreach ($this->attributes as $name) {
          $this->$name= "";
        }
        $this->saved_attributes= array();
        $this->storage_base= $this->config->current["BASE"];
        break;

        case "edit":
          /* Clean values */
          foreach ($this->attributes as $name) {
            $this->$name= "";
          }
        $this->dn = session::get('show_info');
        $this->load();
        $this->orig_cn = $this->cn;
        break;
        case "remove":
          $this->dn = session::get('show_info');
        $this->load();

        /* Load permissions for selected 'dn' and check if
           we're allowed to remove this 'dn' */
        $acl = $this->get_entry_acls($this->dn);
        if (preg_match("/d/",$acl)) {

          /* Check locking, save current plugin in 'back_plugin', so
             the dialog knows where to return. */
          if (($user= get_lock($this->dn)) != "") {
            return gen_locked_message ($user, $this->dn);
          }

          /* Lock the current entry, so nobody will edit it during deletion */
          $ui= get_userinfo();
          add_lock ($this->dn, $ui->dn);
          $smarty->assign("info", msgPool::deleteInfo($this->dn));
          return $smarty->fetch(get_template_path('remove.tpl', TRUE));
        } else {

          /* Obviously the user isn't allowed to delete. Show message and
             clean session. */
          msg_dialog::display(_("Permission"),permDelete($this->dn),ERROR_DIALOG);
        }
      }
      session::set('show_info',"ADD");
    }


    /* Open info window */
    if (isset($_GET['show'])) {
      if (!session::is_set('saved_start')) {
        session::set('saved_start',$_GET['start']);
      }
      $this->dn = base64_decode($_GET['show']);
      $this->view_logged =FALSE;
      if (!$this->view_logged) {
        $this->view_logged = TRUE;
        new log("view","addressbook/".get_class($this),$this->dn);
      }
      session::set('show_info',base64_decode($_GET['show']));
    }


    /* Get ldap link / build filter */
    $ldap= $this->config->get_ldap_link();
    $this->telephone_list= array ();


    /* Assemble bases
        (Depending on checkboxes, we search for organisational entries or seperated
        adressbook entries within dc=adressbook, ) */
    $bases = array();
    $filter = "";
    if ($phonefilter['global'] == "checked") {
      $bases[] = preg_replace("/".$this->config->current['BASE']."/", $this->abobjectclass.",".$this->config->current['BASE'], $this->search_base);
    } else {
      $filter = '(objectClass=gosaAccount)';
    }
    if ($phonefilter['organizational'] == "checked") {
      $bases[] = $this->search_base;
    }


    /* Only display those entries that have at least on of this attributes set */
    $must_have_this = array("telephoneNumber","facsimileTelephoneNumber","mobile","homePhone","mail");

    /* Requested attributes in ldap search */
    $attributes = array("sn", "givenName", "telephoneNumber", "facsimileTelephoneNumber", "mobile", "homePhone", "uid", "mail", "cn");

    /* Create attribute filter part */
    $attribute_filter = "";
    foreach ($attributes as $att) {
      $attribute_filter .= "(".$att."=".$s.")";
    }

    /* Walk through bases an check for usable entries */
    foreach ($bases as $base) {

      $ldap->cd($base);

      if ($phonefilter['object_type'] == '*') {
        $ldap->search(
            "(&(objectClass=person)$filter(!(objectClass=gosaUserTemplate))".   // Skip templates etc ..
            "(!(uid=*$))".                                                      // Skip entries with ...$ as uid
            "(|".$attribute_filter."))"
          ,$attributes);
      } else {
        $ldap->search("(&$filter(!(uid=*$))(!(objectClass=gosaUserTemplate))".                     //array
            "(".$phonefilter['object_type']."=$s))", $attributes);
      }

      /* Walk through LDAP results */
      while ($attrs= $ldap->fetch()) {

        /* prevent empty vaiables */
        foreach ($this->attributes as $atr) {
          if (!isset($attrs[$atr][0])) {
            $attrs[$atr][0] = "";
          }
        }

        /* Check if page number was posted */
        if (!isset($_GET['start'])) {
          $_GET['start']="";
        }

        /* Check if at least one attribute is specified */
        $skip = false;

        foreach ($must_have_this as $attr) {
          if (isset($attrs[$attr][0]) && !empty($attrs[$attr][0])) {
            $skip =false;
            break;
          }
        }

        /* Skip all attributes that we are not allowed to read */
        $any = false;
        foreach ($attributes as $attr) {

          $acls = $this->get_entry_acls($attrs['dn'],$attr);
          if(!preg_match("/r/",$acls)) {
            $attrs[$attr][0] = "";
          } else {
            $any = true;
          }
        }

        /* Only show lines that have set any mail or phone informations */
        if (!$skip && $any) {

          $this->telephone_list[$attrs['sn'][0].$attrs['dn']] =

            "<td class=\"phonelist\" title=\"".$attrs['sn'][0].", ".$attrs['givenName'][0].
              "\"onClick='location.href=\"main.php?plug=".validate($_GET['plug']).
                          "&amp;start=".validate($_GET['start']).
                          "&amp;show=".urlencode(base64_encode($attrs['dn']))."\"'>
              <a style='vertical-align:middle;' href=\"main.php?plug=".validate($_GET['plug']).
                          "&amp;start=".validate($_GET['start']).
                          "&amp;show=".urlencode(base64_encode($attrs['dn']))."\">".
               $attrs['sn'][0].", ".$attrs['givenName'][0].
            "</a>
            &nbsp;</td>
            <td class=\"phonelist\" title=\""._("Dial")." ".$attrs['telephoneNumber'][0]."\">
            <a style='vertical-align:middle;' href=\"main.php?plug=".validate($_GET['plug'])."&amp;dial=telephoneNumber&amp;start=".validate($_GET['start'])."&amp;target=".base64_encode($attrs['dn'])."\">".$attrs['telephoneNumber'][0]."
            </a>
            &nbsp;</td>
            <td class=\"phonelist\" title=\"".$attrs['facsimileTelephoneNumber'][0]."\">
            ".$attrs['facsimileTelephoneNumber'][0]."
            &nbsp;</td>
            <td class=\"phonelist\" title=\""._("Dial")." ".$attrs['mobile'][0]."\">
            <a style='vertical-align:middle;' href=\"main.php?plug=".validate($_GET['plug'])."&amp;dial=mobile&amp;start=".validate($_GET['start'])."&amp;target=".base64_encode($attrs['dn'])."\">".$attrs['mobile'][0]."
            </a>
            &nbsp;</td>
            <td class=\"phonelist\" title=\""._("Dial")." ".$attrs['homePhone'][0]."\">
            <a style='vertical-align:middle;' href=\"main.php?plug=".validate($_GET['plug'])."&amp;dial=homePhone&amp;start=".validate($_GET['start'])."&amp;target=".base64_encode($attrs['dn'])."\">".$attrs['homePhone'][0]."
            </a>
            &nbsp;</td>
            <td>
            <a href=\"plugins/addressbook/getvcard.php?dn=".base64_encode($attrs['dn'])."\">
            <img align=\"top\" border=0 src=\"images/save.png\" alt=\"vcf\" title=\"".sprintf(_("Save contact for %s as vcard"), $attrs['givenName'][0]." ".$attrs['sn'][0])."\">
            </a>";


          if (preg_match("/r/",$this->get_entry_acls($attrs['dn'],"mail"))) {
            if (isset($attrs['mail'][0]) && !empty($attrs['mail'][0])) {
              $dest = sprintf(_("Send mail to %s"), $attrs['mail'][0]);
              $this->telephone_list[$attrs['sn'][0].$attrs['dn']].=

                "<a href=\"mailto:".htmlentities($attrs['mail'][0])."\">".
                "<img  align=\"top\" border=0 src=\"images/mailto.png\" alt=\"vcf\" title=\"$dest\"></a>";
            }
          }
          $this->telephone_list[$attrs['sn'][0].$attrs['dn']] .= "&nbsp;</td>";
        }
      }
      error_reporting(E_ALL | E_STRICT);
    }

    /* Sort up list */
    ksort($this->telephone_list);
    reset($this->telephone_list);

    /* Fill template variables */
    $smarty->assign("search_for", $this->search_for);
    $smarty->assign("object_type", $this->object_type);

    /* Create list of readable departments */
    $deps = array();
    foreach ($this->config->idepartments as $dn => $name) {
      if ( $this->acl_is_readable($dn) || $dn == $this->config->current['BASE']) {
        $deps[$dn] = $name;
      }
    }

    $this->base = $phonefilter['search_base'];
    $smarty->assign("deplist", $deps);
    $smarty->assign("depselect", $this->search_base);
    $smarty->assign("global", $phonefilter['global']);
    $smarty->assign("organizational", $phonefilter['organizational']);
    $smarty->assign("search_image", get_template_path('images/lists/search.png'));
    $smarty->assign("obj_image", get_template_path('plugins/addressbook/images/objects.png'));
    $smarty->assign("tree_image", get_template_path('images/lists/search-subtree.png'));
    $smarty->assign("infoimage", get_template_path('images/info.png'));
    $smarty->assign("actionimage", get_template_path('images/action.png'));
    $smarty->assign("launchimage", get_template_path('images/launch.png'));

    /* Generate alphabet */
    $alphabet = generate_alphabet();

    /* Build list output */
    $output = "";
    $mod = 0;


    /* View detailed infos */
    $smarty->assign("show_info", "");
    if (session::is_set('show_info')) {

      $range= 4;
      $smarty->assign("show_info", "1");
      $smarty->assign("url", "main.php?plug=".validate($_GET['plug'])."&amp;close=1");

      $tmp = $this->plInfo();

      if (isset($_POST['storage_base'])) {
        $this->storage_base = $_POST['storage_base'];
      }

      switch (session::get('show_info')) {

        case "ADD":

          $a_bases = $this->get_allowed_bases();

          if (!isset($a_bases[$this->storage_base])) {
            $base = key($this->get_allowed_bases());
            $this->storage_base = $base;
          }

          $smarty->assign ('storage_base', $this->storage_base);
          $smarty->assign ('address_info', get_template_path('address_edit.tpl', TRUE, dirname(__FILE__)));

          foreach ($tmp['plProvidedAcls'] as $name => $translated) {
            $smarty->assign($name."ACL",$this->get_entry_acls($this->abobjectclass.",".$base,$name));
          }
        break;

        default:
          $smarty->assign ('address_info', get_template_path('address_info.tpl', TRUE, dirname(__FILE__)));
          foreach ($tmp['plProvidedAcls'] as $name => $translated) {
            $smarty->assign($name."ACL",$this->get_entry_acls($this->dn,$name));
          }
        break;
      }

      /* Fill variables from LDAP */
      if (session::get('show_info') != "ADD") {
        $ldap->cat(session::get('show_info'), $this->attributes);
        $info= $ldap->fetch();
      }
      foreach ($this->attributes as $name) {

        $dn = $this->dn;
        if ($dn == "new") $dn = $this->base;

        /* Skip entries we are not allowed to read */
        if (!preg_match("/r/",$this->get_entry_acls($dn,$name))) {
          $smarty->assign("info_$name", "");
        } elseif (session::get('show_info') != "ADD" && isset($info["$name"][0])) {
          error_reporting(0);
          /* Special treatment for phone attributes */
          if ($name == "mobile" ||
              $name == "homePhone" ||
              $name == "telephoneNumber") {
            $smarty->assign("info_$name",
                "<a title=\""._("Dial")." ".$info["$name"][0]."\" href=\"main.php?plug=".
                validate($_GET['plug'])."&amp;dial=$name&amp;start=".validate($_GET['start']).
                "&amp;target=".base64_encode(session::get('show_info'))."\">".$info["$name"][0]."</a>");
          } else {
            $smarty->assign("info_$name", preg_replace("/\n/", "<br>", $info["$name"][0]));
          }
          error_reporting(E_ALL | E_STRICT);
        } elseif (session::get('show_info') == "ADD" && isset($this->$name)) {
          $smarty->assign("info_$name", $this->$name);
        } else {
          $smarty->assign("info_$name", "-");
        }
      }
      if (preg_match("/,".$this->abobjectclass.",/", session::get('show_info'))) {
        $storage= _("global addressbook");
        $smarty->assign("internal", 0);
      } else {
        $storage= _("user database");
        $smarty->assign("internal", 1);
      }
      if (session::get('show_info') != "ADD") {
        $smarty->assign("storage_info", sprintf(_("Contact stored in '%s'"), $storage));
      } else {
        $smarty->assign("storage_info", _("Creating new entry in"));
      }
    } else {
      if (isset($_POST['EntryPerPage'])) {
        $this->range = $_POST['EntryPerPage'];
      }
      $range = $this->range;
      $smarty->assign("internal", 1);
    }
    if (isset($_GET['start'])) {
      $this->start= validate($_GET['start']);
    }
    foreach ($this->telephone_list as $val) {
      if ($mod < $this->start) {
        $mod++;
        continue;
      }
      if ($mod >= ($this->start + $range)) {
        $mod++;
        break;
      }
      if ( ($mod++) & 1) {
        $col = "style=\"background-color: #FFFFFF;\"";
      } else {
        $col = "style=\"background-color: #F5F5F5;\"";
      }
      $output .= "<tr $col>\n$val</tr>\n";
    }

    $smarty->assign("search_result", $output);
    $smarty->assign("apply", apply_filter());
    $smarty->assign("alphabet", $alphabet);
    if ($range < 20) {
      $smarty->assign("range_selector", range_selector(count($this->telephone_list), $this->start, $range));
    } else {
      $smarty->assign("range_selector", range_selector(count($this->telephone_list), $this->start, $range, "EntryPerPage"));
    }
    $tmp = array("*" => _("All"), "sn" => _("Name"), "givenName" => _("Given name"),
        "telephoneNumber" => _("Work phone"), "mobile" => _("Cell phone"),
        "homePhone" => _("Home phone"), "uid" => _("User ID"));
    natsort($tmp);
    $smarty->assign("objlist", $tmp);

    /* Show main page */
    $smarty->assign('personal_image', get_template_path('plugins/addressbook/images/addr_personal.png'));
    $smarty->assign('home_image', get_template_path('plugins/addressbook/images/addr_home.png'));
    $smarty->assign('company_image', get_template_path('plugins/addressbook/images/addr_company.png'));
    $smarty->assign('add_image', get_template_path('images/lists/paste.png'));
    $smarty->assign('edit_image', get_template_path('images/lists/edit.png'));
    $smarty->assign('delete_image', get_template_path('images/lists/delete.png'));
    return $smarty->fetch(get_template_path('contents.tpl', TRUE));
  }

  function save_object()
  {
    plugin::save_object();
    foreach ($this->attributes as $attr) {

      /* save attributes depending on acls */
      $acl = $this->get_entry_acls($this->dn,$attr);

      if (preg_match("/w/",$acl)) {
        if (isset($_POST[$attr])) {
          $this->$attr = $_POST[$attr];
        }
      }
    }
  }

  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* must: sn, givenName */
    if ($this->sn == "") {
      $message[] = msgPool::required(_("Name"));
      return $message;
    }
    if ($this->givenName == "") {
      $message[] = msgPool::required(_("Given name"));
      return $message;
    }

    /* Check for valid name definition */
    if (preg_match ("/[\\\\]/", $this->sn)) {
      $message[] = msgPool::invalid(_("Name"),$this->sn,"/[\\\\]");
    }
    if (preg_match ("/[\\\\]/", $this->givenName)) {
      $message[] = msgPool::invalid(_("Given name"),$this->givenName,"/[\\\\]");
    }

    /* Check phone numbers */
    if (!tests::is_phone_nr($this->homePhone)) {
      $message[] = msgPool::invalid(_("Phone"),$this->homePhone);
    }
    if (!tests::is_phone_nr($this->telephoneNumber)) {
      $message[] = msgPool::invalid(_("Telephone number"),$this->telephoneNumber);
    }
    if (!tests::is_phone_nr($this->facsimileTelephoneNumber)) {
      $message[] = msgPool::invalid(_("Fax"),$this->facsimileTelephoneNumber);
    }
    if (!tests::is_phone_nr($this->mobile)) {
      $message[] = msgPool::invalid(_("Mobile"),$this->mobile);
    }
    if (!tests::is_phone_nr($this->pager)) {
      $message[] = msgPool::invalid(_("Pager"),$this->pager);
    }
    /* Check for reserved characers */
    if (preg_match ('/[,+"<>;]/', $this->givenName)) {
      $message[] = msgPool::invalid(_("Given name"),$this->givenName,'/[,+"<>;]/');
    }
    if (preg_match ('/[,+"<>;]/', $this->sn)) {
      $message[] = msgPool::invalid(_("Name"),$this->sn,'/[,+"<>;]/');
    }

    /* Check mail */
    if (!tests::is_email($this->mail)) {
      $message[] = msgPool::invalid(_("Email"),"","","your-domain@your-domain.com");
    }

    /* Assemble cn/dn */
    $this->cn= $this->givenName." ".$this->sn;
    if ($this->orig_cn != $this->cn || $this->storage_base != $this->orig_storage_base) {
      $this->new_dn= $this->create_unique_dn("cn", preg_replace("/,*".$this->config->current['BASE']."$/", "", $this->storage_base).",".$this->abobjectclass.",".$this->config->current['BASE']);
      if ($this->new_dn == "none") {
        $message[]= _("Cannot create a unique DN for your entry. Please fill more formular fields.");
        return $message;
      }
    } else {
      $this->new_dn= $this->dn;
    }

    return $message;
  }


  function load()
  {
    /* Load base attributes */
    parent::__construct ($this->config, $this->dn);
    $this->view_logged  = FALSE;
    $this->storage_base = preg_replace('/^[^,]+,/', '', preg_replace('/'.$this->abobjectclass.',/', '', $this->dn));
  }


  function save()
  {
    /* First use parents methods to do some basic fillup in $this->attrs */
    plugin::save();

    $this->attrs['cn'] = $this->cn;
    $this->attrs['displayName'] = $this->givenName." ".$this->sn;

    /* Move entry if it got another name... */
    if ($this->dn != "new" && $this->dn != $this->new_dn) {
      $this->move($this->dn, $this->new_dn);
    }
    $this->dn = $this->new_dn;

    /* Save data. Using 'modify' implies that the entry is already present, use 'add' for
       new entries. So do a check first... */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->dn,array('dn'));
    if ($ldap->fetch()) {
      $mode = "modify";
    } else {
      $mode = "add";
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
    }

    /* Finally write data with selected 'mode' */
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->$mode($this->attrs);
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
      return 1;
    }

    if($mode == "add") {
      new log("create","addressbook/".get_class($this),$this->dn, array_keys($this->attrs),$ldap->get_error());
    } else {
      new log("modify","addressbook/".get_class($this),$this->dn, array_keys($this->attrs),$ldap->get_error());
    }
  }


  /* Return entry acls */
  function get_entry_acls($dn,$attr = "")
  {
    $acls = "";
    $combineACLs = FALSE;

    /* If combineACLs is set to false:
       Differentiate between user and addressbook acls, thus leads into two object categories.
        - real FusionDirectory users
        - and addressbook entries
       To view addressbook entries (e.g. ou=addressbook,) you just require permissions on the ldap base for 'addressbook'.
       To view real FusionDirectory users you have to additionally add user permissions.

       You can enable this option, to use only addressbook acls.
     */
    if($combineACLs) {
      $dn = preg_replace("/".preg_quote($this->abobjectclass, '/').",/","",$dn);
      $acls = $this->ui->get_permissions($dn,"addressbook/addressbook",$attr);
    } else {

      /* Use addressbook acls */
      if (preg_match("/".preg_quote($this->abobjectclass, '/')."/",$dn)) {
        $acls = "";
        foreach ($this->acl_targets_objects as $path) {
          $acls .= $this->ui->get_permissions($path,"addressbook/addressbook",$attr);
        }
      }
      /* Use Organizational Person acls */
      else {
        $acls = $this->ui->get_permissions($dn,"addressbook/addressbookgosa",$attr);
      }
    }

    return $acls;
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Addressbook"),
      "plDescription" => _("The addressbook plugin show internal users and external ones"),
      "plSelfModify"  => FALSE,
      "plSection"     => "addons",
      "plPriority"    => 10,
      "plCategory"    => array("addressbook" => array("objectClass" => "inetOrgPerson", "description" => _("Addressbook"))),

      "plProvidedAcls"    => array(
        "sn"                        => _("Surname"),
        "givenName"                 => _("Given name"),
        "telephoneNumber"           => _("Telefon number"),
        "facsimileTelephoneNumber"  => _("Fax number"),
        "mobile"                    => _("Mobile number"),
        "homePhone"                 => _("Home phone number"),
        "uid"                       => _("User identification"),
        "mail"                      => _("Mail address"),
        "pager"                     => _("Pager"),
        "o"                         => _("Organization"),
        "ou"                        => _("Department"),
        "l"                         => _("Location"),
        "postalAddress"             => _("Postal address"),
        "postalCode"                => _("Postal address"),
        "st"                        => _("State"),
        "initials"                  => _("Initials"),
        "title"                     => _("Title"),
        "homePostalAddress"         => _("Home postal address"),
        "cn"                        => _("Common name")
      )
    );
  }
}
?>
