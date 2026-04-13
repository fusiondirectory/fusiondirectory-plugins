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

class archiveTask extends simplePlugin
{
  protected $displayHeader = TRUE;
  private $subStates;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Archive'),
      'plDescription'  => _('Tasks Archive Object'),
      'plIcon'        => 'geticon.php?context=mimetypes&icon=application-x-archive&size=48',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdArchiveTasks'],
      'plFilter'       => '(objectClass=fdArchiveTasks)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [ // The name of the class is required, not the object class.
        'auditTask',
        'automaticGroupsTask',
        'dnsTask',
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
        'name'  => _('SupAnn Resource Archiving Settings'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdArchiveTaskResource', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdArchiveTaskState', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdArchiveTaskSubState', FALSE, [], "None"
          ),
        ]
      ],
      'UserGroupSelection' => [
        'name'  => _('Recipients Users and/or Groups'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Monitored Members'), _('Users or Groups requiring monitoring for archiving.'),
            'fdArchiveTaskMembers', TRUE
          ),
        ],
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    // SupAnn Status management
    if (class_available('supannAccountStatus')) {
      $this->setSupannStates();
    } else {
        msg_dialog::display(
            _('Warning'),
            _('The SupAnn Account Status plugin is not activated. This task cannot function properly without it.'),
            WARNING_DIALOG
        );
    }
  }

  /**
   * @return void
   * Note : Simply get the existing SupAnn states to the archive attributes.
   */
  protected function setSupannStates (): void
  {
    global $config;

    // Define the mandatory ones and get the remaining from configuration.
    $resources = supannAccountStatus::getResources();
    $states    = supannAccountStatus::getStates();

    $this->subStates = supannAccountStatus::getConfiguredSubstates(); // Keys are states and Values are subStates

    $this->attributesAccess['fdArchiveTaskResource']->setChoices(array_keys($resources), array_values($resources));
    $this->attributesAccess['fdArchiveTaskState']->setChoices(array_keys($states), array_values($states));

    // Allows the sub states to be listed when state is modified.
    $this->attributesAccess['fdArchiveTaskState']->setSubmitForm('updateFieldsChoices');
    $this->updateFieldsChoices();
  }

  /**
   * @return void
   * Note : Update list of subStates which depends on the state selected
   */
  function updateFieldsChoices ()
  {
    $subStatesList = $this->subStates[$this->attributesAccess['fdArchiveTaskState']->getValue()] ?? [];
    $this->attributesAccess['fdArchiveTaskSubState']->setChoices(array_keys($subStatesList), array_values($subStatesList));
  }

  /**
   * Generate slave tasks for archiving
   */
  function generateSlaveTasks ()
  {
    $monitoredMembers = $this->attributesAccess['fdArchiveTaskMembers']->getValue();
    $listOfDN = tasks::extractMembersFromGroups($monitoredMembers);

    // Call the method from parent tasks object (first tab) to create sub-tasks.
    $this->parent->getBaseObject()->createSlaveTasks($listOfDN, 'fdTasksGranularDN', NULL, 'archive');
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
      // Check if the SupAnn Account Status plugin is available (It should not happen as attributes verification will trigger errors before save)
    if (!class_available('supannAccountStatus')) {
        msg_dialog::display(
            _('Error'),
            _('The SupAnn Account Status plugin is not activated. This task cannot be saved or executed without it.'),
            ERROR_DIALOG
        );
        return []; // Return an empty array to indicate no action was taken
    }

      // Verify if this task has to be executed upon saving
      $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

    if ($execTasks) {
        $this->generateSlaveTasks();
    }

      return parent::save();
  }
}
