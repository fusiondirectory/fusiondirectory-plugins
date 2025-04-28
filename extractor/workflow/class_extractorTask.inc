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

class extractorTask extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Extractor'),
      'plDescription'  => _('Tasks Extractor Object'),
      'plIcon'        => 'geticon.php?context=applications&icon=extractor-task&size=48',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdExtractorTasks'],
      'plFilter'       => '(objectClass=fdExtractorTasks)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => ['tasksMail', 'tasksLifeCycle', 'tasksNotifications', 'auditTasks', 'tasksArchive'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1' => [
        'name'  => _('Members to Extract'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Selected Members'),
            _('Users or Groups to extract data from.'),
            'fdExtractorTaskMembers',
            TRUE
          ),
          new HiddenAttribute(
            'fdExtractorTaskListOfDN'
          ),
        ],
      ],
      'sectionAttr' => [
        'name'  => _('Attributes to Extract'),
        'attrs' => [
          new SetAttribute(
            new SelectAttribute(
              _('Attributes'), _('Attributes to extract (leave empty for ALL)'),
              'fdExtractorTaskAttributes', TRUE
            ),
          ),
        ]
      ],
      'section2' => [
        'name'  => _('Extraction Settings'),
        'attrs' => [
          new SelectAttribute(
            _('Format'),
            _('Output format for extracted data'),
            'fdExtractorTaskFormat',
            TRUE,
            ['CSV'], // Array of choices/keys
            'CSV',   // Default value
            [_('CSV (Comma Separated Values)')], // Array of display values corresponding to choices
            ''      // ACL (empty string means use its own)
          ),
        ]
      ],
      'section3' => [
        'name'  => _('Email Notification'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Recipients'),
            _('Users or Groups to receive the extract by email.'),
            'fdExtractorRecipientsMembers',
            TRUE
          ),
          new HiddenArrayAttribute(
            'fdExtractorListOfRecipientsMails',
            FALSE,
            ''
          ),
          new MailAttribute(
            _('Sender email address'),
            _('Email address from which the extract will be sent'),
            'fdExtractorEmailSender',
            TRUE,
          ),
        ],
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    // Load available attributes from backend config
    $this->attributesAccess['fdExtractorTaskAttributes']->attribute->setChoices($this->getAttributesListFromConf());
  }

  /**
   * Load extractable attributes from backend config, default to ALL if none set.
   */
  protected function getAttributesListFromConf (): array
  {
    global $config;
    $result = [];

    if (!empty($config->current['EXTRACTORCONFATTRIBUTES'])) {
      if (is_string($config->current['EXTRACTORCONFATTRIBUTES'])) {
        $result[] = $config->current['EXTRACTORCONFATTRIBUTES'];
      } else {
        $result = $config->current['EXTRACTORCONFATTRIBUTES'];
      }
    }

    // Ensure 'ALL' is in the array
    if (!in_array('ALL', $result, TRUE)) {
      array_unshift($result, 'ALL');
    }

    return $result;
  }

  /**
   * Generate slave tasks for extraction
   */
  function generateSlaveTasks ()
  {
    $selectedMembers = $this->attributesAccess['fdExtractorTaskMembers']->getValue();
    $listOfDN = tasks::extractMembersFromGroups($selectedMembers);

    // Store the list of DNs for future use by the orchestrator
    $this->attributesAccess['fdExtractorTaskListOfDN']->setValue($listOfDN);

    // Define a single identifier for the bulk extraction task
    $bulkTaskIdentifier = 'bulkExtractorTask';

    // Call the method from parent tasks object (first tab) to create ONE sub-task for the orchestrator
    // The orchestrator will handle processing the DNs stored in fdExtractorTaskListOfDN.
    $this->parent->getBaseObject()->createSlaveTasks([$bulkTaskIdentifier], 'fdTasksGranularDN', NULL, 'extract');
  }

  /**
   * Resolve recipient emails
   */
  protected function resolveRecipientEmails (): void
  {
    $recipients = $this->attributesAccess['fdExtractorRecipientsMembers']->getValue();
    $emailsList = tasksMail::getEmailsFromSelectedDN(
        'gosaMailAccount',
        'mail',
        tasks::extractMembersFromGroups($recipients)
    );
    $this->attributesAccess['fdExtractorListOfRecipientsMails']->setValue(array_values($emailsList));
  }

  /**
   * @return bool
   */
  function update (): bool
  {
    parent::update();
    $this->resolveRecipientEmails();
    return TRUE;
  }

  /**
   * @return array
   */
  function save (): array
  {
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;
    if ($execTasks) {
        $this->generateSlaveTasks();
    }
    $this->resolveRecipientEmails();
    return parent::save();
  }
}
