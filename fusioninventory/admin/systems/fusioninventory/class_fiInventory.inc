<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2019  FusionDirectory

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

class fiInventory extends simplePlugin
{
  var $displayHeader  = FALSE;

  protected $filter = '';

  static $sections = array (
    'antivirus'       => 'software',
    'batteries'       => 'hardware',
    'controllers'     => 'hardware',
    'cpus'            => 'hardware',
    'drives'          => 'hardware',
    'envs'            => 'software',
    'inputs'          => 'hardware',
    'memories'        => 'hardware',
    'modems'          => 'hardware',
    'monitors'        => 'hardware',
    'networks'        => 'hardware',
    'ports'           => 'hardware',
    'processes'       => 'process',
    'registry'        => 'software',
    'rudder'          => 'software',
    'slots'           => 'software',
    'softwares'       => 'software',
    'sounds'          => 'hardware',
    'storages'        => 'hardware',
    'videos'          => 'hardware',
    'usbdevices'      => 'hardware',
    'users'           => 'users',
    'localusers'      => 'users',
    'localgroups'     => 'users',
    'printers'        => 'hardware',
    'bios'            => 'software',
    'hardware'        => 'hardware',
    'operatingsystem' => 'software',
    'accesslog'       => 'software',
    'virtualmachines' => 'software',
    'logicalvolumes'  => 'software',
    'physicalvolumes' => 'hardware',
    'volumegroups'    => 'software',
    'licenseinfos'    => 'software',
    'versionprovider' => 'other',
  );

  /* Return plugin information */
  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Inventory'),
      'plDescription' => _('Inventory Viewer'),
      'plPriority'    => 90,
      'plObjectClass' => array('fdInventoryContent'),
      'plObjectType'  => array(
        'workstation', 'server',
        'inventory' => array(
          'description' => _('Inventory object'),
          'mainAttr'    => 'cn',
          'icon'        => 'geticon.php?context=applications&icon=fusioninventory&size=16',
          'name'        => _('Inventory object'),
          'ou'          => get_ou('inventoryRDN'),
          'tabClass'    => 'simpleTabs_noSpecial',
        )
      ),
      'plSearchAttrs' => array('cn','ipHostNumber','macAddress'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'match' => array(
        'name'      => _('Matching system'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new DisplayAttribute(_('System'), _('System entry matching this inventory'), 'matching'),
          new HiddenArrayAttribute('macAddress'),
          new HiddenArrayAttribute('ipHostNumber'),
          new HiddenArrayAttribute('cn'),
        ),
      ),
      'hardware' => array(
        'name'      => _('Hardware'),
        'class'     => array('fullwidth'),
        'attrs'     => array(new FakeAttribute('hardware_infos')),
        'template'  => get_template_path('inventory.tpl', TRUE, dirname(__FILE__)),
      ),
      'software' => array(
        'name'      => _('Software'),
        'class'     => array('fullwidth'),
        'attrs'     => array(new FakeAttribute('software_infos')),
        'template'  => get_template_path('inventory.tpl', TRUE, dirname(__FILE__)),
      ),
      'users' => array(
        'name'      => _('Users'),
        'class'     => array('fullwidth'),
        'attrs'     => array(new FakeAttribute('users_infos')),
        'template'  => get_template_path('inventory.tpl', TRUE, dirname(__FILE__)),
      ),
      'process' => array(
        'name'      => _('Processes'),
        'class'     => array('fullwidth'),
        'attrs'     => array(new FakeAttribute('process_infos')),
        'template'  => get_template_path('inventory.tpl', TRUE, dirname(__FILE__)),
      ),
      'other' => array(
        'name'      => _('Other'),
        'class'     => array('fullwidth'),
        'attrs'     => array(new FakeAttribute('other_infos')),
        'template'  => get_template_path('inventory.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    $attributesInfo = static::getAttributesInfo();
    if (preg_match('/'.preg_quote(get_ou('inventoryRDN').$config->current['BASE'], '/').'$/', $dn)) {
      $this->mainTab        = TRUE;
      $this->objectclasses  = array('fdInventoryContent');
      $attributesInfo['match']['attrs'][0]->setAllowHTML(TRUE);
    } else {
      // Remove cn
      unset($attributesInfo['hardware']['attrs'][3]);
      unset($attributesInfo['match']);
    }

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    $this->hardware_infos = NULL;
    $this->software_infos = NULL;
    $this->process_infos  = NULL;
    $this->users_infos    = NULL;
  }

  function execute (): string
  {
    if ($this->hardware_infos === NULL) {
      $infos = $this->get_inventory_infos();
      if (!empty($infos)) {
        $this->hardware_infos = $infos['hardware'];
        $this->software_infos = $infos['software'];
        $this->process_infos  = $infos['process'];
        $this->users_infos    = $infos['users'];
        $this->other_infos    = $infos['other'];
      }
    }
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $smarty->append('css_files', 'plugins/fusioninventory/inventory.css');
    return parent::execute();
  }

  function save (): array
  {
    return array();
  }

  function remove (bool $fulldelete = FALSE): array
  {
    return array();
  }

  function get_inventory_infos ()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    if ($this->mainTab) {
      $dn           = $this->dn;
      $macAddress   = $this->attrs['macAddress'];
      $ipHostNumber = $this->attrs['ipHostNumber'];
      unset($macAddress['count']);
      unset($ipHostNumber['count']);
    } else {
      if (!isset($this->parent)) {
        return array();
      }
      $macAddress   = $this->parent->getBaseObject()->macAddress;
      if (!is_array($macAddress) && ($macAddress != '')) {
        $macAddress = array($macAddress);
      }
      $ipHostNumber = $this->parent->getBaseObject()->ipHostNumber;
    }
    $matching     = $config->get_cfg_value('inventoryMatching', 'mac');
    if (!empty($macAddress)) {
      $macfilter = '(|(macAddress='.implode(')(macAddress=', $macAddress).'))';
    } else {
      $macfilter = NULL;
      if (($matching == 'mac') || ($matching == 'both')) {
        return array();
      }
    }
    if (!empty($ipHostNumber)) {
      $ipfilter = '(|(ipHostNumber='.implode(')(ipHostNumber=', $ipHostNumber).'))';
    } else {
      $ipfilter = NULL;
      if (($matching == 'ip') || ($matching == 'both') || ($macfilter === NULL)) {
        return array();
      }
    }
    switch ($matching) {
      case 'ip':
        $this->filter = $ipfilter;
        break;
      case 'mac':
        $this->filter = $macfilter;
        break;
      case 'both':
        $this->filter = "(&$macfilter$ipfilter)";
        break;
      case 'either':
        $this->filter = "(|$macfilter$ipfilter)";
        break;
    }
    if ($this->mainTab) {
      $this->updateMatching();
    } else {
      $ldap->cd(get_ou('inventoryRDN').$config->current['BASE']);
      $ldap->search('(&(objectClass=fdInventoryContent)'.$this->filter.')');
      if ($attrs = $ldap->fetch()) {
        $dn = $attrs['dn'];
      } else {
        return array();
      }
    }
    $ldap->cd($dn);
    $ldap->search('(objectClass=*)', array('*'), 'one');
    $infos = array();
    foreach (array_unique(static::$sections) as $section) {
      $infos[$section] = array();
    }
    while ($attrs = $ldap->fetch()) {
      $object = array();
      $cn     = FALSE;
      $class  = strtolower(preg_replace('/^fdInventory/', '', $attrs['objectClass'][0]));
      for ($i = 0; $i < $attrs['count']; $i++) {
        $key  = $attrs[$i];
        if ($key == 'objectClass') {
          continue;
        } elseif ($key == 'cn') {
          $cn = $attrs[$key][0];
          continue;
        }
        $attr = $attrs[$key];
        $key  = preg_replace('/^fdInventory/', '', $key);
        if (!is_array($attr)) {
          $object[$key] = $attr;
        } elseif ($attr['count'] == 1) {
          $object[$key] = $attr[0];
        } else {
          unset($attr['count']);
          $object[$key] = '['.join(', ', $attr).']';
        }
      }
      if ($cn === FALSE) {
        trigger_error('inventory object with no cn');
        continue;
      }
      if (empty($object)) {
        continue;
      }

      if (!isset(static::$sections[$class])) {
        static::$sections[$class] = 'other';
      }
      if (!isset($infos[static::$sections[$class]][$class])) {
        $infos[static::$sections[$class]][$class] = array('keys' => array(), 'objects' => array());
      }
      $infos[static::$sections[$class]][$class]['keys'] =
        array_unique(array_merge(
          $infos[static::$sections[$class]][$class]['keys'],
          array_keys($object)
        ));
      $infos[static::$sections[$class]][$class]['objects'][$cn] = $object;
    }

    return $infos;
  }

  function updateMatching ()
  {
    global $config;
    $systemTypes = array('server','workstation','terminal','printer','component','phone','mobilePhone');
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search($this->filter);
    $foundType  = FALSE;
    $fallbackDn = NULL;
    while (!$foundType && ($attrs = $ldap->fetch())) {
      if ($attrs['dn'] == $this->dn) {
        continue;
      }
      foreach ($systemTypes as $type) {
        if (objects::isOfType($attrs, $type)) {
          $foundType = $type;
          break;
        }
      }
      if ($foundType) {
        $this->matching = objects::link($attrs['dn'], $foundType);
      } else {
        $fallbackDn = $attrs['dn'];
      }
    }
    if ($foundType) {
      return;
    }
    if ($fallbackDn !== NULL) {
      $this->matching = $attrs['dn'];
    } else {
      $templates = array();
      foreach ($systemTypes as $type) {
        $tmp = objects::getTemplates($type);
        foreach ($tmp as $tdn => $tlabel) {
          $templates[$type.':'.$tdn] = $tlabel;
        }
      }
      $this->attributesAccess['matching'] = new CompositeAttribute(
        _('Create a system based on this inventory'), 'matching',
        array(
          new SelectAttribute(
            '', 'template',
            'matching_template', TRUE,
            array_keys($templates), '',
            array_values($templates)
          ),
          new SelectAttribute(
            '', 'mac address',
            'matching_macaddress', TRUE,
            $this->macAddress
          ),
          new ButtonAttribute(
            '', 'button',
            'matching_button', _('Create'),
            'createSystem'
          )
        ),
        FALSE, FALSE,
        '', _('No matching system')
      );
      if (empty($templates)) {
        $this->attributesAccess['matching']->setDisabled(TRUE);
      }
      $this->attributesAccess['matching']->setParent($this);
      $this->attributesAccess['matching']->setLinearRendering(TRUE);
    }
  }

  function createSystem ()
  {
    list ($type, $dn) = explode(':', $_POST['matching_template'], 2);
    $infos = objects::infos($type);

    $values = array(
      $infos['mainTab'] => array(
        'cn'            => $this->hardware_infos['hardware']['objects']['hardware0']['NAME'],
        'ipHostNumber'  => $this->ipHostNumber,
        'macAddress'    => $_POST['matching_macaddress'],
      )
    );

    $template = new template($type, $dn);
    $template->reset();
    $template->deserialize($values);
    $tabObject = $template->apply();
    $msgs = $tabObject->save();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
    } else {
      $this->attributesAccess['matching'] = new DisplayAttribute(
        _('System'), _('System entry matching this inventory'),
        'matching'
      );
      $this->attributesAccess['matching']->setParent($this);
      $this->attributesAccess['matching']->setAllowHTML(TRUE);
      $this->updateMatching();
    }
  }
}
