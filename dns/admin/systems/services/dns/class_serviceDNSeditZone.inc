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

class servdnseditZone extends plugin
{
  /* attribute list for save action */
  var $ignore_account = TRUE;
  var $attributes     = array("zoneName","ReverseZone","dNSClass",
      "sOAprimary","sOAmail","sOAserial","sOArefresh","sOAretry","sOAexpire","sOAttl");
  var $objectclasses  = array("whatever");

  var $RecordTypes              = array();

  var $ReverseZone              = "";
  var $zoneName                 = "";
  var $dNSClass                 = "IN";

  var $sOAprimary               = "";
  var $sOAmail                  = "";
  var $sOAserial                = "";
  var $sOArefresh               = "3600";
  var $sOAretry                 = "1800";
  var $sOAexpire                = "720000";
  var $sOAttl                   = "6400";

  var $Records                  = array();
  var $mXRecords                = array();

  var $OldZoneName              = ""; // To detect changes made with this edit
  var $OldReverseZone           = "";

  var $InitialReverseZone       = "";
  var $InitialzoneName          = "";
  var $NetworkClass             = "A"; // One out of A,B,C

  var $dialog                   = FALSE;

  var $zoneEditor               = NULL;

  var $isNew                    = TRUE;

  var $ZoneObject               = array();
  var $Zone_is_used             = FALSE;

  function servdnseditZone (&$config, $dn = NULL, $attrs = array())
  {
    parent::__construct($config, $dn);

    /* All types with required attrs */
    $this->RecordTypes = DNS::getDnsRecordTypes(TRUE);

    if (!count($attrs)) {
      $this->OldZoneName        = "";
      $this->OldReverseZone     = "";
      $this->isNew              = TRUE;
      $this->sOAserial          = date("Ymd")."1";

      $this->InitialzoneName    = "";
      $this->InitialReverseZone = "";
    } else {
      $this->ZoneObject = $attrs;

      if (isset($attrs['zoneEditor'])) {
        $this->zoneEditor = $attrs['zoneEditor'];
      }
      $this->OldZoneName        = $attrs['zoneName'];
      $this->OldReverseZone     = $attrs['ReverseZone'];

      $this->InitialzoneName    = $attrs['InitialzoneName'];
      $this->InitialReverseZone = $attrs['InitialReverseZone'];

      $this->isNew = FALSE;

      foreach ($this->attributes as $value) {
        $this->$value = $attrs[$value];
      }

      $this->sOAmail    = preg_replace("/\./", "@", $this->sOAmail, 1);
      $this->sOAmail    = preg_replace("/\.$/", "", $this->sOAmail);
      $this->sOAprimary = preg_replace("/\.$/", "", $this->sOAprimary);
      $this->zoneName   = preg_replace("/\.$/", "", $this->zoneName);

      if (isset($attrs['RECORDS'])) {
        $this->Records = $attrs['RECORDS'];

        $tmp2 = array();
        $usedPrio = array();
        foreach ($this->Records as $key => $rec) {
          if ($rec['type'] == "mXRecord") {
            $tmp = explode(" ", $rec['value']);
            $tmp2[$tmp[0]] = $rec;
            unset($this->Records[$key]);
          }
          if (($rec['type'] == "nSRecord") && preg_match("/".preg_quote($this->sOAprimary, '/')."/", $rec['value'])) {
            unset($this->Records[$key]);
          }
        }
        if (count($tmp2) != 0) {
          reset($tmp2);
          ksort($tmp2);
        }
        $this->mXRecords = $tmp2;
      } else {
        $this->mXRecords  = array();
        $this->Records    = array();
      }

      $str = date("Ymd");
      if (preg_match("/^".$str."/", $this->sOAserial)) {
        $this->sOAserial = $this->sOAserial + 1;
      } else {
        $this->sOAserial = date("Ymd")."01";
      }

      /* Check if this used, in this case disable forward and reverse configuration */
      $tmp = DNS::getUsedZoneNames();
      $this->Zone_is_used = FALSE;
      if (isset($tmp[DNS::FlipIp($this->InitialReverseZone).DNS::getInAddrArpa()])) {
        $this->Zone_is_used = TRUE;
      }
    }

    /* Detect Network class */
    if (!empty($this->ReverseZone)) {
      $dots = count(explode(".", $this->ReverseZone));
      if ($dots == 1) {
        $this->NetworkClass = "A";
        $this->ReverseZone  .= ".0.0.0";
      } elseif ($dots == 2) {
        $this->NetworkClass = "B";
        $this->ReverseZone  .= ".0.0";
      } else {
        $this->NetworkClass = "C";
        $this->ReverseZone  .= ".0";
      }
    }
  }


  function execute()
  {
    /* Call parent execute */
    parent::execute();

    /* Fill templating stuff */
    $smarty = get_smarty();
    $smarty->assign("Zone_is_used", $this->Zone_is_used);
    $ui = get_userinfo();

    /* Assign ACLs
        All acls are defined in our parent class.
     */
    $tmp = $this->parent->plInfo();
    foreach ($tmp['plProvidedAcls'] as $name => $desc) {
      $smarty->assign($name."ACL", $this->parent->getacl($name));
    }

    $display = "";

    /* Open Zone Entry Edit Dialog */
    if (!count($this->ZoneObject)) {
      $smarty->assign("AllowZoneEdit", FALSE);
    } else {
      $smarty->assign("AllowZoneEdit", TRUE);

      if (isset($_POST['EditZoneEntries']) && $this->parent->acl_is_readable("zoneEditor")) {
        if ($this->zoneEditor == NULL) {
          $this->zoneEditor         = new servDNSeditZoneEntries($this->config, $this->dn, $this->ZoneObject);
          $this->zoneEditor->acl    = $this->parent->getacl("zoneEditor");
          $this->zoneEditor->parent = $this;
        }
        $this->dialog = $this->zoneEditor;
      }
    }

    /* Save Zone Entry Edit Dialog */
    if (isset($_POST['SaveZoneEntryChanges'])) {
      $this->dialog->save_object();
      if (count($this->dialog->check())) {
        $msgs = $this->dialog->check();
        foreach ($msgs as $msg) {
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
        }
      } else {
        $this->zoneEditor = clone $this->dialog;
        $this->dialog     = FALSE;
      }
    }

    /* Cancel Zone Entrie Edit Dialog */
    if (isset($_POST['CancelZoneEntryChanges'])) {
      $this->dialog = FALSE;
    }

    /* Display any type of open dialogs */
    if (is_object($this->dialog)) {
      $this->dialog->save_object();
      return $this->dialog->execute();
    }

    foreach ($_POST as $name => $value) {
      if (preg_match("/^MXdel_/", $name)) {
        $id = preg_replace("/^MXdel_/", "", $name);
        $id = preg_replace("/_.*$/", "", $id);
        $id = base64_decode($id);

        unset($this->mXRecords[$id]);

        $tmp = array();
        foreach ($this->mXRecords as $entry) {
          $tmp[] = $entry;
        }

        $this->mXRecords = $tmp;
        break;
      }
    }

    if ($this->parent->acl_is_writeable("mXRecord")) {
      if (isset($_POST['AddMXRecord']) && !empty($_POST['StrMXRecord'])) {
        $rec = get_post('StrMXRecord');
        if (preg_match("/^[0-9]* /", $rec) && count(preg_split("/ /", $rec)) == 2) {
            list($prio,$value)      = preg_split("/ /", $rec);
            $this->mXRecords[$prio] = array("type" => "mXRecord", "value" => trim($rec));
        } else {
            msg_dialog::display(_("Warning"), msgPool::invalid(_("MxRecord"), "", "", "20 mx.record.entry"), WARNING_DIALOG);
        }
      }

      /* Handle Post events */
      foreach ($_POST as $name => $value) {
        /* Delete record if requested */
        if (preg_match("/RemoveRecord_/", $name)) {
          $id = preg_replace("/RemoveRecord_/", "", $name);
          unset($this->Records[$id]);
          break;
        }
      }

      /* Add new Zonerecord */
      if (isset($_POST['AddNewRecord'])) {
        $this->Records[] = array("type" => "aRecord", "value" => "");
      }
    }

    /* Fill in values */
    foreach ($this->attributes as $name) {
      $smarty->assign($name, $this->$name);
    }


    $div = new divSelectBox("MxRecords");
    $div->setHeight(120);
    $recs = $this->mXRecords;
    ksort($recs);
    $onedel = "<input name='MXdel_%s'   type='image' src='geticon.php?context=actions&icon=edit-delete&size=16'  title='"._("Delete")."'  class='center'>";
    foreach ($recs as $key => $rec) {
      list($prio,$value) = preg_split("/ /", $rec['value']);
      $div->AddEntry(
        array(
          array("html"    => $prio, "attach" => "style='width:20px;'"),
          array("html"    => $value),
          array("html"    => str_replace("%s", base64_encode($key), $onedel),
                "attach"  => "style='width:20px;border-right:0px;'")
        )
      );
    }

    /* Assign records list */
    $smarty->assign("NotNew",         FALSE);
    $smarty->assign("Mxrecords",      $div->DrawList());
    $smarty->assign("records",        $this->generateRecordsList());
    $smarty->assign("NetworkClass",   $this->NetworkClass);
    $smarty->assign("NetworkClasses", array("A" => "255.0.0.0 (Class A)","B" => "255.255.0.0 (Class B)","C" => "255.255.255.0 (Class C)"));

    /* Display tempalte */
    $display .= $smarty->fetch(get_template_path('serviceDNSeditZone.tpl', TRUE, dirname(__FILE__)));
    return $display;
  }

  function remove_from_parent()
  {
  }

  /* Save data to object */
  function save_object()
  {
    parent::save_object();

    if (!is_object($this->parent)) {
      return;
    }

    if ($this->parent->acl_is_writeable("zoneRecords")) {
      foreach ($this->Records as $id => $value) {
        if (isset($_POST['RecordTypeSelectedFor_'.$id])) {
          $this->Records[$id]['type'] = $_POST['RecordTypeSelectedFor_'.$id];
        }
        if (isset($_POST['RecordValue_'.$id])) {
          $this->Records[$id]['value'] = get_post('RecordValue_'.$id);
        }
      }
    }
    if (isset($_POST['NetworkClass']) && !$this->Zone_is_used && $this->parent->acl_is_writeable("NetworkClass")) {
      $this->NetworkClass = $_POST['NetworkClass'];
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message = plugin::check();

    /* Check if zoneName is already in use */
    $usedZones = DNS::getUsedZoneNames();
    if ($this->OldZoneName == "" && !empty($this->zoneName)) {
      $tmp = trim($this->zoneName, ".").".";
      if (isset($usedZones[$tmp])) {
        $message[] = msgPool::duplicated(_("Zone name"));
      }
    }

    if (empty($this->zoneName)) {
      $message[] = msgPool::required(_("Zone name"));
    }

    if (empty($this->ReverseZone)) {
      $message[] = msgPool::required(_("Reverse zone"));
    }

    if ($this->zoneName != strtolower($this->zoneName)) {
      $message[] = msgPool::invalid(_("Zone name"), "", "", _("Only lowercase allowed"));
    }

    if (!is_numeric($this->sOAserial)) {
      $message[] = msgPool::invalid(_("Serial"), $this->sOAserial, "/[0-9]/");
    }

    if (!is_numeric($this->sOArefresh)) {
      $message[] = msgPool::invalid(_("Refresh"), $this->sOArefresh, "/[0-9]/");
    }

    if (!is_numeric($this->sOAttl)) {
      $message[] = msgPool::invalid(_("Time to life"), $this->sOAttl, "/[0-9]/");
    }

    if (!is_numeric($this->sOAexpire)) {
      $message[] = msgPool::invalid(_("Expire"), $this->sOAexpire, "/[0-9]/");
    }

    if (!is_numeric($this->sOAretry)) {
      $message[] = msgPool::invalid(_("Retry"), $this->sOAretry, "/[0-9]/");
    }

    foreach ($this->Records as $name => $values) {
      /* only lower-case is allowed in record entries ... */
      if ($values['value'] != strtolower($values['value'])) {
        $message[] = msgPool::invalid($values['type'], "", "", _("Only lowercase allowed"));
      }
    }

    /* Check class for given Zone Address */
    $addr = preg_replace("/^[^\/]*+\//", "", $this->ReverseZone);

    /* Check for valid&complete IP address */
    if (!tests::is_ip($addr)) {
      $message[] = msgPool::invalid(_("Network address"));
    }

    // Ensure that we do not use reverse zones twice.
    //  this currently not supported by FusionDirectory.
    $zones = DNS::getDNSZoneEntries($this->config, $this->dn);
    switch ($this->NetworkClass) {
      case 'C' :
        $test = preg_replace("/\.[0-9]*$/", "", $this->ReverseZone);
        break;
      case 'B' :
        $test = preg_replace("/\.[0-9]*\.[0-9]*$/", "", $this->ReverseZone);
        break;
      case 'A' :
        $test = preg_replace("/\.[0-9]*\.[0-9]*\.[0-9]*$/", "", $this->ReverseZone);
        break;
      default : trigger_error("Invalid network class given '".$this->NetworkClass."'");
    }
    foreach ($zones as $name => $data) {
      if ($data['ReverseZone'] == $test && $data['zoneName'] != $this->InitialzoneName) {
        $message[] = msgPool::duplicated(_("Network address"));
      }
    }

    /* Check if given address matches selected network class */
    switch ($this->NetworkClass) {
      case 'A':
        if (!preg_match("/^[0-9]*\.0\.0\.0$/", $addr)) {
          $message[] = sprintf(_("The specified network address and the network class (%s/%s) do not match!"), $this->NetworkClass, "255.0.0.0");
        }
        break;
      case 'B':
        if (!preg_match("/^[0-9]*\.[0-9]*\.0\.0$/", $addr)) {
          $message[] = sprintf(_("The specified network address and the network class (%s/%s) do not match!"), $this->NetworkClass, "255.255.0.0");
        }
        break;
      case 'C':
        if (!preg_match("/^[0-9]*\.[0-9]*\.[0-9]*\.0$/", $addr)) {
          $message[] = sprintf(_("The specified network address and the network class (%s/%s) do not match!"), $this->NetworkClass, "255.255.255.0");
        }
        break;
      default : $message[] = msgPool::invalid(_("Network class"), $this->NetworkClass);
    }

    return $message;
  }

  /* Save to LDAP */
  function save()
  {
    $ret = array();
    foreach ($this->attributes as $name) {
      $ret[$name] = $this->$name;
    }

    /* Create mx records */
    foreach ($this->mXRecords as $key => $rec) {
      $this->Records[] = $rec;
    }

    $ret['RECORDS'] = $this->Records;

    switch ($this->NetworkClass) {
      case 'C' :
        $ret['ReverseZone'] = preg_replace("/\.[0-9]*$/", "", $this->ReverseZone);
        break;
      case 'B' :
        $ret['ReverseZone'] = preg_replace("/\.[0-9]*\.[0-9]*$/", "", $this->ReverseZone);
        break;
      case 'A' :
        $ret['ReverseZone'] = preg_replace("/\.[0-9]*\.[0-9]*\.[0-9]*$/", "", $this->ReverseZone);
        break;
      default : trigger_error("Invalid network class given '".$this->NetworkClass."'");
    }

    $ret['InitialReverseZone']  = $this->InitialReverseZone;
    $ret['InitialzoneName']     = $this->InitialzoneName;

    $ret['sOAmail']             = preg_replace("/\@/", ".", $this->sOAmail);

    foreach (array("sOAprimary","zoneName","sOAmail") as $attr) {

      if (($attr == 'zoneName') && !DNS::finalDot()) {
        continue;
      }

      // Append trailing dots if we've full qualified host names.
      if (!preg_match("/\.$/", $ret[$attr])) {
        if (!tests::is_ip($ret[$attr]) && preg_match("/\./", $ret[$attr])) {
          $ret[$attr] = $ret[$attr].".";
        }
      }
    }

    $ret['RECORDS'][] = array("type" => "nSRecord","value" => $ret['sOAprimary']);

    $ret['zoneEditor'] = $this->zoneEditor;
    return $ret;
  }


  /* This function generate a table row for each used record.
     This table row displays the recordtype in a select box
      and the specified value for the record, and a remove button.
     The last element of the table also got an 'add' button.
   */
  function generateRecordsList($changeStateForRecords = "")
  {
    $changeStateForRecords = "";

    $str = "<table summary=''>";
    foreach ($this->Records as $key => $entry) {
      if ($entry['type'] == "mXRecord") {
        continue;
      }

      $changeStateForRecords .= "changeState('RecordTypeSelectedFor_".$key."');\n";
      $changeStateForRecords .= "changeState('RecordValue_".$key."');\n";
      $changeStateForRecords .= "changeState('RemoveRecord_".$key."');\n";

      $str .= " <tr>".
        "   <td>".$this->generateRecordListBox($entry['type'], "RecordTypeSelectedFor_".$key)."</td>".
        "   <td><input type='text' value='".$entry['value']."' name='RecordValue_".$key."' id='RecordValue_".$key."'></td>".
        "   <td><input type='submit' name='RemoveRecord_".$key."' value='"._("Delete")."' id='RemoveRecord_".$key."'></td>".
        "</tr>";
    }

    $str .= "  <tr>".
      "    <td colspan=2></td><td>".
      "      <input type='submit' value='"._("Add")."' name='AddNewRecord'>".
      "    </td>".
      "  </tr>".
      "</table>";
    return $str;
  }

  /* This function generates a select box out of $this->RecordTypes options.
     The Parameter $selected is used to predefine an attribute.
     $name is used to specify a post name
   */
  function generateRecordListBox($selected, $name)
  {
    $str = "<select name='".$name."' id='".$name."'>";
    foreach ($this->RecordTypes as $type => $value) {
      if (preg_match("/^mXRecord$/i", $value)) {
        continue;
      }

      $use = "";
      if ($type == $selected) {
        $use = " selected ";
      }
      $str .= "\n <option value='".$type."' ".$use.">".strtoupper(preg_replace("/record/i", "", $type))."</option>";
    }
    $str .= "</select>";
    return $str;
  }
}
?>
