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

class automaticGroupsTask extends simplePlugin
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
      'plSmallIcon'    => 'geticon.php?context=applications&icon=automatic-groups&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksAutomaticGroups'],
      'plFilter'       => '(objectClass=fdTasksAutomaticGroups)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [ // The name of the class is required, not the object class.
        'auditTask',
        'archiveTask',
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
      'modeSection' => [
        'name'  => _('Task Mode'),
        'class' => ['fullwidth'],
        'attrs' => [
          new BooleanAttribute(
            _('Create Dynamic Group'), _('Create a dynamic group based on SupAnn resource, state and substate'),
            'fdTasksAutomaticGroupsDynamicGroup', FALSE, FALSE
          ),
          new HiddenAttribute(
          'fdTasksAutomaticGroupsDynamicURL', FALSE
          ),
          new HiddenAttribute(
          'fdTasksAutomaticGroupsDynamicName', FALSE
          ),
        ]
      ],
      'supannSection' => [
        'name'  => _('Resource State Criteria'),
        'class' => ['fullwidth'],
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources or Regex'),
            'fdTasksAutomaticGroupsPreResource', TRUE
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksAutomaticGroupsPreState', TRUE
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksAutomaticGroupsPreSubState', FALSE
          ),
        ]
      ],
      'sectionRegexPattern'      => [
        'name'  => _('Regex Pattern Configuration'),
        'class' => ['fullwidth'],
        'attrs' => [
          new StringAttribute(
            _('Regex Pattern'), _('Pattern to match supannRessource name (e.g., "COMPTE" or "MAIL"). This pattern is used when "Regex Pattern" is selected as a resource type. It does not filter by state or sub-state.'),
            'fdTasksAutomaticGroupsRegexPattern', FALSE, '', '',
            '/^.*$/'
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

    // Set these attributes to be stored in LDAP
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicURL']->setInLdap(TRUE);
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicName']->setInLdap(TRUE);

    // Set up the dynamic behavior for the dynamic group toggle
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicGroup']->setSubmitForm('updateDynamicGroupFields');

    // Set up triggers to update the dynamic group URL and name
    $this->attributesAccess['fdTasksAutomaticGroupsPreResource']->setSubmitForm('updateDynamicGroupSettings');
    $this->attributesAccess['fdTasksAutomaticGroupsPreState']->setSubmitForm('updateDynamicGroupSettings');
    $this->attributesAccess['fdTasksAutomaticGroupsPreSubState']->setSubmitForm('updateDynamicGroupSettings');

    // Setup the behavior for resource selection to update regex field states
    $this->attributesAccess['fdTasksAutomaticGroupsPreResource']->setSubmitForm('updateRegexState');

    // Initial state update
    $this->updateRegexState();

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
   * Toggle fields based on regex activation state
   */
  function updateRegexState ()
  {
    $preResourceValue   = $this->attributesAccess['fdTasksAutomaticGroupsPreResource']->getValue();

    $isPreResourceRegex  = ($preResourceValue === 'REGEX');

    // --- Handle Pre-Resource fields (Current resources details) ---
    // PreState is required if PreResource is selected (it's defined as required initially in getAttributesInfo)
    $this->attributesAccess['fdTasksAutomaticGroupsPreResource']->setRequired(!empty($preResourceValue));
    // PreSubState is never required by this logic, its requirement is static.

    // --- Handle Regex Pattern field ---
    // Enable Regex Pattern field if EITHER PreResource is 'REGEX'
    $enableRegexPatternField = $isPreResourceRegex;
    $this->attributesAccess['fdTasksAutomaticGroupsRegexPattern']->setDisabled(!$enableRegexPatternField);
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

    // Add REGEX option
    $resources['REGEX'] = _('Regex Pattern');

    $this->attributesAccess['fdTasksAutomaticGroupsPreResource']->setChoices(array_keys($resources), array_values($resources));
    $this->subStates = supannAccountStatus::getConfiguredSubstates(); //Keys are states and Values are subStates
    $this->attributesAccess['fdTasksAutomaticGroupsPreState']->setChoices(array_keys($states), array_values($states));

    // Allows the sub states to be listed when state is modified.
    $this->attributesAccess['fdTasksAutomaticGroupsPreState']->setSubmitForm('updateFieldsChoices');
    // This initial fill the list choices based on registered states.
    $this->updateFieldsChoices();

  }

  /* Update list of subStates which depends on the state selected */
  function updateFieldsChoices ()
  {
    // Update pre sub states
    $preSubStatesList = $this->subStates[$this->attributesAccess['fdTasksAutomaticGroupsPreState']->getValue()] ?? [];
    $this->attributesAccess['fdTasksAutomaticGroupsPreSubState']->setChoices(array_keys($preSubStatesList), array_values($preSubStatesList));
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
    $resource = $this->attributesAccess['fdTasksAutomaticGroupsPreResource']->getValue();
    $state = $this->attributesAccess['fdTasksAutomaticGroupsPreState']->getValue();
    $subState = $this->attributesAccess['fdTasksAutomaticGroupsPreSubState']->getValue();

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
    $ldapUrl = "ldap:///" . $baseDN . "??sub?" . $filter;

    // Set the hidden field values
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicName']->setValue($groupName);
    $this->attributesAccess['fdTasksAutomaticGroupsDynamicURL']->setValue($ldapUrl);
  }

  /**
   * Generate slave tasks for automatic group assignment
   */
  public function generateSubTasks ()
  {
    // Check if we're creating a dynamic group or using regular automatic groups
    $isDynamicGroup = $this->attributesAccess['fdTasksAutomaticGroupsDynamicGroup']->getValue();

    if ($isDynamicGroup) {
      // Ensure dynamic URL and name are up to date
      $this->updateDynamicGroupSettings();

      // Create just one task for dynamic group creation
      $this->parent->getBaseObject()->createSubTasks(['dynamicGroupTask'], 'fdTasksGranularDN', NULL, 'automaticGroupsDynamic');
    } else {
      // Original code for regular automatic groups
      $membersAndGroups = $this->attributesAccess['fdTasksAutomaticGroupsMembers']->getValue();
      $attributeType = 'fdTasksGranularDN';

      // Call the method from parent tasks object (first tab) to create sub-tasks.
      $this->parent->getBaseObject()->createSubTasks($membersAndGroups, $attributeType, NULL, 'automaticGroups');
    }
  }

  public function update (): bool
  {
    parent::update();

    // Toggle fields based on resource selection state
    $this->updateRegexState();

    // If in dynamic group mode, update the dynamic group settings
    $isDynamicGroup = $this->attributesAccess['fdTasksAutomaticGroupsDynamicGroup']->getValue();
    if ($isDynamicGroup) {
      $this->updateDynamicGroupSettings();
    }

    return TRUE;
  }

  function save (): array
  {
    // Verify if this tasks has to be executed upon saving.
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

    if ($execTasks) {
      $this->generateSubTasks();
    }

    return parent::save();
  }
}
