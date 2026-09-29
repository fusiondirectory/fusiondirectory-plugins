<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2025-2026 FusionDirectory

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

class exportTask extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Export'),
      'plDescription'  => _('Tasks objects Exportation'),
      'plIcon'         => 'geticon.php?context=applications&icon=export-task&size=48',
      'plSmallIcon'    => 'geticon.php?context=applications&icon=export-task&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdExportTasks'],
      'plFilter'       => '(objectClass=fdExportTasks)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [
        'auditTask',
        'archiveTask',
        'automaticGroupsTask',
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
      'exportmembers' => [
        'name'  => _('Members to Export'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Selected Members'),
            _('Users or Groups to export data from.'),
            'fdExportTaskMembers',
            TRUE
          ),
        ],
      ],
      'attributes' => [
        'name'  => _('Attributes to Export'),
        'attrs' => [
          new SetAttribute(
            new SelectAttribute(
              _('Attributes'), _('Attributes to export'),
              'fdExportTaskAttributes', TRUE
            ),
          ),
        ]
      ],
      'exportformat' => [
        'name'  => _('Export format'),
        'attrs' => [
          new SelectAttribute(
            _('Format'),
            _('Output format for exported data'),
            'fdExportTaskFormat',
            TRUE,
            ['CSV'],
            'CSV',
            [_('CSV (Comma Separated Values)')],
            ''
          ),
        ]
      ],
      'email' => [
        'name'  => _('Email Notifications'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Recipients'),
            _('Users or Groups to receive the export by email.'),
            'fdExportRecipientsMembers',
            TRUE
          ),
          new MailAttribute(
            _('Sender email address'),
            _('Email address from which the export will be sent'),
            'fdExportEmailSender',
            TRUE,
          ),
        ],
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    // Load available attributes from backend config
    $this->attributesAccess['fdExportTaskAttributes']->attribute->setChoices($this->getAttributesListFromConf());
  }

  /**
   * Load export attributes from backend config, default to ALL if none set.
   */
  protected function getAttributesListFromConf (): array
  {
    global $config;
    $result = [];

    if (!empty($config->current['EXTRACTORCONFATTRIBUTES'])) {
      if (is_string($config->current['EXTRACTORCONFATTRIBUTES'])) {
        $result[] = $config->current['EXTRACTORCONFATTRIBUTES'];
      } else {
        $result = $config->current['EXTRACTORCONFATTRIBUTES'];
      }
    }

    // Ensure 'ALL' is in the array
    if (!in_array('ALL', $result, TRUE)) {
      array_unshift($result, 'ALL');
    }

    return $result;
  }

  /**
   * Generate  subtasks for export
   */
  function generateSubTasks ()
  {
    $selectedMembers = $this->attributesAccess['fdExportTaskMembers']->getValue();

    // Call the method from parent tasks object (first tab) to create ONE sub-task for the orchestrator
    $this->parent->getBaseObject()->createSubTasks($selectedMembers, 'fdTasksGranularDN', NULL, 'export');
  }

  function check (): array
  {
    $errors = parent::check();
    return $errors;
  }

  /**
   * @return array
   */
  function save (): array
  {
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;
    if ($execTasks) {
        $this->generateSubTasks();
    }
    return parent::save();
  }
}
