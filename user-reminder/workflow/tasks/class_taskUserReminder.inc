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

class taskUserReminder extends simplePlugin
{
  protected $displayHeader = TRUE;
  // LastExec comes from the generic task, it must be filled before saving to get previous lastExec.
  // Indeed, generic task is saved prior of added tasks plugins. Making lastExec with an updated value before this class
  // being processed.
  protected $lastExec = NULL;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks UserReminder'),
      'plDescription'  => _('Tasks User Reminder Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=userReminder&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksUserReminder'],
      'plFilter'       => '(objectClass=fdTasksUserReminder)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => ['tasksMail', 'tasksLifeCycle', 'tasksNotifications', 'auditTasks'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1'                => [
        'name'  => _('User Reminder settings'),
        'attrs' => [
          // Following attribute contain the list of members emails that should receive a notification.
          new HiddenArrayAttribute('fdTasksUserReminderListOfRecipientsMails', FALSE, ''),
          new IntAttribute(
            _('First call'), _('Numbers of days before reaching resource end date'),
            'fdTasksUserReminderFirstCall', TRUE, '0', FALSE, '30'
          ),
          new IntAttribute(
            _('Second call'), _('Numbers of days before reaching resource end date'),
            'fdTasksUserReminderSecondCall', TRUE, '0', FALSE, '15'
          ),
          new BooleanAttribute(
            _('Posix'), _('Monitor posix end date'),
            'fdTasksUserReminderPosix', FALSE,
            FALSE
          ),
          new BooleanAttribute(
            _('PPolicy'), _('Monitoring PPolicy end date'),
            'fdTasksUserReminderPPolicy', FALSE,
            FALSE
          ),
          new BooleanAttribute(
            _('Manager'), _('Provide alert to the manager'),
            'fdTasksUserReminderManager', FALSE,
            FALSE
          ),
        ]
      ],
      // Attributes are grouped by section
      'section2'                => [
        'name'  => _('SupannRessourceEtat to be monitored '),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksUserReminderResource', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksUserReminderState', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksUserReminderSubState', FALSE, [], "None"
          ),
        ]
      ],
      'section3'                => [
        'name'  => _('New SupannRessourceEtat to be set'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksUserReminderNextResource', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksUserReminderNextState', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksUserReminderNextSubState', FALSE, [], "None"
          ),
        ]
      ],
      'UserGroupSelection'      => [
        'name'  => _('Recipients Users and/or Groups'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Recipients'), _('Users or Groups requiring to be notified.'),
            'fdTasksUserReminderRecipientsMembers', TRUE
          ),
        ],
      ],
      'Mail Template Selection' => [
        'name'  => _('Mail Template'),
        'attrs' => [
          new SelectAttribute(
            _('Mail Template'), _('Mail Template Object Selection'),
            'fdTasksUserReminderMailTemplate', TRUE
          ),
          new MailAttribute(
            _('Sender email address'),
            _('Email address from which mails will be sent'), 'fdTasksUserReminderEmailSender',
            TRUE, 'to.be@chang.ed'),
        ],
      ],
    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    // Set the lastExec from generic task before it being updated upon save.
    $this->lastExec = $this->parent->getBaseObject()->fdTasksLastExec ?? NULL;

    // Fill in the list of mail template available to the attribute
    $mailTemplates = $this->getMailTemplates();
    $this->attributesAccess['fdTasksUserReminderMailTemplate']->setChoices(
      array_keys($mailTemplates), array_values($mailTemplates)
    );

    // Supann Status management
    if (class_available('supannAccountStatus')) {
      $this->setSupannStates();
    }
  }

  /**
   * @return void
   * Note : Simply get the existing supann States to the userReminder attributes.
   */
  protected function setSupannStates (): void
  {
    global $config;

    // Define the mandatory ones and get the remaining from configuration.
    $resources = ['COMPTE' => _('Account'), 'MAIL' => _('Mail'), 'NONE' => _('None')];

    foreach ($config->get_cfg_value('SupannRessourceLabels', []) as $line) {
      list($resource, $label) = explode(':', $line, 2);
      $resources[$resource] = $label;
    }
    $this->subStates = supannAccountStatus::getConfiguredSubstates(); //Keys are states and Values are subStates

    // Set the resource
    $this->attributesAccess['fdTasksUserReminderResource']->setChoices(array_keys($resources), array_values($resources));
    $this->attributesAccess['fdTasksUserReminderNextResource']->setChoices(array_keys($resources), array_values($resources));

    // Set the state
    $this->attributesAccess['fdTasksUserReminderState']->setChoices(array_keys($this->subStates));
    $this->attributesAccess['fdTasksUserReminderNextState']->setChoices(array_keys($this->subStates));

    // Allows the sub states to be listed when state is modified.
    $this->attributesAccess['fdTasksUserReminderState']->setSubmitForm('updateFieldsChoices');
    $this->attributesAccess['fdTasksUserReminderNextState']->setSubmitForm('updateFieldsChoices');
    // This initial fill the list choices based on registered states.
    $this->updateFieldsChoices();
  }

  /**
   * @return void
   * Note : Update list of subStates which depends on the state selected
   */
  function updateFieldsChoices ()
  {
    // Update the available supann states
    $subStatesList = $this->subStates[$this->attributesAccess['fdTasksUserReminderState']->getValue()] ?? [];
    $subStatesList = $this->subStates[$this->attributesAccess['fdTasksUserReminderNextState']->getValue()] ?? [];

    // Update subStates
    $this->attributesAccess['fdTasksUserReminderSubState']->setChoices(array_keys($subStatesList), array_values($subStatesList));
    $this->attributesAccess['fdTasksUserReminderNextSubState']->setChoices(array_keys($subStatesList), array_values($subStatesList));
  }

  /**
   * @return array
   * Note : Simply search for mail template and return the list.
   */
  protected function getMailTemplates (): array
  {
    global $config;
    //Search within LDAP and retrieve all mail objects for current base.
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdMailTemplate))', ['cn']);
    $tmpSearch = [];
    while ($attrs = $ldap->fetch()) {
      $tmpSearch[$attrs['cn'][0]] = $attrs['cn'][0];
    }
    asort($tmpSearch);

    return $tmpSearch;
  }

  /**
   * Retrieve all the DNs from groups or members
   */
  protected function getListOfDN (): void
  {
    // Get the members or groups selected
    $recipients = $this->attributesAccess['fdTasksUserReminderRecipientsMembers']->getValue();
    // Call static function from tasks object in order to get a real list of members and related emails directly.
    $emailsList = tasksMail::getEmailsFromSelectedDN('gosaMailAccount',
                                                     'mail',
                                                     tasks::extractMembersFromGroups($recipients)
    );

    // Fill the attribute with emails.
    $this->attributesAccess['fdTasksUserReminderListOfRecipientsMails']->setValue(array_values($emailsList));
  }


  /**
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  function generateSlaveTasks ()
  {
    $listOfDN = $this->attributesAccess['fdTasksUserReminderListOfNewAuditUsersDN']->getValue();
    // The attribute required to be search in createSlaveTasks
    $attributeType = 'fdTasksGranularDN';
    // Store the full associative array coming from the list of new audit user DN, containing all audit DN related to that user.
    $attrs['ref'] = $listOfDN;

    // Call the method from parent tasks object (first tab) to create sub-tasks. listOfDN keys contains the UID of the users.
    $this->parent->getBaseObject()->createSlaveTasks(array_keys($listOfDN), $attributeType, $attrs, 'userReminder');
  }

  /**
   * @return bool
   */
  function update (): bool
  {
    parent::update();

    // Take the list of DN based on the groups or members list selected and filled the related attributes
    $this->getListOfDN();

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
