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
      'plIcon'         => 'geticon.php?context=applications&icon=automatic-groups&size=48',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksAutomaticGroups'],
      'plFilter'       => '(objectClass=fdTasksAutomaticGroups)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [ // The name of the class is required, not the object class.
        'tasksMail',          // from mail plugin (core)
        'taskReminder',       // from user-reminder plugin
        'tasksNotifications', // from notifications plugin
        'archiveTask',        // from archive plugin
        'extractorTask',      // from extractor plugin (if present)
        'tasksLifeCycle',     // from lifecycle plugin (if present)
        'auditTask',          // from archive plugin (if present)
        //'tasksAutomaticGroups', // from automatic-groups plugin
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
          // Hidden attributes for dynamic group settings
          new HiddenAttribute('fdTasksAutomaticGroupsDynamicURL', FALSE),
          new HiddenAttribute('fdTasksAutomaticGroupsDynamicName', FALSE),
        ]
      ],
      'modeSection' => [
        'name'  => _('Task Mode'),
        'class' => ['fullwidth'],
        'attrs' => [
          new BooleanAttribute(
            _('Create Dynamic Group'), _('Create a dynamic group based on SupAnn resource, state and substate'),
            'fdTasksAutomaticGroupsDynamicGroup', FALSE, FALSE
          ),
        ]
      ],
      'supannSection' => [
        'name'  => _('Resource State Criteria'),
        'class' => ['fullwidth'],
        'attrs' => [
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
      'manualSection' => [
        'name'  => _('Manual Group Assignment'),
        'class' => ['fullwidth'],
        'attrs' => [
          new ObjectSelectAttribute(
            _('Target Group'), _('Group where users will be added/removed'),
            'fdTasksAutomaticGroupsOfName', TRUE,
            ['ogroup'], 'dn', 'cn',
            '(objectClass=groupOfNames)'
          ),
          new UsersGroupsRolesAttribute(
            _('Members'), _('Users or groups to check for automatic group assignment.'),
            'fdTasksAutomaticGroupsMembers', TRUE
          ),
        ]
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
    // Set these attributes to be stored in LDAP
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicURL']->setInLdap(TRUE);
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicName']->setInLdap(TRUE);

    // Set up the dynamic behavior for the dynamic group toggle
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicGroup']->setSubmitForm('updateDynamicGroupFields');

    // Set up triggers to update the dynamic group URL and name
    $this->attributesAccess['fdTasksAutomaticGroupsResource']->setSubmitForm('updateDynamicGroupSettings');
    $this->attributesAccess['fdTasksAutomaticGroupsState']->setSubmitForm('updateDynamicGroupSettings');
    $this->attributesAccess['fdTasksAutomaticGroupsSubState']->setSubmitForm('updateDynamicGroupSettings');

    // SupAnn Status management
    if (class_available('supannAccountStatus')) {
      $this->setSupannStates();
    } else {
      $warning = new FusionDirectoryWarning(htmlescape(_('The plugin Supann does not seem to be installed!')));
      $warning->display();
    }

    // Initialize fields state
    $this->updateDynamicGroupFields();
  }

  /**
   *  Set attributes values with defined supann states from configuration
   */
  protected function setSupannStates (): void
  {
    global $config;

    // Define the mandatory ones and get the remaining from configuration.
    $resources = supannAccountStatus::getResources();
    $states    = supannAccountStatus::getStates();

    $this->attributesAccess['fdTasksAutomaticGroupsResource']->setChoices(array_keys($resources), array_values($resources));
    $this->subStates = supannAccountStatus::getConfiguredSubstates(); //Keys are states and Values are subStates
    $this->attributesAccess['fdTasksAutomaticGroupsState']->setChoices(array_keys($states), array_values($states));

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
   * Toggle fields based on dynamic group selection
   */
  function updateDynamicGroupFields ()
  {
    $isDynamicGroup = $this->attributesAccess['fdTasksAutomaticGroupsDynamicGroup']->getValue();

    // Toggle only the manual group assignment fields
    $this->attributesAccess['fdTasksAutomaticGroupsOfName']->setRequired(!$isDynamicGroup);
    $this->attributesAccess['fdTasksAutomaticGroupsOfName']->setDisabled($isDynamicGroup);
    $this->attributesAccess['fdTasksAutomaticGroupsMembers']->setDisabled($isDynamicGroup);

    // Update the dynamic group settings if we're in dynamic group mode
    if ($isDynamicGroup) {
      $this->updateDynamicGroupSettings();
    }
  }

  /**
   * Update the hidden dynamic group URL and name based on the selected criteria
   */
  function updateDynamicGroupSettings ()
  {
    $isDynamicGroup = $this->attributesAccess['fdTasksAutomaticGroupsDynamicGroup']->getValue();

    if (!$isDynamicGroup) {
      return;
    }

    // Get the values needed to generate the URL and group name
    $resource = $this->attributesAccess['fdTasksAutomaticGroupsResource']->getValue();
    $state = $this->attributesAccess['fdTasksAutomaticGroupsState']->getValue();
    $subState = $this->attributesAccess['fdTasksAutomaticGroupsSubState']->getValue();

    // Only proceed if we have the minimum required information
    if (empty($resource) || empty($state)) {
      return;
    }

    // Generate the dynamic group name - ensure all parts are lowercase
    $groupName = 'dynamic-' . strtolower($resource) . '-' . strtolower($state);
    if (!empty($subState)) {
      $groupName .= '-' . strtolower($subState);
    }

    // Generate the LDAP filter - keep original case for the filter
    $filter = '';
    if (!empty($subState)) {
      $filter = "(supannRessourceEtat={" . $resource . "}" . $state . ":" . $subState . ")";
    } else {
      $filter = "(supannRessourceEtat={" . $resource . "}" . $state . ")";
    }

    // Get the base DN from configuration or use a default
    global $config;
    $baseDN    = $config->current['BASE'];

    // Create the complete LDAP URL
    $ldapUrl = "ldap:///" . $baseDN . "??one?" . $filter;

    // Set the hidden field values
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicName']->setValue($groupName);
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicURL']->setValue($ldapUrl);
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
    // Check if we're creating a dynamic group or using regular automatic groups
    $isDynamicGroup = $this->attributesAccess['fdTasksAutomaticGroupsDynamicGroup']->getValue();

    if ($isDynamicGroup) {
      // Ensure dynamic URL and name are up to date
      $this->updateDynamicGroupSettings();

      // Create just one task for dynamic group creation
      $this->parent->getBaseObject()->createSlaveTasks(['dynamicGroupTask'], 'fdTasksGranularDN', NULL, 'Dynamic-Group');
    } else {
      // Original code for regular automatic groups
      $listOfDN = $this->attributesAccess['fdTasksAutomaticGroupsListOfDN']->getValue();
      $attributeType = 'fdTasksGranularDN';

      // Call the method from parent tasks object (first tab) to create sub-tasks.
      $this->parent->getBaseObject()->createSlaveTasks($listOfDN, $attributeType, NULL, 'Automatic Groups');
    }
  }

  public function update (): bool
  {
    parent::update();

    // If in dynamic group mode, update the dynamic group settings
    $isDynamicGroup = $this->attributesAccess['fdTasksAutomaticGroupsDynamicGroup']->getValue();
    if ($isDynamicGroup) {
      $this->updateDynamicGroupSettings();
    } else {
      // Take the list of DN based on the groups or members list selected and fill the related attributes
      $this->getListOfDN();
    }

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
