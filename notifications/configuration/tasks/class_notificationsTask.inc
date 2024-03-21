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

class tasksNotifications extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Notifications'),
      'plDescription'  => _('Tasks Notifications Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=tasks&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksNotifications'],
      'plFilter'       => '(objectClass=fdTasksNotifications)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => ['tasksMail', 'tasksLifeCycle'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1'           => [
        'name'  => _('Modified attributes list to be notified'),
        'attrs' => [
          // Following attribute will contain all the DNs where a new snapshot was taken, required by orchestrator.
          new HiddenArrayAttribute('fdTasksNotificationsListOfNewSnapUsersDN', FALSE, ''),
          // Following attribute contain the list of members emails that should receive a notification.
          new HiddenArrayAttribute('fdTasksNotificationsListOfRecipientsDN', FALSE, ''),
          new SetAttribute(
            new SelectAttribute(
              'Attributes', _('Any changes listed to set attributes will trigger a notification'),
              'fdTasksNotificationsAttributes', FALSE,
            ),
          ),
        ]
      ],
      'UserGroupSelection' => [
        'name'  => _('Recipients Users and/or Groups'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Recipients'), _('Users or Groups requiring to be notified.'),
            'fdTasksNotificationsRecipientsMembers', TRUE
          ),
        ],
      ],
    ];
  }

  /**
   * @throws NonExistingLdapNodeException
   */
  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

  }

  /**
   * Retrieve all the DNs from groups or members
   */
  public function getListOfDN (): void
  {
    // Get the members or groups selected
    $membersAndGroups = $this->attributesAccess['fdTasksNotificationsRecipientsMembers']->getValue();
    // Call static function from tasks object in order to get a real list of members
    $this->attributesAccess['fdTasksNotificationsListOfRecipientsDN']->setValue(array_values(tasks::extractMembersFromGroups($membersAndGroups)));
  }

  /**
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  public function generateSlaveTasks ()
  {
    $listOfDN = $this->attributesAccess['fdTasksLifeCycleListOfDN']->getValue();
    // The attribute required to be search in createSlaveTasks
    $attributeType = 'fdTasksGranularDN';

    // Call the method from parent tasks object (first tab) to create sub-tasks.
    $this->parent->getBaseObject()->createSlaveTasks($listOfDN, $attributeType);
  }

  public function update (): bool
  {
    parent::update();

    // Take the list of DN based on the groups or members list selected and filled the related attributes
    $this->getListOfDN();

    return TRUE;
  }

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
