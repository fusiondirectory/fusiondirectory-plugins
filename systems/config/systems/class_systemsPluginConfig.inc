<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016 FusionDirectory

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

class systemsPluginConfig extends simplePlugin
{
  static protected $ipClasses = [];

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Systems'),
      'plDescription'   => _('Systems plugin configuration'),
      'plObjectClass'   => ['fdSystemsPluginConf'],
      'plPriority'      => 20,
      'plObjectType'    => ['configuration'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'rdns' => [
        'name'  => _('LDAP tree for systems'),
        'attrs' => [
          new StringAttribute(
            _('Systems RDN'), _('Branch in which systems will be stored'),
            'fdSystemRDN', TRUE,
            'ou=systems'
          ),
          new StringAttribute(
            _('Server RDN'), _('Branch in which servers will be stored'),
            'fdServerRDN', TRUE,
            'ou=servers,ou=systems'
          ),
          new StringAttribute(
            _('Workstations RDN'), _('Branch in which workstations will be stored'),
            'fdWorkstationRDN', TRUE,
            'ou=workstations,ou=systems'
          ),
          new StringAttribute(
            _('Terminal RDN'), _('Branch in which terminals will be stored'),
            'fdTerminalRDN', TRUE,
            'ou=terminals,ou=systems'
          ),
          new StringAttribute(
            _('Printer RDN'), _('Branch in which printers will be stored'),
            'fdPrinterRDN', TRUE,
            'ou=printers,ou=systems'
          ),
          new StringAttribute(
            _('Component RDN'), _('Branch in which network devices will be stored'),
            'fdComponentRDN', TRUE,
            'ou=netdevices,ou=systems'
          ),
          new StringAttribute(
            _('Phone RDN'), _('Branch in which phones will be stored'),
            'fdPhoneRDN', TRUE,
            'ou=phones,ou=systems'
          ),
          new StringAttribute(
            _('Mobile phone RDN'), _('Branch in which mobile phones will be stored'),
            'fdMobilePhoneRDN', TRUE,
            'ou=mobile,ou=systems'
          ),
        ]
      ],
      'config' => [
        'name'  => _('Miscellaneous'),
        'attrs' => [
          new OrderedArrayAttribute(
            new CompositeAttribute(
              _('Available encodings for share services'),
              'fdEncodings',
              [
                new StringAttribute(
                  _('Encoding'), _('The encoding code name'),
                  'encoding_code', TRUE, '', '',
                  '/^[^=]+$/'
                ),
                new StringAttribute(
                  _('Label'), _('The encoding displayed name'),
                  'encoding_label', TRUE, '', '',
                  '/^[^=]+$/'
                ),
              ],
              '/^([^=]*)=([^=]*)$/',
              '%s=%s',
              '',
              _('Encodings')
            ),
            FALSE, /* no order */
            [
              'UTF-8=UTF-8', 'ISO8859-1=ISO8859-1 (Latin 1)',
              'ISO8859-2=ISO8859-2 (Latin 2)', 'ISO8859-3=ISO8859-3 (Latin 3)',
              'ISO8859-4=ISO8859-4 (Latin 4)', 'ISO8859-5=ISO8859-5 (Latin 5)',
              'cp850=CP850 (Europe)'
            ],
            TRUE /* edit enabled */
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Mandatory IP'), _('Object types tabs for which IP field should be mandatory'),
              'fdMandatoryIpClasses', FALSE
            ),
            ['componentGeneric','printGeneric','workstationGeneric','terminalGeneric','serverGeneric']
          ),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    if (empty(static::$ipClasses)) {
      $plist = session::global_get('plist');
      static::$ipClasses = [];
      foreach ($plist->info as $cname => $info) {
        if ($cname == 'all') {
          continue;
        }
        if (is_subclass_of($cname, 'ipHostPlugin')) {
          static::$ipClasses[$cname] = $info['plTitle'];
        }
      }
    }
    $this->attributesAccess['fdMandatoryIpClasses']->attribute->setChoices(array_keys(static::$ipClasses), array_values(static::$ipClasses));
  }
}
