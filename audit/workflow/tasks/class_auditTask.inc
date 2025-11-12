<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2024 FusionDirectory

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

class auditTask extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Audit'),
      'plDescription'  => _('Tasks Audit Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=audit&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdAuditTasks'],
      'plFilter'       => '(objectClass=fdAuditTasks)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [ // The name of the class is required, not the object class.
        'tasksMail',          // from mail plugin (core)
        'taskReminder',       // from user-reminder plugin
        'tasksNotifications', // from notifications plugin
        'archiveTask',        // from archive plugin
        'extractorTask',      // from extractor plugin (if present)
        'tasksLifeCycle',     // from lifecycle plugin (if present)
        //'auditTask',          // from archive plugin (if present)
        'tasksAutomaticGroups', // from automatic-groups plugin
      ],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1' => [
        'name'  => _('Schedule Audit Retention'),
        'attrs' => [
          new IntAttribute(
            _('Retaining days'), _('Numbers of days audit logs should be kept before deletion'),
            'fdAuditTasksRetention', TRUE, '0', FALSE, '0'
          ),
        ]
      ],
      'section2' => [
        'name'  => _('Syslog Transformation'),
        'attrs' => [
          new BooleanAttribute(
            _('Enable Syslog Transformation'), _('Enable transformation of audit logs into syslog format'),
            'fdAuditSyslogEnabled', FALSE, FALSE
          ),
          new StringAttribute(
            _('Syslog Name Prefix'), _('Prefix to use for syslog files name'),
            'fdAuditSyslogPrefix', FALSE, 'fd_syslog'
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    // Dynamically disable "Retaining days" if Syslog Transformation is enabled
    $this->attributesAccess['fdAuditTasksRetention']->setDisabled(
      $this->attributesAccess['fdAuditSyslogEnabled']->getValue()
    );

    // Dynamically disable "Syslog Name Prefix" if Syslog Transformation is not enabled
    $this->attributesAccess['fdAuditSyslogPrefix']->setDisabled(
      !$this->attributesAccess['fdAuditSyslogEnabled']->getValue()
    );

    // Add a listener to update the state dynamically when Syslog Transformation is toggled
    $this->attributesAccess['fdAuditSyslogEnabled']->setSubmitForm('updateFieldsState');
  }

  function updateFieldsState ()
  {
    // Disable "Retaining days" if Syslog Transformation is enabled
    $this->attributesAccess['fdAuditTasksRetention']->setDisabled(
      $this->attributesAccess['fdAuditSyslogEnabled']->getValue()
    );

    // Disable "Syslog Name Prefix" if Syslog Transformation is not enabled
    $this->attributesAccess['fdAuditSyslogPrefix']->setDisabled(
      !$this->attributesAccess['fdAuditSyslogEnabled']->getValue()
    );
  }

  /**
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  function generateSlaveTasks ()
  {
    // Check if syslog transformation is enabled
    $syslogEnabled = $this->attributesAccess['fdAuditSyslogEnabled']->getValue();

    // The attribute required to be searched in createSlaveTasks
    $attributeType = 'fdTasksGranularDN';

    if ($syslogEnabled) {
      // Create sub-tasks for audit syslog transformation
      $this->parent->getBaseObject()->createSlaveTasks(['auditSyslogTask'], $attributeType, NULL, 'Audit-Syslog');
    } else {
      // Create sub-tasks for audit deletion
      $this->parent->getBaseObject()->createSlaveTasks(['auditRetentionTask'], $attributeType, NULL, 'Audit');
    }
  }

  /**
   * @return array
   */
  function save (): array
  {
    // Verify if this tasks has to be executed upon saving.
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

    if ($execTasks) {
      $this->generateSlaveTasks();
    }

    return parent::save();
  }
}
