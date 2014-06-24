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

class inventoryManagement extends simpleManagement
{
  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;
  protected $skipCpHandler          = TRUE;
  protected $skipSnapHandler        = TRUE;
  protected $autoActions            = FALSE;
  protected $autoFilterAttributes   = array('dn', 'cn', 'macAddress');

  protected $objectTypes = array('inventory');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Inventory objects'),
      'plDescription' => _('Browse inventory objects'),
      'plIcon'        => 'plugins/fusioninventory/images/icon.png',
      'plSection'     => 'addons',
      'plManages'     => array('inventory'),

      'plProvidedAcls' => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->listXMLPath  = get_template_path('inventory-list.xml', TRUE, dirname(__FILE__));
    parent::__construct($config, $ui);
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();
    $this->headpage->registerElementFilter('filterOS', 'inventoryManagement::filterOS');
  }

  static function filterOS()
  {
    $pid    = func_get_arg(0);
    $row    = func_get_arg(1);
    $dn     = func_get_arg(2);
    $trans  = array();

    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->ls('(objectClass=fdInventoryOPERATINGSYSTEM)', $dn, array('fdInventoryNAME','fdInventoryFQDN'));
    while ($attrs = $ldap->fetch()) {
      $link = $attrs['fdInventoryNAME'][0];
      if (isset($attrs['fdInventoryFQDN'][0])) {
        $link .= ' ('.$attrs['fdInventoryFQDN'][0].')';
      }
      $trans[] = $link;
    }

    $trans  = join("<br/>\n", $trans);

    if ($trans != "") {
      return '<a href="?plug='.$_GET['plug'].'&amp;PID='.$pid.'&amp;act=listing_edit_'.$row.'" title="'.$dn.'">'.$trans.'</a>';
    }

    return '';
  }
}
?>
