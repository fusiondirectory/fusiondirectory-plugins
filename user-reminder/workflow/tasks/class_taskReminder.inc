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

class taskReminder extends simplePlugin
{
  protected $displayHeader = TRUE;
  // LastExec comes from the generic task, it must be filled before saving to get previous lastExec.
  // Indeed, generic task is saved prior of added tasks plugins. Making lastExec with an updated value before this class
  // being processed.
  protected $lastExec = NULL;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Reminder'),
      'plDescription'  => _('Tasks User Reminder Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=Reminder&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksReminder'],
      'plFilter'       => '(objectClass=fdTasksReminder)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => ['tasksMail', 'tasksLifeCycle', 'tasksNotifications', 'auditTasks'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1' => [
        'name'  => _('User Reminder general settings'),
        'attrs' => [
          // Following hidden attributes contain the list of members emails that should receive a reminder
          new HiddenArrayAttribute('fdTasksReminderListOfRecipientsMails', FALSE, ''),
          new IntAttribute(
            _('First reminder'), _('Numbers of days before reaching resource end date'),
            'fdTasksReminderFirstCall', TRUE, '0', FALSE, '30'
          ),
          new BooleanAttribute(
            _('Manager'), _('Provide alert to the manager'),
            'fdTasksReminderManager', FALSE,
            FALSE
          ),
        ]
      ],
      // Attributes are grouped by section
      'section2' => [
        'name'  => _('SupannRessourceEtat to be monitored '),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksReminderResource', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksReminderState', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksReminderSubState', FALSE, [], "None"
          ),
        ]
      ],
      'section3' => [
        'name'  => _('Account prolongation'),
        'class' => ['fullwidth', 'information'],
        'attrs' => [
          new BooleanAttribute(
            _('Account prolongation'), _('Activate / Deactivate account prolongation (token issue)'),
            'fdTasksReminderAccountProlongation', FALSE,
            FALSE
          ),
          new IntAttribute(
            _('Second reminder'), _('Numbers of days before reaching resource end date'),
            'fdTasksReminderSecondCall', FALSE, '0', FALSE, '15'
          ),
          new BooleanAttribute(
            _('Posix'), _('Monitor posix end date'),
            'fdTasksReminderPosix', FALSE,
            FALSE
          ),
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksReminderNextResource', FALSE, [], "None"
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksReminderNextState', FALSE, [], "None"
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksReminderNextSubState', FALSE, [], "None"
          ),
          new IntAttribute(
            _('Account added duration'), _('Numbers of days to add to the new end date of supann resource'),
            'fdTasksReminderSupannNewEndDate', FALSE, '0', FALSE, '0'
          ),
        ]
      ],
      'section4' => [
        'name'  => _('Mail Template'),
        'attrs' => [
          new SelectAttribute(
            _('Mail Template'), _('Mail Template Object Selection'),
            'fdTasksReminderMailTemplate', TRUE
          ),
          new MailAttribute(
            _('Sender email address'),
            _('Email address from which mails will be sent'), 'fdTasksReminderEmailSender',
            TRUE, 'to.be@chang.ed'),
        ],
      ],
      'section5' => [
        'name'  => _('Recipients Users and/or Groups'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Forward To'), _('Users or Groups requiring to be notified.'),
            'fdTasksReminderRecipientsMembers', FALSE
          ),
          new UsersGroupsRolesAttribute(
            _('Monitored Members'), _('Users or Groups requiring monitoring.'),
            'fdTasksReminderMembers', TRUE
          ),
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
    $this->attributesAccess['fdTasksReminderMailTemplate']->setChoices(
      array_keys($mailTemplates), array_values($mailTemplates)
    );

    // Supann Status management
    if (class_available('supannAccountStatus')) {
      $this->setSupannStates();
    }

    $this->attributesAccess['fdTasksReminderAccountProlongation']->setManagedAttributes(
      [
        'disable' => [
          FALSE => [
            'fdTasksReminderSecondCall',
            'fdTasksReminderNextResource',
            'fdTasksReminderPosix',
            'fdTasksReminderNextResource',
            'fdTasksReminderNextState',
            'fdTasksReminderNextSubState',
            'fdTasksReminderSupannNewEndDate'
          ]
        ]
      ]
    );
  }

  /**
   * @return void
   * Note : Simply get the existing supann States to the Reminder attributes.
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
    $this->attributesAccess['fdTasksReminderResource']->setChoices(array_keys($resources), array_values($resources));
    $this->attributesAccess['fdTasksReminderNextResource']->setChoices(array_keys($resources), array_values($resources));

    // Set the state
    $this->attributesAccess['fdTasksReminderState']->setChoices(array_keys($this->subStates));
    $this->attributesAccess['fdTasksReminderNextState']->setChoices(array_keys($this->subStates));

    //    // Allows the sub states to be listed when state is modified.
    $this->attributesAccess['fdTasksReminderState']->setSubmitForm('updateFieldsChoices');
    $this->attributesAccess['fdTasksReminderNextState']->setSubmitForm('updateFieldsChoices');
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
    $subStatesList     = $this->subStates[$this->attributesAccess['fdTasksReminderState']->getValue()] ?? [];
    $subStatesNextList = $this->subStates[$this->attributesAccess['fdTasksReminderNextState']->getValue()] ?? [];

    // Update subStates
    $this->attributesAccess['fdTasksReminderSubState']->setChoices(array_keys($subStatesList), array_values($subStatesList));
    $this->attributesAccess['fdTasksReminderNextSubState']->setChoices(array_keys($subStatesNextList), array_values($subStatesNextList));
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
   * Retrieve all the DNs from groups or members, recipients and monitored members.
   * As both require receiving a reminder.
   */
  protected function getListOfDN (): void
  {
    // Get the members or groups selected
    $recipients = $this->attributesAccess['fdTasksReminderRecipientsMembers']->getValue();

    // Call static function from tasks object in order to get a real list of members and related emails directly.
    $recipientsEmailsList = tasksMail::getEmailsFromSelectedDN('gosaMailAccount',
                                                               'mail',
                                                               tasks::extractMembersFromGroups($recipients));

    // Fill the attribute with emails.
    $this->attributesAccess['fdTasksReminderListOfRecipientsMails']->setValue(array_values($recipientsEmailsList));
  }


  /**
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  function generateSlaveTasks ($monitoredDN)
  {
    // The attribute required to be search in createSlaveTasks
    $attributeType                       = 'fdTasksGranularDN';
    $firstCall['helper']['reminderTime'] = $this->attributesAccess['fdTasksReminderFirstCall']->getValue();

    // Call the method from parent tasks object (first tab) to create sub-tasks. listOfDN keys contains the UID of the users.
    // Create the first sub task with primary call
    $this->parent->getBaseObject()->createSlaveTasks($monitoredDN, $attributeType, $firstCall, 'Reminder');

    // Create the second sub task with secondary call (if present).
    if (!empty($this->attributesAccess['fdTasksReminderSecondCall']->getValue() &&
      $this->attributesAccess['fdTasksReminderAccountProlongation']->getValue())) {

      $secondCall['helper']['reminderTime'] = $this->attributesAccess['fdTasksReminderSecondCall']->getValue();
      $this->parent->getBaseObject()->createSlaveTasks($monitoredDN, $attributeType, $secondCall, 'Reminder');
    }
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

    // Get the DN list extracted from members and groups of users to serve the slave task.
    $monitoredMembers = $this->attributesAccess['fdTasksReminderMembers']->getValue();

    if ($execTasks) {
      $this->generateSlaveTasks(tasks::extractMembersFromGroups($monitoredMembers));
    }

    return parent::save();
  }
}
