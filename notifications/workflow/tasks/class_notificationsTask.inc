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

class notificationsTask extends simplePlugin
{
  protected $displayHeader = TRUE;
  // lastActivation comes from the generic task, it must be filled before saving to get previous lastActivation.
  // Indeed, generic task is saved prior of added tasks plugins. Making lastActivation with an updated value before this class
  // being processed.
  protected $lastActivation = NULL;

  public $subStates;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Notifications'),
      'plDescription'  => _('Tasks Notifications Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=notifications&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksNotifications'],
      'plFilter'       => '(objectClass=fdTasksNotifications)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [ // The name of the class is required, not the object class.
        'auditTask',
        'archiveTask',
        'automaticGroupsTask',
        'extractorTask',
        'lifeCycleTask',
        'mailTask',
        'reminderTask',
      ],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1'                => [
        'name'  => _('Modified attributes list to be notified'),
        'attrs' => [
          // Following attribute will contain all the DNs where a new snapshot was taken, required by orchestrator.
          new HiddenArrayAttribute('fdTasksNotificationsListOfNewAuditUsersDN', FALSE, ''),
          // Following attribute contain the list of members emails that should receive a notification.
          new HiddenArrayAttribute('fdTasksNotificationsListOfRecipientsMails', FALSE, ''),
          new SetAttribute(
            new SelectAttribute(
              'Attributes', _('Any changes listed to set attributes will trigger a notification'),
              'fdTasksNotificationsAttributes', TRUE
            ),
          ),
        ]
      ],
      'Future ressources details' => [
        'name'  => _('Future resources details'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksNotificationsPostResource', FALSE, [], "None"
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksNotificationsPostState', FALSE, [], "None"
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksNotificationsPostSubState', FALSE, [], "None"
          ),
        ]
      ],
      'UserGroupSelection'      => [
        'name'  => _('Recipients Users and/or Groups'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Recipients'), _('Users or Groups requiring to be notified.'),
            'fdTasksNotificationsRecipientsMembers', TRUE
          ),
          new UsersGroupsRolesAttribute(
            _('Monitored Members'), _('Users or Groups requiring monitoring.'),
            'fdTasksNotificationsMonitoredMembers', TRUE
          ),
        ],
      ],
      // Attributes are grouped by section
      'supannSection'           => [
        'name'  => _('Add notification when the following Supann state is reached (supannRessourceEtat)'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksNotificationsResource', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksNotificationsState', TRUE, [], "None"
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksNotificationsSubState', FALSE, [], "None"
          ),
        ]
      ],
      'Mail Template Selection' => [
        'name'  => _('Mail Template'),
        'attrs' => [
          new SelectAttribute(
            _('Mail Template'), _('Mail Template Object Selection'),
            'fdTasksNotificationsMailTemplate', TRUE
          ),
          new MailAttribute(
            _('Sender email address'),
            _('Email address from which mails will be sent'), 'fdTasksNotificationsEmailSender',
            TRUE, 'to.be@chang.ed'),
        ],
      ],
    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    // Set the list of available attributes to follow in the set selection
    $this->attributesAccess['fdTasksNotificationsAttributes']->attribute->setChoices($this->getAttributesListFromConf());

    // Set the lastActivation from generic task before it being updated upon save.
    $this->lastActivation = $this->parent->getBaseObject()->fdTasksLastActivation ?? NULL;

    // This avoids keeping obsolete AuditUsersDN once subtasks have been generated.
    $this->attributesAccess['fdTasksNotificationsListOfNewAuditUsersDN']->setInLdap(FALSE);

    // Fill in the list of mail template available to the attribute
    $mailTemplates = $this->getMailTemplates();
    $this->attributesAccess['fdTasksNotificationsMailTemplate']->setChoices(
      array_keys($mailTemplates), array_values($mailTemplates)
    );

    // Supann Status management
    if (class_available('supannAccountStatus')) {
      $this->setSupannStates();
    }
  }

  /**
   * @return void
   * Note : Simply get the existing supann States to the notifications attributes.
   */
  protected function setSupannStates (): void
  {
    global $config;

    // Define the mandatory ones and get the remaining from configuration.
    $resources       = supannAccountStatus::getResources();
    $states          = supannAccountStatus::getStates();
    $this->subStates = supannAccountStatus::getConfiguredSubstates(); //Keys are states and Values are subStates

    // Montired supann
    $this->attributesAccess['fdTasksNotificationsResource']->setChoices(array_keys($resources), array_values($resources));
    $this->attributesAccess['fdTasksNotificationsState']->setChoices(array_keys($states), array_values($states));

    // Allows the sub states to be listed when state is modified (monitored)
    $this->attributesAccess['fdTasksNotificationsState']->setSubmitForm('updateFieldsChoices');

    // Next supann value
    $this->attributesAccess['fdTasksNotificationsPostResource']->setChoices(array_keys($resources), array_values($resources));
    $this->attributesAccess['fdTasksNotificationsPostState']->setChoices(array_keys($states), array_values($states));

    // Allows the sub states to be listed when state is modified (next supann value)
    $this->attributesAccess['fdTasksNotificationsPostState']->setSubmitForm('updateFieldsChoices');

    // This initial fill the list choices based on registered states.
    $this->updateFieldsChoices();
  }

  /**
   * @return void
   * Note : Update list of subStates which depends on the state selected
   */
  function updateFieldsChoices ()
  {
    // Only get the substate list from one of the two because they are the same anyways
    $subStatesList = $this->subStates[$this->attributesAccess['fdTasksNotificationsState']->getValue()] ?? [];

    // Update the available supann states
    $this->attributesAccess['fdTasksNotificationsSubState']->setChoices(array_keys($subStatesList), array_values($subStatesList));
    $this->attributesAccess['fdTasksNotificationsPostSubState']->setChoices(array_keys($subStatesList), array_values($subStatesList));
  }

  /**
   * @return array
   * Note: This method is required because setAttribute can contain one value STRING or multiple ARRAY but,
   * selectAttribute only accepts arrays.
   */
  protected function getAttributesListFromConf (): array
  {
    global $config;
    $result = [];

    if (!empty($config->current['TASKSNOTIFICATIONSCONFATTRIBUTES'])) {
      if (is_string($config->current['TASKSNOTIFICATIONSCONFATTRIBUTES'])) {
        $result[] = $config->current['TASKSNOTIFICATIONSCONFATTRIBUTES'];
      } else {
        $result = $config->current['TASKSNOTIFICATIONSCONFATTRIBUTES'];
      }
    }

    // Ensure 'None' is in the array
    if (!in_array('None', $result, TRUE)) {
      array_unshift($result, 'None'); // Add 'None' at the beginning
    }

    return $result;
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
    $recipients = $this->attributesAccess['fdTasksNotificationsRecipientsMembers']->getValue();
    // Call static function from tasks object in order to get a real list of members and related emails directly.
    $emailsList = mailTask::getEmailsFromSelectedDN('gosaMailAccount',
                                                     'mail',
                                                     tasks::extractMembersFromGroups($recipients)
    );

    // Fill the attribute with emails.
    $this->attributesAccess['fdTasksNotificationsListOfRecipientsMails']->setValue(array_values($emailsList));
  }

  /**
   * @return array
   * Note : Return the list of users DN having had an audit listed since the last exec of this task.
   */
  public function auditDnSinceLastActivation (array $dn): array
  {
    global $config;
    $result = [];

    if (empty($this->lastActivation)) {
      $unixLastActivation = strtotime(date("Y-m-d H:i:s"));
    } else { // Convert to unix timestamp
      $unixLastActivation = strtotime($this->lastActivation);
    }

    $ldap = $config->get_ldap_link();
    // Restrict search on the OU of audit specified in backend.
    $ldap->cd(get_ou('auditRDN') . $config->current['BASE']);

    // Create a long filter to enhance ldap searching pattern
    $ldapFilter = '';
    foreach ($dn as $monitoredMember) {
      $ldapFilter .= '(fdAuditObject=' . $monitoredMember . ')';
    }

    $ldap->search('(&(objectClass=fdAuditEvent)(|' . $ldapFilter . '))', ['fdAuditDateTime', 'fdAuditObject']);

    while ($attrs = $ldap->fetch()) {
      if (!empty($attrs['fdAuditDateTime'][0])) {

        // Convert generic time to DateTime object. (Generic time with Z are in UTC)
        $date = DateTime::createFromFormat('YmdHis.u\Z', $attrs['fdAuditDateTime'][0], new DateTimeZone('UTC'));
        // Convert to defined local timezone.
        $date->setTimezone(timezone::getDefaultTimeZone());
        $unixAuditTime = $date->getTimestamp(); // Change object dateTime to unix format.

        if ($unixAuditTime >= $unixLastActivation) {
          $result[$attrs['fdAuditObject'][0]][] = $attrs['dn'];
        }
      }
    }

    return $result;
  }

  /**
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  function generateSlaveTasks ()
  {
    $listOfDN = $this->attributesAccess['fdTasksNotificationsListOfNewAuditUsersDN']->getValue();
    // The attribute required to be search in createSlaveTasks
    $attributeType = 'fdTasksGranularDN';
    // Store the full associative array coming from the list of new audit user DN, containing all audit DN related to that user.
    $attrs['ref'] = $listOfDN;

    // Call the method from parent tasks object (first tab) to create sub-tasks. listOfDN keys contains the UID of the users.
    $this->parent->getBaseObject()->createSlaveTasks(array_keys($listOfDN), $attributeType, $attrs, 'Notifications');
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

    // Set the DN of all audit user DN since the last exec.
    $monitoredMembers = $this->attributesAccess['fdTasksNotificationsMonitoredMembers']->getValue();
    $this->attributesAccess['fdTasksNotificationsListOfNewAuditUsersDN']->setValue(
      $this->auditDnSinceLastActivation(tasks::extractMembersFromGroups($monitoredMembers))
    );

    if ($execTasks) {
      $this->generateSlaveTasks();
    }

    return parent::save();
  }
}
