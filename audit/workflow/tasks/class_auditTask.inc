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
  // To understand the last Exec mechanism, reference yourself to Notifications tasks.
  protected $lastExec = NULL;

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
      // plConflicts take the name of the object class without the 'fd' in front.
      'plConflicts'    => ['tasksMail', 'tasksLifeCycle', 'tasksNotifications'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1'                => [
        'name'  => _('Schedule Audit Retention'),
        'attrs' => [
          new IntAttribute(
            _('Retaining days'), _('Numbers of days audit logs should be kept before deletion'),
            'fdAuditTasksRetention', TRUE, '0', FALSE, '0'
          ),
        ]
      ],
    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    // Set the list of available attributes to follow in the set selection
  }

  /**
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  function generateSlaveTasks ()
  {
    // The attribute required to be search in createSlaveTasks
    $attributeType = 'fdTasksGranularDN';

    // Call the method from parent tasks object (first tab) to create sub-tasks.
    $this->parent->getBaseObject()->createSlaveTasks(['auditRetentionTask'], $attributeType, NULL, 'Audit');
  }

  /**
   * @return bool
   */
  function update (): bool
  {
    parent::update();

    return TRUE;
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
