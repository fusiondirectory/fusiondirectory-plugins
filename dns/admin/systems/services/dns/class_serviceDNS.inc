<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class serviceDNS extends goService
{
  /* attribute list for save action */
  var $ignore_account   = FALSE;
  var $attributes       = array();
  var $objectclasses    = array("whatever");

  var $Zones            = array();

  var $orig_dn          = "";

  var $initially_was_account;

  /* ServerService tab vars */
  var $conflicts        = array("serviceDNS");
  var $DisplayName      = "";
  var $StatusFlag       = "";
  var $view_logged      = FALSE;

  function serviceDNS (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->DisplayName = _("DNS service");

    $this->orig_dn = $dn;

    $this->initially_was_account = $this->is_account;
  }

  function is_this_account($attrs)
  {
    if (!isset($attrs['dn'])) {
      return FALSE;
    }
    /* Get all zone Informations */
    $this->Zones = DNS::getDNSZoneEntries($this->config, $attrs['dn']);

    /* If there is at least one entry in this -> types, we have DNS enabled
     */
    if (count($this->Zones) == 0) {
      return FALSE;
    } else {
      return TRUE;
    }
  }

  function execute()
  {
    /* Call parent execute
     */
    plugin::execute();

    if ($this->is_account && !$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view", "server/".get_class($this), $this->dn);
    }

    /* Fill templating stuff
     */
    $smarty = get_smarty();
    $smarty->assign("is_createable", $this->acl_is_createable());
    $display = "";


    $this->initially_was_account = $this->is_account;

    /* Do we need to flip is_account state? */
    if (isset($_POST['modify_state'])) {
      $this->is_account = !$this->is_account;
    }

    /* Edited or Added zone */
    if (isset($_POST['SaveZoneChanges'])) {
      $this->dialog->save_object();

      /* Check for errors */
      if (count($this->dialog->check())) {
        foreach ($this->dialog->check() as $msgs) {
          msg_dialog::display(_("Error"), $msgs, ERROR_DIALOG);
        }
      } else {
        /* add new/edited zone */
        $ret = $this->dialog->save();
        if (!$this->dialog->isNew) {
          unset($this->Zones[$this->dialog->OldZoneName]);
        }
        $this->Zones[$ret['zoneName']]  = $ret;
        $this->dialog                   = FALSE;
      }
    }

    /* Cancel zone edit / new */
    if (isset($_POST['CancelZoneChanges'])) {
      $this->dialog = FALSE;
    }

    /* Add empty new zone */
    if (isset($_POST['AddZone'])) {
      $this->dialog = new servdnseditZone($this->config, $this->dn);
      if ($this->is_new) {
        $this->dialog->acl_base = $this->acl_base;
      }
    }

    /* Check for edit zone request */
    foreach ($_POST as $name => $value) {
      /* check all post for edit request */
      if (preg_match("/^editZone_/", $name)) {
        $tmp  = preg_replace("/^editZone_/", "", $name);
        $tmp  = base64_decode(preg_replace("/_.*$/", "", $tmp));
        $this->dialog = new servdnseditZone($this->config, $this->dn, $this->Zones[$tmp]);
        break;
      }

      /* check posts for delete zone */
      if (preg_match("/^delZone_/", $name)) {
        $tmp  = preg_replace("/^delZone_/", "", $name);
        $tmp  = base64_decode(preg_replace("/_.*$/", "", $tmp));

        /* Initiate ldap2zone */
        $this->RemoveZone($tmp);
        break;
      }

      /* check posts for ldap2zone */
      if (preg_match("/^ldap2Zone_/", $name)) {
        $tmp  = preg_replace("/^ldap2Zone_/", "", $name);
        $tmp  = base64_decode(preg_replace("/_.*$/", "", $tmp));

        /* Initiate deletion */
        $this->Ldap2Zone($tmp);
        break;
      }
    }

    if (isset($_GET['act']) && ($_GET['act'] == "edit") && isset($_GET['id'])) {
      $id = base64_decode($_GET['id']);
      if (isset($this->Zones[$id])) {
         $this->dialog = new servdnseditZone($this->config, $this->dn, $this->Zones[$id]);
      }
    }

    if (isset($_GET['act']) && ($_GET['act'] == "edit") && isset($_GET['id'])) {
      $id = base64_decode($_GET['id']);
      if (isset($this->Zones[$id])) {
         $this->dialog = new servdnseditZone($this->config, $this->dn, $this->Zones[$id]);
      }
    }

    /* Show dialog */
    if (is_object($this->dialog)) {
      $this->dialog->save_object();
      $this->dialog->parent = $this;
      return $this->dialog->execute();
    }

    /* Create Listbox with existing Zones */
    $ZoneList = new divSelectBox("dNSZones");
    $ZoneList->SetHeight(254);

    /* Add entries to divselectbox */
    $editImg = "<input type='image' src='geticon.php?context=actions&icon=document-edit&size=16' name='editZone_%s' alt='edit' title='edit'>";
    if ($this->acl_is_removeable()) {
      $editImg .= "<input type='image' src='geticon.php?context=actions&icon=edit-delete&size=16' name='delZone_%s' alt='delete' title='delete'>";
    }
    $editImg .= "<input type='image' src='geticon.php?context=actions&icon=view-refresh&size=16' name='ldap2Zone_%s' alt='ldap2zone' title='ldap2zone'>";

    $link = "<a href='?plug=".$_GET['plug']."&act=edit&id=%s'>%s</a>";
    foreach ($this->Zones as $zone => $values) {
      $ZoneList->AddEntry(
        array(
          array("html"    => sprintf($link, base64_encode($zone), $zone)),
          array("html"    => sprintf($link, base64_encode($zone), _("Reverse zone")." : ".($values['ReverseZone']))),
          array("string"  => _("TTL")." : ".$values['sOAttl']),
          array("string"  => _("Class")." : ".$values['dNSClass']),
          array("html"    => str_replace("%s", base64_encode($zone), $editImg))
        )
      );
    }

    /* Display tempalte */
    $smarty->assign("ZoneList", $ZoneList->DrawList());
    $display .= $smarty->fetch(get_template_path('serviceDNS.tpl', TRUE, dirname(__FILE__)));
    return $display;
  }


  /* Delete specified zone
   */
  function RemoveZone($id, $force = FALSE)
  {
    $zones = DNS::getUsedZoneNames();

    if (isset($this->Zones[$id]['InitialReverseZone'])) {
      $rev = DNS::FlipIp($this->Zones[$id]['InitialReverseZone']);
    } else {
      $rev = DNS::FlipIp($this->Zones[$id]['ReverseZone']);
    }

    $zonename = "";
    if (isset($this->Zones[$id]['InitialzoneName'])) {
      $zonename = $this->Zones[$id]['InitialzoneName'];
    }

    $used = array();

    /* Add Records which use this zoneName */
    if (isset($zones[$zonename])) {
      $used = array_merge($used, $zones[$zonename]);
    }

    /* Add Records which uses this reverse zone */
    if (isset($zones[$rev.DNS::getInAddrArpa()])) {
      $used = array_merge($used, $zones[$rev.DNS::getInAddrArpa()]);
    }

    /* There are still entries using this configuration
     *  Abort deletion
     */
    if (count($used) && !$force) {
      $i    = 2;
      $str  = "";
      foreach ($used as $dn) {
        if (($i > 0) && !preg_match("/,relativeDomainName=/", $dn)) {
          $i--;
          $name = preg_replace("/^[^=]+=([^,]*),.*$/", "\\1", $dn);
          $zone = preg_replace("/^.*zoneName=([^,]*),.*$/", "\\1", $dn);
          $str  .= $name.".".$zone." ";
        }
      }

      /*  Only show 2 dns in the error message */
      if (count($used) > 2) {
        $str .= " ... ";
      }
      msg_dialog::display(_("Error"), sprintf(_("Cannot delete the selected zone. It is still in use by '%s'"), trim($str)), ERROR_DIALOG);
      return FALSE;
    } else {
      unset($this->Zones[$id]);
      return TRUE;
    }
  }

  /* This funtion calls ldap2zone */
  function Ldap2Zone($zone)
  {
    $o_queue = new supportDaemon();
    if ($o_queue->is_error()) {
      msg_dialog::display(_("Error"), sprintf(_("Was not able to contact argonaut server: %s"), "<br/><br/>".$o_queue->get_error()), ERROR_DIALOG);
    } else {
      $o_queue->append_call("Ldap2Zone.start",
                            array($this->attrs['macAddress'][0]),
                            array("args" => array($zone)));
      if ($o_queue->is_error()) {
        msg_dialog::display(_("Error"), sprintf(_("Was not able to launch ldap2zone: %s"), "<br/><br/>".$o_queue->get_error()), ERROR_DIALOG);
      }
    }
  }

  /* Remove dns service */
  function remove_from_parent()
  {
    if ($this->initially_was_account) {
      $bool             = TRUE;
      $this->is_account = FALSE;
      foreach ($this->Zones as $key => $zone) {
        $bool = $bool & $this->RemoveZone($key, TRUE);
      }
      if ($bool) {
        $this->save();
      }
      return $bool;
    }
  }

  /* Save to LDAP */
  function save()
  {
    /* Save zone editor changes now */
    foreach ($this->Zones as $name => $zone) {
      if (isset($zone['zoneEditor']) && ($zone['zoneEditor'] != NULL) && is_object($zone['zoneEditor'])) {
        $zone['zoneEditor']->save();
        unset($this->Zones[$name]['zoneEditor']);;
      }
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    /* Get differences
     */
    $old_dn = $this->orig_dn;
    if ($old_dn == "new") {
      $old_dn = $this->dn;
    }

    /* Update dns to current object dn */
    $tmp = DNS::getDNSZoneEntriesDiff($this->config, $this->Zones, $old_dn);
    $tmp2 = array();
    foreach ($tmp as $key1 => $data1) {
      $tmp2[$key1] = array();
      foreach ($data1 as $key2 => $data2) {
        $tmp2[$key1][preg_replace("/".preg_quote($old_dn, '/')."$/", $this->dn, $key2)] = $data2;
      }
    }
    $tmp = $tmp2;

    /* Updated zone entries if reverser or forward name has changed
     * Must be done before moving entries, else the given dn is invalid
     */
    if (isset($tmp['zoneUpdates'])) {
      foreach ($tmp['zoneUpdates'] as $dn => $attrs) {
        $ldap->cd($dn);
        $ldap->modify($attrs);
        new log("modify", "unknown/".get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());
        if (!$ldap->success()) {
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()), LDAP_ERROR);
        }
      }
    }

    /* Delete dns */
    foreach ($tmp['del'] as $dn => $del) {
      $for = $del['InitialzoneName'];
      $rev = DNS::FlipIp($del['InitialReverseZone']).DNS::getInAddrArpa();

      $ldap->cd($dn);
      $ldap->rmdir_recursive($dn);
      new log("remove", "unknown/".get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()), LDAP_ERROR);
      }
    }

    /* move follwoing entries */
    foreach ($tmp['move'] as $src => $dst) {
      $this->move($src, $dst);
    }

    /* Add || Update new DNS entries */
    foreach ($tmp['add'] as $dn => $attrs) {
      $ldap->cd($dn);
      $ldap->cat($dn, array('dn'));
      if ($ldap->fetch()) {
        $ldap->cd($dn);
        $ldap->modify ($attrs);
        if (!$ldap->success()) {
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()), LDAP_ERROR);
        }
      } else {
        $ldap->cd($dn);
        $ldap->add($attrs);
        if (!$ldap->success()) {
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_ADD, get_class()), LDAP_ERROR);
        }
      }
    }
    $this->handle_post_events("modify");
  }

  function getListEntry()
  {
    $fields             = goService::getListEntry();
    $fields['Message']  = _("DNS service");
    return $fields;
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS service'),
      'plDescription' => _('DNS service').' ('._('Services').')',
      'plIcon'        => 'plugins/dns/images/iconMini.png',
      'plPriority'    => 83,
      'plCategory'    => array('server'),

      'plProvidedAcls' => array(
        'start'         => _('Start service'),   // Remove this to hide the start button at all.
        'stop'          => _('Stop service'),    // Remove this to hide the stop button at all.
        'restart'       => _('Restart service'), // Remove this to hide the restart button at all.

        'zoneName'      => _('Zone name'),
        'ReverseZone'   => _('Reverse zone'),
        'NetworkClass'  => _('Network class'),
        'zoneEditor'    => _('Zone entry editor'),
        'sOAprimary'    => _('Primary dns server'),
        'sOAmail'       => _('Mail address'),
        'sOAserial'     => _('Serial'),
        'sOArefresh'    => _('Refresh'),
        'sOAretry'      => _('Retry'),
        'sOAexpire'     => _('Expire'),
        'sOAttl'        => _('TTL'),
        'mXRecord'      => _('MX records'),
        'zoneRecords'   => _('Zone records')
      )
    );
  }

}
?>
