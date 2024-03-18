<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2023 FusionDirectory

  Author : Thibault Paul M Dockx

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class aidaFreedivingAccount extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('AIDA-Freediving'),
      'plDescription'  => _('AIDA-Freediving Plugin'),
      'plObjectType'   => ['user'],
      'plObjectClass'  => ['fdAidaFreediving'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'aidaAccountInformation' => [
        'name'  => _('Student information'),
        'attrs' => [
          new SelectAttribute(
            _('Student current level'), _('Student current level'),
            'fdAidaStudentLevel', FALSE, ['UNCERTIFIED', 'AIDA1', 'AIDA2', 'AIDA3', 'AIDA4'], 'UNCERTIFIED'
          ),
          new BooleanAttribute(
            _('Certified'), _('Student is certified current level'), 'fdAida1StudentCertified', FALSE, FALSE
          ),
          new BooleanAttribute(
            _('Certified'), _('Student is certified current level'), 'fdAida2StudentCertified', FALSE, FALSE
          ),
          new BooleanAttribute(
            _('Certified'), _('Student is certified current level'), 'fdAida3StudentCertified', FALSE, FALSE
          ),
          new BooleanAttribute(
            _('Certified'), _('Student is certified current level'), 'fdAida4StudentCertified', FALSE, FALSE
          ),
          new BooleanAttribute(
            _('Exam'), _('Student for current level is completed'), 'fdAida2StudentExam', FALSE, FALSE
          ),
          new BooleanAttribute(
            _('Exam'), _('Student for current level is completed'), 'fdAida3StudentExam', FALSE, FALSE
          ),
          new BooleanAttribute(
            _('Exam'), _('Student for current level is completed'), 'fdAida4StudentExam', FALSE, FALSE
          ),
          new BooleanAttribute(
            _('Medical form'), _('Medical form completed'), 'fdAidaStudentMedicalFormCompleted', FALSE, FALSE
          ),
          new FileDownloadAttribute(
            _('Medical document'), _('Import medical document'),
            'fdAidaStudentMedicalDocument', FALSE, '', TRUE
          ),
          new BooleanAttribute(
            _('Liability form'), _('Liability form completed'), 'fdAidaStudentLiabilityFormCompleted', FALSE, FALSE
          ),
          new FileDownloadAttribute(
            _('Liability release document'), _('Import liability release document'),
            'fdAidaStudentLiabilityDocument', FALSE, '', TRUE
          ),
          // Somehow the format of the DateAttribute does not change the selection. Only Ymd format is displayed.
          new EpochDaysDateAttribute(
            _('Start date'), _('Starting date of the course/coaching session'),
            'fdAidaStudentStartingDate', FALSE, ''

          ),
          new EpochDaysDateAttribute(
            _('Next training date'), _('Next training date of the course/coaching session'),
            'fdAidaStudentNextDate', FALSE, ''

          ),
          new EpochDaysDateAttribute(
            _('End date'), _('End date of the course/coaching'),
            'fdAidaStudentEndDate', FALSE, ''

          ),
        ]

      ]
    ],
      'aidaAccountInformationCompletion' => [
    'name'  => _('Student Completion Form'),
    'attrs' => [
      # Attributes related to AIDA 1
      new BooleanAttribute(
        _('Breath Holding'), _('Breath Holding'),
        'fdAida1CompletionBreathHold', FALSE
      ),
      new BooleanAttribute(
        _('Water session'), _('Water session'),
        'fdAida1CompletionWaterSession', FALSE
      ),
      # Attributes related to AIDA 2 - Confined water

      new BooleanAttribute(
        _('Swim 200'), _('Swim 200m or 300m with snorkel'),
        'fdAida2CompletionSwimming', FALSE
      ),
      new BooleanAttribute(
        _('Relaxation'), _('Demonstrate proper relaxation'),
        'fdAida2CompletionRelaxation', FALSE
      ),
      new BooleanAttribute(
        _('Breathing'), _('Demonstration proper breathing technics'),
        'fdAida2CompletionBreathing', FALSE
      ),
      new BooleanAttribute(
        _('STA Buddying'), _('STA Buddying'),
        'fdAida2CompletionSTABuddying', FALSE
      ),
      new BooleanAttribute(
        _('STA Timer'), _('STA Timer min 2:00'),
        'fdAida2CompletionSTATimer', FALSE
      ),
      new BooleanAttribute(
        _('STA Rescue'), _('STA Rescue - safety exercise'),
        'fdAida2CompletionSTARescue', FALSE
      ),
      new BooleanAttribute(
        _('DYN Buddying'), _('DYN Buddying'),
        'fdAida2CompletionDYNBuddying', FALSE
      ),
      new BooleanAttribute(
        _('DYN Finning'), _('DYN Finning technics'),
        'fdAida2CompletionDYNFinning', FALSE
      ),
      new BooleanAttribute(
        _('DYN Length'), _('DYN performed equal or over 40m'),
        'fdAida2CompletionDYNLength', FALSE
      ),
      new BooleanAttribute(
        _('DYN Rescue'), _('DYN Rescue exercise'),
        'fdAida2CompletionDYNRescue', FALSE
      ),
      # Attributes related to AIDA 2 - Open water
      new BooleanAttribute(
        _('FIM'), _('FIM'),
        'fdAida2CompletionFIM', FALSE
      ),
      new BooleanAttribute(
        _('Lanyard'), _('Lanyard usage'),
        'fdAida2CompletionLanyard', FALSE
      ),
      new BooleanAttribute(
        _('Controlled Turn'), _('Controlled turn technic'),
        'fdAida2CompletionControlledTurn', FALSE
      ),
      new BooleanAttribute(
        _('Equalization'), _('Efficient Frenzel or Valsalva'),
        'fdAida2CompletionEqualization', FALSE
      ),
      new BooleanAttribute(
        _('Duck Dive'), _('Duck dive'),
        'fdAida2CompletionDuckDive', FALSE
      ),
      new BooleanAttribute(
        _('CWT Body positioning'), _('Student position in CWT'),
        'fdAida2CompletionCWTPositioning', FALSE
      ),
      new BooleanAttribute(
        _('CWT Line orientation'), _('Student position related to the line in CWT'),
        'fdAida2CompletionLineOrientation', FALSE
      ),
      new BooleanAttribute(
        _('CWT Finning'), _('CWT Finning'),
        'fdAida2CompletionCWTFinning', FALSE
      ),
      new BooleanAttribute(
        _('CWT Buddying'), _('CWT Buddying'),
        'fdAida2CompletionCWTBuddying', FALSE
      ),
      new BooleanAttribute(
        _('CWT Depth'), _('Student CWT Depth (max 20m)'),
        'fdAida2CompletionCWTDepth', FALSE
      ),
      new BooleanAttribute(
        _('Surface Rescue'), _('Surface rescue - safety exercise'),
        'fdAida2CompletionSurfaceRescue', FALSE
      ),
      new BooleanAttribute(
        _('Depth Rescue'), _('Depth rescue between 5 - 10m deep'),
        'fdAida2CompletionDepthRescue', FALSE
      ),
    ]
  ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    // Management of visibility of some attributes
    $this->setAllAttributesInvisible();
    $this->showProperAttributesForStudentLevel();

    //When changing the level, refresh page by sending post and calling method name sent as string (see class_Attributes).
    $this->attributesAccess['fdAidaStudentLevel']->setSubmitForm('showProperAttributesForStudentLevel');
  }

  // Function allowing to hide or show attributes related to the level of the student
  public function showProperAttributesForStudentLevel (): void
  {
    // Get the student current level
    $aidaStudentLevel = $this->attributesAccess['fdAidaStudentLevel']->getValue();

    //Set all attribute invisible before rendering them visible (Important during the refresh form of student level).
    $this->setAllAttributesInvisible();

    switch ($aidaStudentLevel) {
      case 'AIDA1' :
        $this->setRequiredAidaAttributesVisible('Aida1');
        break;

      case 'AIDA2' :
        $this->setRequiredAidaAttributesVisible('Aida2');
        break;

      case 'AIDA3' :
        $this->setRequiredAidaAttributesVisible('Aida3');
        break;

      case 'AIDA4' :
        $this->setRequiredAidaAttributesVisible('Aida4');
        break;

      default :
        $this->setAllAttributesInvisible();
        break;
    }
  }

  // Simply set all attributes invisible
  public function setAllAttributesInvisible (): void
  {
    foreach ($this->attributesAccess as $attrObject) {
      $attrObject->setVisible(FALSE);
    }
    // Set mandatory attributes visible
    $this->attributesAccess['fdAidaStudentLevel']->setVisible(TRUE);
    $this->attributesAccess['fdAidaStudentMedicalFormCompleted']->setVisible(TRUE);
    $this->attributesAccess['fdAidaStudentLiabilityFormCompleted']->setVisible(TRUE);
    $this->attributesAccess['fdAidaStudentStartingDate']->setVisible(TRUE);
    $this->attributesAccess['fdAidaStudentNextDate']->setVisible(TRUE);
    $this->attributesAccess['fdAidaStudentEndDate']->setVisible(TRUE);
    $this->attributesAccess['fdAidaStudentMedicalDocument']->setVisible(TRUE);
    $this->attributesAccess['fdAidaStudentLiabilityDocument']->setVisible(TRUE);
  }

  // Simply set all required AIDA level attributes visible
  // The needle can only contain Aida1, Aida2, Aida3, Aida4 - per standard and convention.
  public function setRequiredAidaAttributesVisible (string $needle): void
  {
    foreach ($this->attributesAccess as $aidaObjectsKeys => $aidaObjectsValues) {
      // Search for the needle 'aida2' within the key array of attribute table
      if (strpos($aidaObjectsKeys, $needle)) {
        $aidaObjectsValues->setVisible(TRUE);
      }
    }
  }

  protected function shouldSave (): bool
  {
    return parent::shouldSave();
  }

  /*
   * Save method, by interface rule, must remain public
   */
  public function save (): array
  {
    $errors = parent::save();
    return $errors;
  }
}
