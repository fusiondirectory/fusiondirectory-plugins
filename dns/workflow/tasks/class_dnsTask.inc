<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2025 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class dnsTask extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks DNS'),
      'plDescription'  => _('Tasks DNS Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=tasks&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdDnsTasks'],
      'plFilter'       => '(objectClass=fdDnsTasks)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [ // The name of the class is required, not the object class.
        'auditTask',
        'archiveTask',
        'automaticGroupsTask',
        'extractorTask',
        'lifeCycleTask',
        'mailTask',
        'notificationsTask',
        'reminderTask',
      ],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1'           => [
        'name'  => _('Machines'),
        'attrs' => [
          new ObjectsAttribute(
            _('DNS zones'), _('DNS zones'),
            'fdDnsZones', TRUE, ['dnsZone'], [],
            'dn', 'zoneName'
          ),
        ]
      ],
    ];
  }
}

