<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

class workstationGeneric extends ipHostPlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Workstation'),
      'plDescription' => _('Workstation information'),
      'plObjectClass' => ['fdWorkstation', 'ieee802Device'],
      'plFilter'      => '(objectClass=fdWorkstation)',
      'plObjectType'  => ['workstation' =>
        [
          'name'        => _('Workstation'),
          'description' => _('Workstation'),
          'ou'          => get_ou('workstationRDN'),
          'icon'        => 'geticon.php?context=devices&icon=computer&size=16',
          'mainAttr'    => 'cn',
        ]
      ],
      'plSearchAttrs' => ['description', 'ipHostNumber', 'macAddress'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo ($word = NULL, $rdn = NULL): array
  {
    if ($word === NULL) {
      $word = _('workstation');
    }
    if ($rdn === NULL) {
      $rdn = get_ou('workstationRDN');
    }
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute($rdn),
          new HostNameAttribute(
            _('Name'),
            sprintf(_('The name of the %s'), $word),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'),
            sprintf(_('A short description of the %s'), $word),
            'description', FALSE
          ),
          new StringAttribute(
            _('Location'),
            sprintf(_('The location of the %s'), $word),
            'l', FALSE
          ),
          new BooleanAttribute(
            sprintf(_('Lock this %s'), $word),
            sprintf(_('This will prevent the %s from being reinstalled'), $word),
            'fdMode',
            FALSE,
            FALSE,
            '',
            'locked',
            'unlocked'
          ),
        ]
      ],
      'network' => [
        'name'      => _('Network settings'),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
        'attrs'     => [
          new SetAttribute(
            new IPAttribute(
              _('IP address'), _('IP addresses this system uses (v4 or v6)'),
              'ipHostNumber', TRUE
            )
          ),
          new SetAttribute(
            new MacAddressAttribute(
              _('Mac address'), _('Mac address of this system'),
              'macAddress', FALSE
            )
          ),
        ],
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique('one');
  }

  protected function post_remove ()
  {
    /* Clean queue from entries with this mac */
    if (class_available('supportDaemon')) {
      $q = new supportDaemon();
      $q->clean_queue_from_mac($this->attributesAccess['macAddress']->getInitialValue());
    }

    return parent::post_remove();
  }
}
