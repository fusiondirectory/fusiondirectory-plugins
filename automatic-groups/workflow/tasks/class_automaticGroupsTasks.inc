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

class automaticGroupsTasks extends simplePlugin
{
  protected $displayHeader = TRUE;
  /**
   * @var array|array[]
   */
  private $subStates;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Automatic Groups'),
      'plDescription'  => _('Tasks Automatic Groups Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=user-group&size=48',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksAutomaticGroups'],
      'plFilter'       => '(objectClass=fdTasksAutomaticGroups)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [
        'tasksMail',
        'taskReminder',         // from user-reminder plugin
        'notificationsTask',    // from notifications plugin
        'archiveTask',          // from archive plugin
        'extractorTask',        // from extractor plugin (if present)
        'lifecycleTask'         // from lifecycle plugin (if present)
      ],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1'           => [
        'class' => ['fullwidth', 'information'],
        'name'  => _('We recommend the task redundancy to be set to daily, and the case New Members Only to be unpicked'),
        'attrs' => [
          // This hiddenAttribute is only present to show the information label on the section.
          new HiddenAttribute(
            'fdShowInformation', FALSE,
          ),
          // Following attributes will contain all the DNs required to be verified by Orchestrator.
          new HiddenArrayAttribute('fdTasksAutomaticGroupsListOfDN', FALSE, ''),
        ]
      ],
      'section2'           => [
        'name'  => _('Automatic Group criteria'),
        'class' => ['fullwidth'],
        'attrs' => [
          new ObjectSelectAttribute(
            _('Target Group'), _('Group where users will be added/removed'),
            'fdTasksAutomaticGroupsOfName', TRUE,
            ['ogroup'], 'dn', 'cn',
            '(objectClass=groupOfNames)'
          ),
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksAutomaticGroupsResource', TRUE
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksAutomaticGroupsState', TRUE
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksAutomaticGroupsSubState', FALSE
          ),
        ]
      ],
      'section3' => [
        'name'  => _('Source Users and/or Groups'),
        'class' => ['fullwidth'],
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Members'), _('Users or groups to check for automatic group assignment.'),
            'fdTasksAutomaticGroupsMembers', TRUE
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
    $this->attributesAccess['fdShowInformation']->setInLdap(FALSE);

    // SupAnn Status management
    if (class_available('supannAccountStatus')) {
      $this->setSupannStates();
    } else {
      $warning = new FusionDirectoryWarning(htmlescape(_('The plugin Supann does not seem to be installed!')));
      $warning->display();
    }
  }

  /**
   *  Set attributes values with defined supann states from configuration
   */
  protected function setSupannStates (): void
  {
    global $config;

    // Define the mandatory ones and get the remaining from configuration.
    $resources = ['COMPTE' => _('Account'), 'MAIL' => _('Mail')];

    foreach ($config->get_cfg_value('SupannRessourceLabels', []) as $line) {
      list($resource, $label) = explode(':', $line, 2);
      $resources[$resource] = $label;
    }

    $this->attributesAccess['fdTasksAutomaticGroupsResource']->setChoices(array_keys($resources), array_values($resources));
    $this->subStates = supannAccountStatus::getConfiguredSubstates(); //Keys are states and Values are subStates
    $this->attributesAccess['fdTasksAutomaticGroupsState']->setChoices(array_keys($this->subStates));

    // Allows the sub states to be listed when state is modified.
    $this->attributesAccess['fdTasksAutomaticGroupsState']->setSubmitForm('updateFieldsChoices');
    // This initial fill the list choices based on registered states.
    $this->updateFieldsChoices();
  }

  /* Update list of subStates which depends on the state selected */
  function updateFieldsChoices ()
  {
    $subStatesList = $this->subStates[$this->attributesAccess['fdTasksAutomaticGroupsState']->getValue()] ?? [];
    $this->attributesAccess['fdTasksAutomaticGroupsSubState']->setChoices(array_keys($subStatesList), array_values($subStatesList));
  }

  /**
   * Retrieve all the DNs from groups or members
   */
  public function getListOfDN (): void
  {
    // Get the members or groups selected
    $membersAndGroups = $this->attributesAccess['fdTasksAutomaticGroupsMembers']->getValue();
    // Call static function from tasks object, always return an array.
    $this->attributesAccess['fdTasksAutomaticGroupsListOfDN']->setValue(array_values(tasks::extractMembersFromGroups($membersAndGroups)));
  }

  /**
   * Generate slave tasks for automatic group assignment
   */
  public function generateSlaveTasks ()
  {
    $listOfDN = $this->attributesAccess['fdTasksAutomaticGroupsListOfDN']->getValue();
    // The attribute required to be search in createSlaveTasks
    $attributeType = 'fdTasksGranularDN';

    // Call the method from parent tasks object (first tab) to create sub-tasks.
    $this->parent->getBaseObject()->createSlaveTasks($listOfDN, $attributeType, NULL, 'Automatic Groups');
  }

  public function update (): bool
  {
    parent::update();

    // Take the list of DN based on the groups or members list selected and fill the related attributes
    $this->getListOfDN();

    return TRUE;
  }

  function save (): array
  {
    // Verify if this task has to be executed upon saving.
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

    if ($execTasks) {
      $this->generateSlaveTasks();
    }

    return parent::save();
  }
}
