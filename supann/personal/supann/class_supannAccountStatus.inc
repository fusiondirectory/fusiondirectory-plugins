<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2018-2019 FusionDirectory

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

class supannAccountStatus extends simplePlugin implements UserTabLockingAction
{
  protected $displayHeader = TRUE;

  protected $substatus;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn status'),
      'plDescription' => _('SupAnn status management'),
      'plFilter'      => '(supannRessourceEtatDate=*)',
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann&size=16',
      'plSelfModify'  => FALSE,
      'plPriority'    => 15,
      'plObjectClass' => [],
      'plObjectType'  => ['user'],
      'plDepends'     => ['supannAccount'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'status' => [
        'name'  => _('Status'),
        'attrs' => [
          new OrderedArrayAttribute(
            new CompositeAttribute(
              _('State of the account and associated resources'),
              'supannRessourceEtatDate',
              [
                new SelectAttribute(
                  _('Resource'), _('Which resource this state concerns'),
                  'supannRessourceEtatDate_label', TRUE,
                  ['COMPTE','MAIL'], '',
                  [_('Account'), _('Mail')]
                ),
                new SelectAttribute(
                  _('Status'), _('Active status'),
                  'supannRessourceEtatDate_status', TRUE,
                  ['A','I','S'], 'A',
                  [_('Active'), _('Inactive'), _('Suspended')]
                ),
                new SelectAttribute(
                  _('Substatus'), _('Substatus'),
                  'supannRessourceEtatDate_substatus', FALSE,
                  []
                ),
                new DateAttribute(
                  _('Start date'), _('Date this status started'),
                  'supannRessourceEtatDate_start', FALSE,
                  'Ymd', '',
                  NULL, 'now'
                ),
                new DateAttribute(
                  _('End date'), _('Date this status will end'),
                  'supannRessourceEtatDate_end', FALSE,
                  'Ymd', '',
                  'now', NULL
                ),
              ],
              '/^\{([^}]+)\}([^:]+):([^:]*):([^:]*):([^:]*)$/',
              '{%s}%s:%s:%s:%s',
              '',
              ''
            ),
            FALSE,
            [],
            TRUE
          ),
          new HiddenArrayAttribute('supannRessourceEtat', FALSE, []),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->substatus = static::getConfiguredSubstates();

    $this->attributesAccess['supannRessourceEtatDate']->setLinearRendering(FALSE);
    $this->attributesAccess['supannRessourceEtatDate']->setRequired(TRUE);
    $this->attributesAccess['supannRessourceEtatDate']->setHeaders([
      _('Resource'),
      _('Status'),
      _('Substatus'),
      _('Begin'),
      _('End'),
      ''
    ]);
    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[1]->setSubmitForm('updateFieldsChoices');
    $this->updateFieldsChoices();

    /* Import from supannRessourceEtat if needed */
    if (!empty($this->supannRessourceEtat) && empty($this->supannRessourceEtatDate)) {
      $this->supannRessourceEtatDate = array_map(
        function ($supannRessourceEtat)
        {
          if (strpos($supannRessourceEtat, ':') === FALSE) {
            return $supannRessourceEtat.':::';
          } else {
            return $supannRessourceEtat.'::';
          }
        },
        $this->supannRessourceEtat
      );
    }
  }

  /* Update choices of fields which depends on other fields values */
  function updateFieldsChoices ()
  {
    $substatusChoices = $this->substatus[$this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[1]->getValue()] ?? [];
    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[2]->setChoices(
      array_keys($substatusChoices),
      array_values($substatusChoices)
    );
  }

  protected function prepare_save (): array
  {
    /* Copy active states over to supannRessourceEtat */
    $this->supannRessourceEtat = array_map(
      function ($supannRessourceEtatDate)
      {
        list($labelstate,$substate) = explode(':', $supannRessourceEtatDate);
        if (empty($substate)) {
          return $labelstate;
        } else {
          return $labelstate.':'.$substate;
        }
      },
      $this->supannRessourceEtatDate
    );
    return parent::prepare_save();
  }

  function check (): array
  {
    $errors = parent::check();

    $labels = [];
    foreach ($this->supannRessourceEtatDate as $line) {
      $label = preg_replace('/^\{([^}]+)\}.*$/', '\1',  $line);
      if (in_array($label, $labels)) {
        $errors[] = sprintf(_('Label "%s" is present more than once in supannRessourceEtatDate'), $label);
      } else {
        $labels[] = $label;
      }
    }

    return $errors;
  }

  protected function post_save ()
  {
    foreach ($this->supannRessourceEtatDate as $line) {
      list ($label, $state, $substate, $start, $end) = $this->attributesAccess['supannRessourceEtatDate']->readValues($line);
      if ($label == 'COMPTE') {
        if ($state == 'A') {
          /* Unlock account */
          userManagement::lockUser('unlock', $this->parent->getBaseObject()->userPassword, $this->dn);
        } elseif (($state == 'S') || ($state == 'I')) {
          /* Lock account */
          userManagement::lockUser('lock', $this->parent->getBaseObject()->userPassword, $this->dn);
        }
        break;
      }
    }
    parent::post_save();
  }

  public function fillLockingLDAPAttrs (string $mode, array &$modify)
  {
    if (empty($this->supannRessourceEtatDate)) {
      return;
    }
    $modify['supannRessourceEtatDate']  = [];
    $modify['supannRessourceEtat']      = [];
    foreach ($this->supannRessourceEtatDate as $line) {
      list ($label, $state, $substate, $start, $end) = $this->attributesAccess['supannRessourceEtatDate']->readValues($line);
      if (($label == 'COMPTE') && ($state == 'A') && ($mode == 'LOCK')) {
        /* Lock account */
        $modify['supannRessourceEtatDate'][]  = '{'.$label.'}S:SupannVerrouille:'.date('Ymd').':';
        $modify['supannRessourceEtat'][]      = '{'.$label.'}S:SupannVerrouille';
      } elseif (($label == 'COMPTE') && (($state == 'S') || ($state == 'I')) && ($mode == 'UNLOCK')) {
        /* Unlock account */
        $modify['supannRessourceEtatDate'][]  = '{'.$label.'}A:SupannActif:'.date('Ymd').':';
        $modify['supannRessourceEtat'][]      = '{'.$label.'}A:SupannActif';
      } else {
        /* Do not touch other fields */
        $modify['supannRessourceEtatDate'][]  = $line;
        $modify['supannRessourceEtat'][]      = '{'.$label.'}'.$state.(empty($substate) ? '' : ':'.$substate);
      }
    }
  }

  static function getOfficialSubstates (): array
  {
    return [
      'A' => [
        'SupannAnticipe'  => _('Anticipated'),
        'SupannActif'     => _('Active'),
        'SupannSursis'    => _('Extension'),
      ],
      'I' => [
        'SupannPrecree'       => _('Precreated'),
        'SupannCree'          => _('Created'),
        'SupannExpire'        => _('Expired'),
        'SupannInactif'       => _('Inactive'),
        'SupannSupprDonnees'  => _('Data deletion'),
        'SupannSupprCompte'   => _('Account deletion'),
      ],
      'S' => [
        'SupannVerrouille'          => _('Locked'),
        'SupannVerrouAdministratif' => _('Administrative lock'),
        'SupannVerrouTechnique'     => _('Technical lock'),
      ],
    ];
  }

  static function getConfiguredSubstates (): array
  {
    global $config;

    $officialSubstates = static::getOfficialSubstates();

    $substates = [
      'A' => [],
      'I' => [],
      'S' => [],
    ];

    $labels = [];

    foreach ($config->get_cfg_value('SupannRessourceSubStatesLabels', []) as $line) {
      list($substate,$label) = explode(':', $line, 2);
      $labels[$substate] = $label;
    }

    foreach ($config->get_cfg_value('SupannRessourceSubStates', []) as $line) {
      list(,$state,$substate) = explode(':', $line, 3);
      if (isset($officialSubstates[$state][$substate])) {
        $substates[$state][$substate] = $officialSubstates[$state][$substate];
      } elseif (isset($labels[$substate])) {
        $substates[$state][$substate] = $labels[$substate];
      } else {
        $substates[$state][$substate] = $substate;
      }
    }

    return $substates;
  }
}
