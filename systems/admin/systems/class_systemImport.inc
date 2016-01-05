<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016  FusionDirectory

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

class ImportServerAttribute extends ObjectSelectAttribute
{
  function __construct ($label, $description, $ldapName, $filter, $acl = "")
  {
    parent::__construct($label, $description, $ldapName, FALSE, 'server', NULL, $filter, $acl);
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
        $this->plugin->massImport($this->postValue);
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

class systemImport extends simplePlugin
{
  protected $defaultType = '';

  static function massImport ($hosts, $defaultObjectType, $base = NULL, $otherTabs = array())
  {
    global $config;
    if ($base === NULL) {
      $base = $config->current['BASE'];
    }
    session::global_set("CurrentMainBase", $base);

    $messages = array();
    $dhcps    = array();
    $dns      = DNS::getAvailableZones($config);
    foreach ($hosts as $host) {
      if (!preg_match('/^([^.]+)\.(.*)$/', $host['id'], $m)) {
        $messages[$host['id']] = array(sprintf(_('Could not parse %s'), $host['id']));
        continue;
      }
      $cn   = $m[1].'$';
      $zone = $m[2].'.';

      $objectType = $defaultObjectType;
      if (isset($host['objectType'])) {
        $objectType = $host['objectType'];
      }
      $tabObject = objects::create($objectType);
      $tabObject->set_acl_base($base);
      $baseObject = $tabObject->getBaseObject();
      $baseObject->cn   = $cn;
      $baseObject->uid  = $baseObject->cn;
      $baseObject->description  = $host['description'];
      $baseObject->ipHostNumber = $host['ipAddress'];
      $baseObject->macAddress   = $host['hardwareAddress'];

      /* Get existing DHCP servers */
      if (empty($dhcps)) {
        $dhcpParentNodes = $baseObject->netConfigDNS->dhcpParentNodes;
        $ldap = $config->get_ldap_link();
        foreach ($dhcpParentNodes as $dn => $value) {
          $ldap->cat($dn);
          if ($attrs = $ldap->fetch()) {
            if (isset($attrs['dhcpNetMask'])) {
              $dhcps[$dn] = array($attrs['cn'][0],normalize_netmask($attrs['dhcpNetMask'][0]));
            }
          }
        }
      }
      /* Activate DHCP if we find a matching server */
      if (!empty($baseObject->ipHostNumber)) {
        $baseObject->netConfigDNS->dhcp_is_Account = FALSE;
        foreach ($dhcps as $dn => $infos) {
          list($ip, $mask) = $infos;
          if (tests::is_in_network($ip, $mask, $baseObject->ipHostNumber)) {
            $baseObject->netConfigDNS->dhcp_is_Account  = TRUE;
            $baseObject->netConfigDNS->dhcpParentNode   = $dn;
            break;
          }
        }
        if ($baseObject->netConfigDNS->dhcp_is_Account == FALSE) {
          $messages[$host['id']] = array(sprintf(_('No DHCP server found for ip %s'), $baseObject->ipHostNumber));
          continue;
        }
      }
      /* Activate DNS if we find a matching server */
      $baseObject->netConfigDNS->DNS_is_account = FALSE;
      foreach ($dns as $key => $value) {
        if (preg_match('/'.$zone.'$/', $value)) {
          $baseObject->netConfigDNS->DNS_is_account       = TRUE;
          $baseObject->netConfigDNS->dnsEntry['zoneName'] = $value;
          break;
        }
      }
      if ($baseObject->netConfigDNS->DNS_is_account == FALSE) {
        $messages[$host['id']] = array(sprintf(_('No DNS server found for zone %s'), $zone));
        continue;
      }

      /* Fill other tabs as asked */
      foreach ($otherTabs as $tab => $fields) {
        if (isset($tabObject->by_object[$tab])) {
          foreach ($fields as $field => $value) {
            $tabObject->by_object[$tab]->$field = $value;
          }
        } else {
          die("Tab $tab is not existing");
        }
      }

      /* Perform checks */
      $message = $tabObject->check();

      if (count($message) == 0) {
        /* No errors, save object */
        $messages[$host['id']] = TRUE;
        $tabObject->save();
      } else {
        /* Errors found */
        $messages[$host['id']] = $message;
      }
    }
    return $messages;
  }

  static function getAttributesInfo($filter = '')
  {
    return array(
      'import' => array(
        'template'  => get_template_path('server_import.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Import'),
        'attrs'     => array(
          new BaseSelectorAttribute(get_ou('sambaMachineAccountRDN')),
          new ImportServerAttribute(
            _('Server'), _('The server you wish to import hosts from'),
            'apiServer', $filter
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
    $smarty = get_smarty();
    $smarty->assign('importResult', $this->messages);
    return parent::execute();
  }

  function save()
  {
  }

  function systemMassImport ($server_dn)
  {
    $hosts = $this->getHosts($server_dn);
    if ($hosts === FALSE) {
      return;
    }

    $this->messages = systemImport::massImport($hosts, $this->defaultType, $this->base, $this->getExtraTabs($server_dn));
  }

  protected function getHosts($server_dn)
  {
    die('Not implemented');
  }

  protected function getExtraTabs($server_dn)
  {
    return array();
  }
}
?>
