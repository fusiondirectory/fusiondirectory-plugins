<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2018  FusionDirectory

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
    parent::__construct($label, $description, $ldapName, TRUE, 'server', NULL, $filter, $acl);
    $this->setInLdap(FALSE);
  }
}

class systemImport extends simplePlugin
{
  protected $types = array();

  static function massImport ($hosts, $type, $template_dn, $base = NULL)
  {
    global $config;
    if ($base === NULL) {
      $base = $config->current['BASE'];
    }
    session::global_set("CurrentMainBase", $base);

    $messages = array();
    $dhcps    = array();
    try {
      $dns    = objects::ls('dnsZone');
    } catch (NonExistingObjectTypeException $e) {
      $dns    = array();
    }
    if (!empty($template_dn)) {
      $template = new template($type, $template_dn);
    }
    foreach ($hosts as $host) {
      if (!preg_match('/^([^.]+)\.(.*)$/', $host['id'], $m)) {
        $messages[$host['id']] = array(sprintf(_('Could not parse %s'), $host['id']));
        continue;
      }
      $cn   = $m[1];
      $zone = $m[2];
      if (empty($host['ipAddress'])) {
        $host['ipAddress'] = array();
      } elseif (!is_array($host['ipAddress'])) {
        $host['ipAddress'] = array($host['ipAddress']);
      }
      if (empty($host['hardwareAddress'])) {
        $host['hardwareAddress'] = array();
      } elseif (!is_array($host['hardwareAddress'])) {
        $host['hardwareAddress'] = array($host['hardwareAddress']);
      }

      if (empty($template_dn)) {
        $objectType = $type;
        if (isset($host['objectType'])) {
          $objectType = $host['objectType'];
        }
        $tabObject = objects::create($objectType);
        $tabObject->set_acl_base($base);
        $baseObject = $tabObject->getBaseObject();
        $baseObject->cn           = $cn;
        $baseObject->description  = $host['description'];
        $baseObject->ipHostNumber = $host['ipAddress'];
        $baseObject->macAddress   = $host['hardwareAddress'];
      } else {
        $template->reset();
        // For now we only need to import workstations
        $values = array(
          'workstationGeneric' => array(
            'base'          => $base,
            'cn'            => $cn,
            'description'   => $host['description'],
            'ipHostNumber'  => $host['ipAddress'],
            'macAddress'    => $host['hardwareAddress'],
          )
        );
        $error = $template->deserialize($values);
        if ($error !== TRUE) {
          $messages[$host['id']] = $error;
          continue;
        }
        $tabObject  = $template->apply();
        $baseObject = $tabObject->getBaseObject();
      }

      if (class_available('dhcpSystem') && !$tabObject->by_object['dhcpSystem']->is_account) {
        /* Get existing DHCP servers */
        if (empty($dhcps)) {
          list(, $dhcpParentNodes) = dhcpSystem::getDhcpParentNodes();
          foreach ($dhcpParentNodes as $dn => $attrs) {
            if (isset($attrs['dhcpNetMask'])) {
              $dhcps[$dn] = array($attrs['cn'][0], normalize_netmask($attrs['dhcpNetMask'][0]));
            }
          }
        }
        /* Activate DHCP if we find a matching server */
        if (!empty($baseObject->ipHostNumber)) {
          foreach ($dhcps as $dn => $infos) {
            list($ip, $mask) = $infos;
            foreach ($baseObject->ipHostNumber as $ipHostNumber) {
              if (tests::is_in_network($ip, $mask, $ipHostNumber)) {
                $tabObject->by_object['dhcpSystem']->attributesAccess['dhcpHosts']->addPostValue(
                  array($dn, $baseObject->macAddress, $ipHostNumber, '')
                );
                $tabObject->by_object['dhcpSystem']->attributesAccess['dhcpHosts']->applyPostValue();
              }
            }
          }
          if (empty($tabObject->by_object['dhcpSystem']->dhcpHosts)) {
            $messages[$host['id']] = array(sprintf(_('No DHCP server found for IPs %s'), implode(',', $baseObject->ipHostNumber)));
            continue;
          }
        }
      }

      if (class_available('dnsHost') && !$tabObject->by_object['dnsHost']->is_account) {
        /* Activate DNS if we find a matching server */
        foreach ($dns as $dn => $value) {
          if (preg_match('/^'.$zone.'\.?$/', $value)) {
            $tabObject->by_object['dnsHost']->is_account  = TRUE;
            $tabObject->by_object['dnsHost']->fdDNSZoneDn = array($dn);
            foreach ($baseObject->ipHostNumber as $ipHostNumber) {
              $tabObject->by_object['dnsHost']->addRecord($dn, array($cn, 'aRecord', $ipHostNumber, ''));
            }
            break;
          }
        }
        if ($tabObject->by_object['dnsHost']->is_account == FALSE) {
          $messages[$host['id']] = array(sprintf(_('No DNS server found for zone %s'), $zone));
          continue;
        }
      }

      /* Fill other tabs as asked */
      $otherTabs = $this->getExtraTabs($this->apiServer, $host);
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
      $message = $tabObject->save();

      if (count($message) == 0) {
        /* No errors, save object */
        $messages[$host['id']] = TRUE;
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
          new BaseSelectorAttribute(get_ou('workstationRDN')),
          new ImportServerAttribute(
            _('Server'), _('The server you wish to import hosts from'),
            'apiServer', $filter
          ),
          new SelectAttribute(
            _('Type'), _('Type of objects you wish to import'),
            'type', TRUE,
            array(), '', NULL
          ),
          new SelectAttribute (
            _('Template'), _('Select a template to apply to imported entries'),
            'template_dn', FALSE,
            array(), '', NULL
          ),
          new ButtonAttribute (
            '', '',
            'import',
            _('Import')
          )
        ),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->messages = FALSE;

    $this->attributesAccess['type']->setSubmitForm('typeChanged');
    $choices = array();
    foreach ($this->types as $type) {
      $infos = objects::infos($type);
      $choices[$type] = $infos['name'];
    }
    asort($choices);
    $this->attributesAccess['type']->setChoices(array_keys($choices), array_values($choices));
    $this->attributesAccess['type']->resetToDefault();
    if (count($choices) == 1) {
      $this->attributesAccess['type']->setVisible(FALSE);
    }
  }

  function typeChanged()
  {
    $templates = objects::getTemplates($this->type);
    $this->attributesAccess['template_dn']->setChoices(array_keys($templates), array_values($templates));
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

  function handle_import ()
  {
    $hosts = $this->getHosts($this->apiServer);
    if ($hosts === FALSE) {
      return;
    }

    $this->messages = systemImport::massImport($hosts, $this->type, $this->template_dn, $this->base);
  }

  protected function getHosts($server_dn)
  {
    die('Not implemented');
  }

  protected function getExtraTabs($server_dn, $host)
  {
    return array();
  }
}
?>
