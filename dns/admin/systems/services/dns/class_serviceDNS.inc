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

class DnsZoneDialog extends GenericDialog
{
  protected $post_cancel = 'CancelZoneChanges';
  protected $post_finish = 'SaveZoneChanges';
  protected $dialogClass = 'servdnseditZone';

  protected $initialDialogValue = NULL;

  function __construct($simplePlugin, &$attribute, $value = array())
  {
    $this->attribute      = $attribute;
    $this->dialog         = new $this->dialogClass($simplePlugin->config, $simplePlugin->dn, $value);
    $this->dialog->parent = $simplePlugin;
    if (!empty($value)) {
      $this->initialDialogValue = $value;
    }
  }

  function dialog_execute ()
  {
    $this->dialog->save_object();
    return $this->dialog->execute();
  }

  function handle_finish ()
  {
    $this->dialog->save_object();

    /* Check for errors */
    $checks = $this->dialog->check();
    if (count($checks)) {
      foreach ($checks as $msg) {
        msg_dialog::display(_('Error'), $msg, ERROR_DIALOG);
      }
      return $this->dialog->execute();
    } else {
      /* add new/edited zone */
      $ret = $this->dialog->save();
      $this->attribute->addValue($ret);
      return FALSE;
    }
  }

  function handle_cancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addValue($this->initialDialogValue);
    }
    return FALSE;
  }
}

class DnsZonesAttribute extends DialogOrderedArrayAttribute
{
  protected $dialogClass = 'DnsZoneDialog';

  protected function getAttributeArrayValue($key, $value)
  {
    return array(
      $value['zoneName'],
      sprintf(_('Reverse zone: %s'), $value['ReverseZone']),
      sprintf(_('TTL: %s'), $value['sOAttl']),
    );
  }

  protected function loadAttrValue ($attrs)
  {
  }

  /* Not saving anything into base node */
  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
  }

  protected function genRowIcons($key, $value)
  {
    $id = $this->getHtmlId();
    list ($img, $width) = parent::genRowIcons($key, $value);
    $img .= $this->renderInputField(
      'image', $id.'_ldap2zone_'.$key,
      array(
        'src'   => 'geticon.php?context=actions&amp;icon=view-refresh&amp;size=16',
        'title' => _('ldap2zone'),
        'alt'   => _('ldap2zone'),
        'class' => 'center'
      )
    ).'&nbsp;';
    $width += 15;

    return array ($img, $width);
  }

  protected function handlePostValueActions($id, $postValue)
  {
    if (parent::handlePostValueActions($id, $postValue)) {
      return TRUE;
    }
    if (preg_match('/^'.$id.'_ldap2zone_/', $postValue)) {
      $key = preg_replace('/^'.$id.'_ldap2zone_/', '', $postValue);
      $key = preg_replace('/_[xy]$/', '', $key);

      $this->plugin->callLdap2zone($this->value[$key]['zoneName']);
      return TRUE;
    }
    return FALSE;
  }
}

class serviceDNS extends simpleService
{
  var $objectclasses    = array();

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('DNS zones'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new DnsZonesAttribute (
            '', _('The DNS zones'),
            'dnsZones'
          )
        )
      ),
    );
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS service'),
      'plDescription' => _('DNS service'),
      'plIcon'        => 'plugins/dns/images/iconMini.png',

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

  function is_this_account($attrs)
  {
    if (!isset($attrs['dn'])) {
      return FALSE;
    }
    /* Get all zone Informations */
    $this->dnsZones = array_values(DNS::getDNSZoneEntries($this->config, $attrs['dn']));

    /* If there is at least one entry in dnsZones, we have DNS enabled */
    if (count($this->dnsZones) == 0) {
      return FALSE;
    } else {
      return TRUE;
    }
  }

  function ldap_save ($cleanup = TRUE)
  {
    /* Save zone editor changes now */
    $dnsZones = $this->dnsZones;
    foreach ($dnsZones as $key => $zone) {
      if (isset($zone['zoneEditor']) && ($zone['zoneEditor'] != NULL) && is_object($zone['zoneEditor'])) {
        $zone['zoneEditor']->save();
        unset($dnsZones[$key]['zoneEditor']);
      }
    }
    $this->dnsZones = $dnsZones;

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    /* Get differences */
    $old_dn = $this->orig_dn;
    if ($old_dn == "new") {
      $old_dn = $this->dn;
    }

    /* Update dns to current object dn */
    $tmp = DNS::getDNSZoneEntriesDiff($this->config, $this->dnsZones, $old_dn);
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

    /* move following entries */
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
  }

  protected function prepare_remove ()
  {
    if ($this->initially_was_account) {
      $this->handle_pre_events('remove');
    }
  }

  protected function ldap_remove ()
  {
    $ldap = $this->config->get_ldap_link();
    if ($this->initially_was_account) {
      foreach ($this->dnsZones as $zone) {
        $zonedn = "zoneName=".$zone['InitialzoneName'].",".$this->dn;
        $ldap->rmdir_recursive($zonedn);
      }
    }
    return TRUE;
  }

  /* This function calls ldap2zone */
  function callLdap2Zone($zone)
  {
    $o_queue = new supportDaemon();
    if ($o_queue->is_error()) {
      msg_dialog::display(_('Error'), sprintf(_('Was not able to contact argonaut server: <br/><br/>%s'), $o_queue->get_error()), ERROR_DIALOG);
    } else {
      $o_queue->append_call(
        'Ldap2Zone.start',
        array($this->attrs['macAddress'][0]),
        array('args' => array($zone))
      );
      if ($o_queue->is_error()) {
        msg_dialog::display(_('Error'), sprintf(_('Was not able to launch ldap2zone: <br/><br/>%s'), $o_queue->get_error()), ERROR_DIALOG);
      }
    }
  }
}
?>
