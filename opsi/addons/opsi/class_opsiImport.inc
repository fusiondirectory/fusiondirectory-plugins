<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011-2013 FusionDirectory project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class OPSIServerAttribute extends SelectAttribute
{
  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->updateChoices();
    }
  }

  function updateChoices()
  {
    $ldap = $this->plugin->config->get_ldap_link();
    $ldap->cd($this->plugin->config->current['BASE']);
    $ldap->search('(objectClass=opsiServer)', array('cn'));
    $serversdn    = array();
    $serverslabel = array();
    while ($attrs = $ldap->fetch()) {
      $serversdn[]    = $attrs['dn'];
      $serverslabel[] = $attrs['cn'][0];
    }
    $this->setChoices($serversdn, $serverslabel);
  }
}

class ImportOPSIAttribute extends OPSIServerAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, array(), "", NULL, $acl);
    $this->setInLdap(FALSE);
  }

  function loadPostValue ()
  {
    $id = $this->getHtmlId();
    if (isset($_POST[$id.'_action'])) {
      parent::loadPostValue();
    }
  }

  function applyPostValue ()
  {
    if (!$this->disabled && $this->postValue != '') {
      if (in_array($this->postValue, $this->choices)) {
        $this->plugin->opsiMassImport($this->postValue);
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $disabled = ($this->disabled?' disabled="disabled"':'');
    $button = '<input type="submit" name="'.$id.'_action" id="'.$id.'_action" value="{t}Import{/t}"'.$disabled.'/>';
    return parent::renderFormInput().$this->renderAcl($button);
  }
}

class opsiImport extends simplePlugin
{
  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI import'),
      'plDescription' => _('Import windows stations from OPSI into FD'),
      'plIcon'        => 'plugins/opsi/images/icon.png',
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('opsiImport' => array('name' => _('OPSI import'), 'filter' => '')),
      'plSection'     => 'addons',
      'plPriority'    => 6,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'import' => array(
        'template'  => get_template_path('opsiimport.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Import'),
        'attrs'     => array(
          new BaseSelectorAttribute(get_ou('sambaMachineAccountRDN')),
          new ImportOPSIAttribute(
            _('OPSI Server'), _('The OPSI server you wish to import host from'),
            'opsiServer', FALSE
          ),
        ),
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->messages = FALSE;
  }

  function execute()
  {
    $infos = "";
    if ($this->messages !== FALSE) {
      foreach ($this->messages as $id => $message) {
        if (is_array($message)) {
          $infos .= '<tr class="error">';
          $infos .= '<td><img src="geticon.php?context=status&icon=task-failure&size=16"/></td>';
          $infos .= '<td>'.$id.'</td>';
          $infos .= '<td>';
          foreach ($message as $m) {
            $infos .= "$m<br/>\n";
          }
        } else {
          $infos .= '<tr class="success">';
          $infos .= '<td><img src="geticon.php?context=status&icon=task-complete&size=16"/></td>';
          $infos .= '<td>'.$id.'</td>';
          $infos .= '<td>saved</td>';
        }
        $infos .= "</td></tr>\n";
      }
    }
    if (!empty($infos)) {
      $infos = '<table class="listingTable"><thead><tr>'.
                '<th><img src="images/info_small.png"/></th><th>Host</th><th>Details</th></tr></thead>'.
                '<tbody>'.$infos.'</tbody></table>';
    }
    $smarty = get_smarty();
    $smarty->assign('opsiImport_result', $infos);
    return parent::execute();
  }


  function save()
  {
  }

  function opsiMassImport ($server_dn)
  {
    session::global_set("CurrentMainBase", $this->base);
    $hosts = self::opsiGetHosts($this->config, $server_dn);
    if ($hosts === FALSE) {
      return;
    }
    $plInfo = pluglist::pluginInfos('opsiClient'); // Just to get the objectType dynamically
    $key = key($plInfo['plObjectType']);
    if (is_numeric($key)) {
      $key = $plInfo['plObjectType'][$key];
    }

    $tabs = array(
      'opsiClient' => array(
        'is_account'      => TRUE,
        'fdOpsiServerDn'  => $server_dn,
      )
    );
    $this->messages = systemImport::massImport($hosts, $key, $this->base, $tabs);
  }

  static function opsiGetHosts ($config, $server_dn)
  {
    $s_daemon = new supportDaemon();
    if (!$s_daemon->is_available()) {
      msg_dialog::display(
        _("Argonaut server is not available"), $s_daemon->get_error(), ERROR_DIALOG
      );
      return FALSE;
    }
    $ldap = $config->get_ldap_link();
    $ldap->cat($server_dn);
    if ($attrs = $ldap->fetch()) {
      $macAddress = $attrs['macAddress'][0];
    } else {
      msg_dialog::display(
        _("Could not find $server_dn in the LDAP"),
        msgPool::ldaperror($ldap->get_error(), $server_dn), ERROR_DIALOG
      );
      return FALSE;
    }
    $res = $s_daemon->append_call("OPSI.host_getObjects", array($macAddress), array('args' => array(array(), array('type' => 'OpsiClient'))));
    if ($s_daemon->is_error()) {
      msg_dialog::display(
        _("Could not update OPSI informations"),
        msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
      );
      return FALSE;
    }
    return $res;
  }
}
?>
