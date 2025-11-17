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

class lifeCycleTask extends simplePlugin
{
  protected $displayHeader = TRUE;
  /**
   * @var array|array[]
   */
  private $subStates;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Life Cycle'),
      'plDescription'  => _('Tasks Life Cycle Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=lifecycle&size=48',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksLifeCycle'],
      'plFilter'       => '(objectClass=fdTasksLifeCycle)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [ // The name of the class is required, not the object class.
        'auditTask',
        'archiveTask',
        'automaticGroupsTask',
        'extractorTask',
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
        'class' => ['fullwidth', 'information'],
        'name'  => _('We recommend the task redundancy to be set to daily, and the case New Members Only to be unpicked'),
        'attrs' => [
          // This hiddenAttribute is only present to show the information label on the section.
          new HiddenAttribute(
            'fdShowInformation', FALSE,
          ),
          // Following attributes will contain all the DNs required to be verified by Orchestrator.
          new HiddenArrayAttribute('fdTasksLifeCycleListOfDN', FALSE, ''),
        ]
      ],
      'section2'           => [
        'name'  => _('Future resources details'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources or Regex'),
            'fdTasksLifeCyclePostResource', TRUE
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksLifeCyclePostState', TRUE
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksLifeCyclePostSubState', FALSE
          ),
          new IntAttribute(
            _('Extra days to add'), _('Extra days to be added after today'),
            'fdTasksLifeCyclePostEndDate', FALSE, '0', FALSE, '0'
          ),
        ]
      ],
      // Attributes are grouped by section
      'section3'           => [
        'name'  => _('Current resources details'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources or Regex'),
            'fdTasksLifeCyclePreResource', TRUE
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksLifeCyclePreState', TRUE
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksLifeCyclePreSubState', FALSE
          ),
        ]
      ],
      'sectionRegexPattern'      => [
        'name'  => _('Regex Pattern Configuration'),
        'class' => ['fullwidth'],
        'attrs' => [
          new StringAttribute(
            _('Regex Pattern'), _('Pattern to match supannRessource name (e.g., "COMPTE" or "MAIL"). This pattern is used when "Regex Pattern" is selected as a resource type. It does not filter by state or sub-state.'),
            'fdTasksLifeCycleRegexPattern', FALSE, '', '',
            '/^.*$/'
          ),
        ]
      ],
      'sectionAccountClosure' => [
        'name'  => _('Automatic Account Closure'),
        'class' => ['fullwidth'],
        'attrs' => [
          new BooleanAttribute(
            _('Enable Account Closure'),
            _('If ticked, the account will be locked if all "Current Resources" (either the single selected resource, or all resources matching the "Regex Pattern" if that option is chosen for "Current resources details") are in a "Suspended" (S) or "Inactive" (I) state, AND none of these resources are "Active" (A).'),
            'fdTasksLifeCycleEnableAccountClosure',
            FALSE, // Required
            FALSE  // Default value
          ),
        ]
      ],
      'section4' => [
        'name'  => _('Recipients Users and/or Groups'),
        'class' => ['fullwidth'],
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Members'), _('Users or groups to assign to this task.'),
            'fdTasksLifeCycleMembers', TRUE
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

    // Setup the behavior for resource selection to update regex field states
    $this->attributesAccess['fdTasksLifeCyclePreResource']->setSubmitForm('updateRegexState');
    $this->attributesAccess['fdTasksLifeCyclePostResource']->setSubmitForm('updateRegexState');

    // Setup behavior for account closure checkbox
    $this->attributesAccess['fdTasksLifeCycleEnableAccountClosure']->setSubmitForm('updateAccountClosureState');

    // Initial state updates
    $this->updateRegexState();
    $this->updateAccountClosureState();

    if (class_available('supannAccountStatus')) {
      $this->setSupannStates();
    } else {
      $warning = new FusionDirectoryWarning(htmlescape(_('The plugin Supann does not seem to be installed !')));
      $warning->display();
    }
  }

  /**
   * Toggle fields based on regex activation state and manage 'required' status.
   * State/SubState fields are enabled by default here; updateAccountClosureState handles disabling them.
   */
  function updateRegexState ()
  {
    $preResourceValue   = $this->attributesAccess['fdTasksLifeCyclePreResource']->getValue();
    $postResourceValue  = $this->attributesAccess['fdTasksLifeCyclePostResource']->getValue();
    $accountClosureEnabled = $this->attributesAccess['fdTasksLifeCycleEnableAccountClosure']->getValue();

    $isPreResourceRegex  = ($preResourceValue === 'REGEX');
    $isPostResourceRegex = ($postResourceValue === 'REGEX');

    // --- Handle Pre-Resource fields (Current resources details) ---
    // Enable PreState and PreSubState by default from this function's perspective.
    // updateAccountClosureState will disable them if account closure is enabled.
    if (!$accountClosureEnabled) {
        $this->attributesAccess['fdTasksLifeCyclePreState']->setDisabled(FALSE);
        $this->attributesAccess['fdTasksLifeCyclePreSubState']->setDisabled(FALSE);
        // PreState is required if PreResource is selected and account closure is not enabled.
        $this->attributesAccess['fdTasksLifeCyclePreState']->setRequired(!empty($preResourceValue));
    } else {
        // If account closure is enabled, PreState is not required.
        $this->attributesAccess['fdTasksLifeCyclePreState']->setRequired(FALSE);
    }


    // --- Handle Post-Resource fields (Future resources details) ---
    // Enable PostState and PostSubState by default from this function's perspective.
    // updateAccountClosureState will disable them (and PostResource/PostEndDate) if account closure is enabled.
    if (!$accountClosureEnabled) {
        $this->attributesAccess['fdTasksLifeCyclePostState']->setDisabled(FALSE);
        $this->attributesAccess['fdTasksLifeCyclePostSubState']->setDisabled(FALSE);
        // PostState is required if PostResource is selected and account closure is not enabled.
        $this->attributesAccess['fdTasksLifeCyclePostState']->setRequired(!empty($postResourceValue));
    } else {
        // If account closure is enabled, PostState is not required.
        $this->attributesAccess['fdTasksLifeCyclePostState']->setRequired(FALSE);
    }


    // --- Handle Regex Pattern field ---
    // Enable Regex Pattern field if EITHER PreResource OR PostResource is 'REGEX'
    $enableRegexPatternField = $isPreResourceRegex || $isPostResourceRegex;
    $this->attributesAccess['fdTasksLifeCycleRegexPattern']->setDisabled(!$enableRegexPatternField);
    $this->attributesAccess['fdTasksLifeCycleRegexPattern']->setRequired($enableRegexPatternField);
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

    $this->attributesAccess['fdTasksLifeCyclePreResource']->setChoices(array_keys($resources), array_values($resources));
    $this->attributesAccess['fdTasksLifeCyclePostResource']->setChoices(array_keys($resources), array_values($resources));
    $this->subStates = supannAccountStatus::getConfiguredSubstates(); //Keys are states and Values are subStates
    $this->attributesAccess['fdTasksLifeCyclePreState']->setChoices(array_keys($this->subStates));
    $this->attributesAccess['fdTasksLifeCyclePostState']->setChoices(array_keys($this->subStates));

    // Allows the sub states to be listed when state is modified.
    $this->attributesAccess['fdTasksLifeCyclePreState']->setSubmitForm('updateFieldsChoices');
    $this->attributesAccess['fdTasksLifeCyclePostState']->setSubmitForm('updateFieldsChoices');
    // This initial fill the list choices based on registered states.
    $this->updateFieldsChoices();

  }

  /* Update list of subStates which depends on the state selected */
  function updateFieldsChoices ()
  {
    // Update pre sub states
    $preSubStatesList = $this->subStates[$this->attributesAccess['fdTasksLifeCyclePreState']->getValue()] ?? [];
    $this->attributesAccess['fdTasksLifeCyclePreSubState']->setChoices(array_keys($preSubStatesList), array_values($preSubStatesList));
    // Update post sub states
    $postSubStatesList = $this->subStates[$this->attributesAccess['fdTasksLifeCyclePostState']->getValue()] ?? [];
    $this->attributesAccess['fdTasksLifeCyclePostSubState']->setChoices(array_keys($postSubStatesList), array_values($postSubStatesList));
  }

  /**
   * Retrieve all the DNs from groups or members
   */
  public function getListOfDN (): void
  {
    // Get the members or groups selected
    $membersAndGroups = $this->attributesAccess['fdTasksLifeCycleMembers']->getValue();
    // Call static function from tasks object, always return an array.
    $this->attributesAccess['fdTasksLifeCycleListOfDN']->setValue(array_values(tasks::extractMembersFromGroups($membersAndGroups)));
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
    $this->parent->getBaseObject()->createSlaveTasks($listOfDN, $attributeType, NULL, 'Life Cycle');
  }

  public function update (): bool
  {
    parent::update();

    // Toggle fields based on resource selection state
    $this->updateRegexState();

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

  /**
   * Toggle fields based on account closure state
   */
  function updateAccountClosureState ()
  {
    $accountClosureEnabled = $this->attributesAccess['fdTasksLifeCycleEnableAccountClosure']->getValue();

    // --- Current resources details ---
    $this->attributesAccess['fdTasksLifeCyclePreState']->setDisabled($accountClosureEnabled);
    $this->attributesAccess['fdTasksLifeCyclePreSubState']->setDisabled($accountClosureEnabled);
    if ($accountClosureEnabled) {
      $this->attributesAccess['fdTasksLifeCyclePreState']->setRequired(FALSE);
    }
    // If not $accountClosureEnabled, updateRegexState will handle setRequired for PreState.

    // --- Future resources details ---
    $this->attributesAccess['fdTasksLifeCyclePostResource']->setDisabled($accountClosureEnabled);
    $this->attributesAccess['fdTasksLifeCyclePostState']->setDisabled($accountClosureEnabled);
    $this->attributesAccess['fdTasksLifeCyclePostSubState']->setDisabled($accountClosureEnabled);
    $this->attributesAccess['fdTasksLifeCyclePostEndDate']->setDisabled($accountClosureEnabled);
    if ($accountClosureEnabled) {
      $this->attributesAccess['fdTasksLifeCyclePostState']->setRequired(FALSE);
    }
    // If not $accountClosureEnabled, updateRegexState will handle setRequired for PostState.

    // Call updateRegexState to ensure 'required' status and Regex field are correctly set
    // based on current resource selections, especially when disabling account closure.
    $this->updateRegexState();
  }
}
