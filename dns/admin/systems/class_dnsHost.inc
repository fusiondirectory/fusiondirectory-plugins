<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015  FusionDirectory

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

class dnsHost extends simplePlugin
{
  var $objectclasses = array('fdDNSHost');
  var $displayHeader = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('DNS'),
      'plDescription'   => _('Edit the DNS zones of a system'),
      'plIcon'          => 'geticon.php?context=applications&icon=dns&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=dns&size=16',
      'plObjectType'    => array('server', 'workstation'),
      'plPriority'      => 5,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'zones' => array(
        'name'  => _('DNS zones'),
        'attrs' => array(
          new SetAttribute(
            new SelectAttribute(
              '', _('DNS zones for this host'),
              'fdDNSZoneDn', TRUE
            )
          )
        )
      ),
      'records' => array(
        'name'      => _('DNS Records'),
        'template'  => get_template_path('dnsrecords.tpl', TRUE, dirname(__FILE__)),
        'attrs'     => array(
          new FakeAttribute('dnsRecords')
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $zones = objects::ls('dnsZone');
    $this->attributesAccess['fdDNSZoneDn']->attribute->setChoices(array_keys($zones), array_values($zones));
  }

  function execute()
  {
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $dnsZones   = $this->fdDNSZoneDn;
    $dnsRecords = array();
    $ips        = $this->parent->getBaseObject()->ipHostNumber;
    $cn         = $this->parent->getBaseObject()->cn;
    foreach($dnsZones as $dn) {
      $tabObject = objects::open($dn, 'dnsZone');
      $dnsRecords[] = array(
        'records' => array_filter(
          $tabObject->getBaseObject()->dnsRecords,
          function ($record) use ($ips, $cn) {
            return (in_array($record[2], $ips) || ($record[0] == $cn));
          }
        ),
        'name'    => $tabObject->getBaseObject()->zoneName
      );
    }
    $this->dnsRecords = $dnsRecords;
    return parent::execute();
  }
}
