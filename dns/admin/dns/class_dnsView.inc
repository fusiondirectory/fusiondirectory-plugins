<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

class dnsView extends simplePlugin
{
  public static function plInfo (): array
  {
    return array(
      'plShortName'   => _('DNS view'),
      'plDescription' => _('DNS view'),
      'plObjectClass' => array('fdDNSView'),
      'plObjectType'  => array('dnsView' => array(
        'name'        => _('DNS view'),
        'ou'          => get_ou('dnsRDN'),
        'icon'        => 'geticon.php?context=applications&icon=dns&size=16'
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'  => _('View'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('dnsRDN')),
          new HostNameAttribute(
            _('View name'), _('Name of this view'),
            'cn', TRUE
          ),
          new SelectAttribute(
            _('Match clients ACL'), _('Name of the ACL to use for the source IP address of the incoming requests'),
            'fdDNSViewMatchClientsAcl', FALSE
          ),
          new SelectAttribute(
            _('Match destinations ACL'), _('Name of the ACL to use for the destination IP address of the incoming requests'),
            'fdDNSViewMatchDestinationsAcl', FALSE
          ),
          new BooleanAttribute(
            _('Match recursive only'), _('Match only recursive queries in this view'),
            'fdDNSViewMatchRecursiveOnly', FALSE
          ),
          new SetAttribute(
            new SelectAttribute(
              _('DNS zones'), _('DNS zones in this view'),
              'fdDNSZoneDn', FALSE
            )
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $zones = objects::ls('dnsZone');
    $this->attributesAccess['fdDNSZoneDn']->attribute->setChoices(array_keys($zones), array_values($zones));
    $acls = objects::ls('dnsAcl', 'cn');
    $this->attributesAccess['fdDNSViewMatchClientsAcl']->setChoices($acls);
    $this->attributesAccess['fdDNSViewMatchDestinationsAcl']->setChoices($acls);
  }
}
