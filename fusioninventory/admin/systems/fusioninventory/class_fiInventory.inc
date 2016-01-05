<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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
  var $objectclasses  = array();
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
  );

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Inventory'),
      'plDescription' => _('Inventory Viewer'),
      'plPriority'    => 90,
      'plObjectType'  => array(
        'workstation', 'server',
        'inventory' => array(
          'description' => _('Inventory object'),
          'filter'      => 'objectClass=fdInventoryContent',
          'mainAttr'    => 'cn',
          'icon'        => 'plugins/fusioninventory/images/iconMini.png',
          'name'        => _('Inventory object'),
          'ou'          => get_ou('inventoryRDN'),
          'tabClass'    => 'simpleTabs_noSpecial',
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'match' => array(
        'name'      => _('Matching system'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new DisplayAttribute(_('System'), _('System entry matching this inventory'), 'matching'),
          new HiddenArrayAttribute('macAddress'),
          new HiddenArrayAttribute('ipHostNumber'),
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
    );
  }

  function __construct(&$config, $dn, $object)
  {
    $attributesInfo = self::getAttributesInfo();
    if (preg_match('/'.preg_quote(get_ou('inventoryRDN').$config->current['BASE'], '/').'$/', $dn)) {
      $this->mainTab        = TRUE;
      $this->objectclasses  = array('fdInventoryContent');
      $attributesInfo['hardware']['attrs'][] = new HiddenAttribute('cn');
      $attributesInfo['match']['attrs'][0]->setAllowHTML(TRUE);
    } else {
      unset($attributesInfo['match']);
    }

    parent::__construct($config, $dn, $object, $attributesInfo);

    $this->hardware_infos = NULL;
    $this->software_infos = NULL;
    $this->process_infos  = NULL;
    $this->users_infos    = NULL;
  }

  function compute_dn()
  {
    return 'cn='.$this->cn.','.get_ou('inventoryRDN').$this->config->current['BASE'];
  }

  function execute()
  {
    if ($this->hardware_infos === NULL) {
      $infos = $this->get_inventory_infos();
      if (!empty($infos)) {
        $this->hardware_infos = $infos['hardware'];
        $this->software_infos = $infos['software'];
        $this->process_infos  = $infos['process'];
        $this->users_infos    = $infos['users'];
      }
    }
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $smarty->append('css_files', 'plugins/fusioninventory/inventory.css');
    return parent::execute();
  }

  function save()
  {
  }

  function get_inventory_infos()
  {
    $ldap = $this->config->get_ldap_link();

    if ($this->mainTab) {
      $dn = $this->dn;
      $macAddress   = $this->attrs['macAddress'];
      $ipHostNumber = $this->attrs['ipHostNumber'];
      unset($macAddress['count']);
      unset($ipHostNumber['count']);
    } else {
      if (!isset($this->parent)) {
        return array();
      }
      $macAddress   = $this->parent->getBaseObject()->macAddress;
      if ($macAddress != '') {
        $macAddress = array($macAddress);
      }
      $ipHostNumber = $this->parent->getBaseObject()->ipHostNumber;
    }
    $matching     = $this->config->get_cfg_value('inventoryMatching', 'mac');
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
      $ldap->cd(get_ou('inventoryRDN').$this->config->current['BASE']);
      $ldap->search('(&(objectClass=fdInventoryContent)'.$this->filter.')');
      if ($attrs = $ldap->fetch()) {
        $dn = $attrs['dn'];
      } else {
        return array();
      }
    }
    $ldap->ls('(objectClass=*)', $dn);
    $infos = array();
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

      if (!isset($infos[self::$sections[$class]][$class])) {
        $infos[self::$sections[$class]][$class] = array('keys' => array(), 'objects' => array());
      }
      $infos[self::$sections[$class]][$class]['keys'] =
        array_unique(array_merge(
          $infos[self::$sections[$class]][$class]['keys'],
          array_keys($object)
        ));
      $infos[self::$sections[$class]][$class]['objects'][$cn] = $object;
    }

    return $infos;
  }

  function updateMatching()
  {
    $systemTypes = array('server','workstation','terminal','printer','component','winstation','phone','mobilePhone');
    $ldap = $this->config->get_ldap_link();
    $ldap->cd(get_ou('systemRDN').$this->config->current['BASE']);
    $ldap->search($this->filter);
    if ($ldap->count()) {
      $attrs = $ldap->fetch();
      $foundType = FALSE;
      foreach ($systemTypes as $type) {
        if (objects::isOfType($attrs, $type)) {
          $foundType = $type;
          break;
        }
      }
      if ($foundType) {
        $this->matching = objects::link($attrs['dn'], $foundType);
      } else {
        $this->matching = $attrs['dn'];
      }
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
      $this->attributesAccess['matching']->setParent($this);
      $this->attributesAccess['matching']->setLinearRendering(TRUE);
    }
  }

  function createSystem()
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
    $msgs = $tabObject->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
    } else {
      $tabObject->save();
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
?>
