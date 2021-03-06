<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2014-2019  FusionDirectory

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
  function __construct (string $label, string $description, string $ldapName, $objectFilter = '', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, TRUE, ['server'], 'dn', 'nameAttr', $objectFilter, $acl);
    $this->setInLdap(FALSE);
  }
}

class systemImport extends simplePlugin
{
  protected $types = [];

  static function getAttributesInfo (string $filter = ''): array
  {
    return [
      'import' => [
        'template'  => get_template_path('server_import.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Import'),
        'attrs'     => [
          new BaseSelectorAttribute(get_ou('workstationRDN')),
          new ImportServerAttribute(
            _('Server'), _('The server you wish to import hosts from'),
            'apiServer', $filter
          ),
          new SelectAttribute(
            _('Type'), _('Type of objects you wish to import'),
            'type', TRUE,
            [], '', NULL
          ),
          new SelectAttribute(
            _('Template'), _('Select a template to apply to imported entries'),
            'template_dn', FALSE,
            [], '', NULL
          ),
          new ButtonAttribute(
            '', '',
            'import',
            _('Import')
          )
        ],
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->messages = FALSE;

    $this->attributesAccess['type']->setSubmitForm('typeChanged');
    $choices = [];
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

  function massImport ($hosts, $type, $template_dn, $base = NULL)
  {
    global $config, $ui;

    if ($base === NULL) {
      $base = $config->current['BASE'];
    }
    $ui->setCurrentBase($base);

    $messages = [];
    $dhcps    = [];
    try {
      $dns    = objects::ls('dnsZone');
    } catch (NonExistingObjectTypeException $e) {
      $dns    = [];
    }
    if (!empty($template_dn)) {
      $template = new template($type, $template_dn);
    }
    foreach ($hosts as $host) {
      if (!preg_match('/^([^.]+)\.(.*)$/', $host['id'], $m)) {
        $messages[$host['id']] = [sprintf(_('Could not parse %s'), $host['id'])];
        continue;
      }
      $cn   = $m[1];
      $zone = $m[2];

      if (!isset($template)) {
        $objectType = $type;
        if (isset($host['objectType'])) {
          $objectType = $host['objectType'];
        }
        $tabObject = objects::create($objectType);
        $baseObject = $tabObject->getBaseObject();
        $baseObject->cn           = $cn;
        $baseObject->description  = $host['description'];
        if (isset($host['interfaces'])) {
          $tabObject->by_object['interfacesManagement']->networkInterfaces = $host['interfaces'];
        }
        $tabObject->update();
      } else {
        $template->reset();
        // For now we only need to import workstations
        $values = [
          'workstationGeneric' => [
            'base'          => $base,
            'cn'            => $cn,
            'description'   => $host['description'],
          ],
          'interfacesManagement' => [
            'networkInterfaces' => $host['interfaces'],
          ],
        ];
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
              $dhcps[$dn] = [$attrs['cn'][0], normalize_netmask($attrs['dhcpNetMask'][0])];
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
                  [$dn, $baseObject->macAddress, $ipHostNumber, '']
                );
                $tabObject->by_object['dhcpSystem']->attributesAccess['dhcpHosts']->applyPostValue();
              }
            }
          }
          if (empty($tabObject->by_object['dhcpSystem']->dhcpHosts)) {
            $messages[$host['id']] = [sprintf(_('No DHCP server found for IPs %s'), implode(',', $baseObject->ipHostNumber))];
            continue;
          }
        }
      }

      if (class_available('dnsHost') && !$tabObject->by_object['dnsHost']->is_account) {
        /* Activate DNS if we find a matching server */
        foreach ($dns as $dn => $value) {
          if (preg_match('/^'.$zone.'\.?$/', $value)) {
            $tabObject->by_object['dnsHost']->is_account  = TRUE;
            $tabObject->by_object['dnsHost']->fdDNSZoneDn = [$dn];
            foreach ($baseObject->ipHostNumber as $ipHostNumber) {
              $tabObject->by_object['dnsHost']->addRecord($dn, [$cn, 'aRecord', $ipHostNumber, '']);
            }
            break;
          }
        }
        if ($tabObject->by_object['dnsHost']->is_account == FALSE) {
          $messages[$host['id']] = [sprintf(_('No DNS server found for zone %s'), $zone)];
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

  function typeChanged ()
  {
    $templates = objects::getTemplates($this->type);
    $this->attributesAccess['template_dn']->setChoices(array_keys($templates), array_values($templates));
  }

  public function render (): string
  {
    $smarty = get_smarty();
    $smarty->assign('importResult', $this->messages);
    return parent::render();
  }

  function save (): array
  {
    return [];
  }

  function handle_import ()
  {
    $hosts = $this->getHosts($this->apiServer);
    if ($hosts === FALSE) {
      return;
    }

    $this->messages = $this->massImport($hosts, $this->type, $this->template_dn, $this->base);
  }

  protected function getHosts ($server_dn)
  {
    die('Not implemented');
  }

  protected function getExtraTabs ($server_dn, $host): array
  {
    return [];
  }
}
