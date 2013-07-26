<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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

  static function getAttributesInfo ()
  {
    return array(
      'inventory' => array(
        'name'  => _('Inventory'),
        'attrs' => array(new FakeAttribute('inventory_infos')),
        'template' => get_template_path('inventory.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Inventory'),
      'plDescription' => _('Inventory Viewer'),
      'plPriority'    => 90,
      'plCategory'    => array('workstation', 'server'),
      'plObjectType'  => array('workstation', 'server'),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct(&$config, $dn, $object)
  {
    parent::__construct($config, $dn, $object);

    $this->inventory_infos = NULL;
  }

  function execute()
  {
    if ($this->inventory_infos === NULL) {
      $this->inventory_infos = $this->get_inventory_infos();
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
    if (!isset($this->parent)) {
      return array();
    }
    $macAddress = $this->parent->getBaseObject()->netConfigDNS->macAddress;
    $ldap = $this->config->get_ldap_link();
    $ldap->cd(get_ou('inventoryRDN').$this->config->current['BASE']);
    $ldap->search('(&(objectClass=fdInventoryContent)(macAddress='.$macAddress.'))');
    if ($attrs = $ldap->fetch()) {
      $dn = $attrs['dn'];
    } else {
      return array();
    }
    $ldap->ls('(objectClass=*)', $dn);
    $infos = array();
    while ($attrs = $ldap->fetch()) {
      $object = array();
      $cn     = FALSE;
      $class  = '';
      for ($i = 0; $i < $attrs['count']; $i++) {
        $key  = $attrs[$i];
        if ($key == 'objectClass') {
          $class = strtolower(preg_replace('/^fdInventory/', '', $attrs[$key][0]));
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
          $object[$key] = $attr;
          unset($object[$key]['count']);
        }
      }
      if ($cn === FALSE) {
        trigger_error('inventory object with no cn');
        continue;
      }
      $infos[$cn] = array('class' => $class, 'infos' => $object);
    }

    return $infos;
  }
}
?>
