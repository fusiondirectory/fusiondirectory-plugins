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
          new HiddenArrayAttribute('fdTasksNotificationsListOfNewAuditUsersDN', FALSE, ''),
          // Following attribute contain the list of members emails that should receive a notification.
          new HiddenArrayAttribute('fdTasksNotificationsListOfRecipientsMails', FALSE, ''),
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
          new UsersGroupsRolesAttribute(
            _('Monitored Members'), _('Users or Groups requiring monitoring.'),
            'fdTasksNotificationsMonitoredMembers', TRUE
          ),
        ],
      ],
    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    // Set the list of available attributes to follow in the set selection
    $this->attributesAccess['fdTasksNotificationsAttributes']->attribute->setChoices($this->getAttributesListFromConf());

    // This avoids keeping obsolete AuditUsersDN once subtasks have been generated.
    $this->attributesAccess['fdTasksNotificationsListOfNewAuditUsersDN']->setInLdap(FALSE);
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

    return $result;
  }

  /**
   * Retrieve all the DNs from groups or members
   */
  protected function getListOfDN (): void
  {
    // Get the members or groups selected
    $recipients = $this->attributesAccess['fdTasksNotificationsRecipientsMembers']->getValue();
    // Call static function from tasks object in order to get a real list of members and related emails directly.
    $emailsList = tasksMail::getEmailsFromSelectedDN('gosaMailAccount',
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
  public function auditDnSinceLastExec (array $dn): array
  {
    global $config;
    $result = [];

    $lastExec = $this->parent->getBaseObject()->fdTasksLastExec ?? NULL;
    // Simply convert to unix timestamp.
//    if (!empty($lastExec)) {
//      $unixLastExec = strtotime($lastExec);
//    } else { // In case lastExec is indeed NULL, we set today time in order to avoid being notified on the pasts modifications.
//      $unixLastExec = time();
//    }

    if (empty($lastExec)) {
      $unixLastExec = time();
    } else { // Convert to unix timestamp
      $unixLastExec = strtotime($lastExec);
    }

    $ldap = $config->get_ldap_link();
    // Restrict search on the OU of audit specified in backend.
    $ldap->cd(get_ou('auditRDN') . $config->current['BASE']);

    foreach ($dn as $monitoredMember) {
      $ldap->search('(&(objectClass=fdAuditEvent)(fdAuditObject=' . $monitoredMember . '))', ['fdAuditDateTime']);
      while ($attrs = $ldap->fetch()) {
        if (!empty($attrs['fdAuditDateTime'][0])) {

          // Extract the part before the dot to remove milliseconds
//        $auditTime = explode('.', $attrs['fdAuditDateTime'][0]);
          // Convert to unix format without forgetting the Z (coming from ISO 8601 standard, Z means UTC).
//        $unixAuditTime = strtotime($auditTime[0].'Z');

//        $auditTime = DateTime::createFromFormat('YmdHis.u\Z', $attrs['fdAuditDateTime'][0]);
//        $systemTimeZone = new DateTimeZone('America/New_York');
//        $auditTime->setTimezone($systemTimeZone);
//        $unixAuditTime = $dateTime->format('U');
//        $unixTimestamp = $dateTime->format('U');


          $date          = DateTime::createFromFormat('YmdHis.u\Z', $attrs['fdAuditDateTime'][0]);
          $unixAuditTime = $date->getTimestamp();


//        echo "Time lastExec: " . $lastExec->format('Y-m-d H:i:s'). PHP_EOL;
//        echo "Time auditTime: " . $auditTime->format('Y-m-d H:i:s') . PHP_EOL;
          echo "Time lastExec: " . date("Y-m-d H:i:s", $unixLastExec) . PHP_EOL;
          echo "Time auditTime: " . date("Y-m-d H:i:s", $unixAuditTime) . PHP_EOL;

          // Verification of the lastExec time and audit time.
//        if ($unixAuditTime >= $unixLastExec) {
//          $result[] = $monitoredMember;
//        }
          if ($unixAuditTime >= $unixLastExec) {
            $result[] = $monitoredMember;
          }
        }

      }
    }

    // Only return once the DN
    print_r($result);
    return array_unique($result);
  }

  /**
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  public
  function generateSlaveTasks ()
  {
    $listOfDN = $this->attributesAccess['fdTasksNotificationsListOfNewAuditUsersDN']->getValue();
    // The attribute required to be search in createSlaveTasks
    $attributeType = 'fdTasksGranularDN';

    // Call the method from parent tasks object (first tab) to create sub-tasks.
    $this->parent->getBaseObject()->createSlaveTasks($listOfDN, $attributeType);
  }

  function update (): bool
  {
    parent::update();

    // Take the list of DN based on the groups or members list selected and filled the related attributes
    $this->getListOfDN();

    return TRUE;
  }

 function shouldSave (): bool
 {
   // Set the attributes before entering within a save methodology. lastExec is already changed in main tasks if below
   // block is set in save().
   $monitoredMembers = $this->attributesAccess['fdTasksNotificationsMonitoredMembers']->getValue();
   $this->attributesAccess['fdTasksNotificationsListOfNewAuditUsersDN']->setValue(
     $this->auditDnSinceLastExec(tasks::extractMembersFromGroups($monitoredMembers))
   );

   return parent::shouldSave();
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
