<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2018-2019 FusionDirectory
  License: GNU GPL v2 or later
*/

class supannAccountStatus extends simplePlugin implements UserTabLockingAction
{
  protected $displayHeader = TRUE;

  /**
   * Stores all configured fdSupannRessource objects.
   * Format: ['ressourceName1' => 'RessourceLabel1', ...]
   * @var array
   */
  protected $configuredResources = [];

  /**
   * Stores all configured fdSupannRessourceState objects and their linked substates.
   * Format: [
   *   'stateName1' => [
   *     'label' => 'StateLabel1',
   *     'substates' => ['subStateName1' => 'SubStateLabel1', ...]
   *   ],
   *   ...
   * ]
   * @var array
   */
  protected $configuredStatesWithSubstates = [];

  static function plInfo (): array
  {
    global $config;
    if ($config->get_cfg_value('SupannShowAccountStatus', 'TRUE') !== 'TRUE') {
      return [];
    }
    return [
      'plShortName' => _('SupAnn status'),
      'plDescription' => _('SupAnn status management'),
      'plFilter' => '(supannRessourceEtatDate=*)',
      'plIcon' => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon' => 'geticon.php?context=applications&icon=supann-status&size=16',
      'plSelfModify' => TRUE,
      'plPriority' => 15,
      'plObjectClass' => [],
      'plObjectType' => ['user'],
      'plDepends' => ['supannAccount'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'status' => [
        'name' => _('Status'),
        'class' => ['fullwidth'],
        'attrs' => [
          new OrderedArrayAttribute(
            new CompositeAttribute(
              _('State of the account and associated resources'),
              'supannRessourceEtatDate',
              [
                new SelectAttribute(
                  _('Resource'), _('Which resource this state concerns'),
                  'supannRessourceEtatDate_label', TRUE,
                  ['COMPTE', 'MAIL'], '',
                  [_('Account'), _('Mail')]
                ),
                new SelectAttribute(
                  _('Status'), _('Active status'),
                  'supannRessourceEtatDate_status', TRUE,
                  ['A', 'I', 'S'], 'A',
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
                  NULL, NULL
                ),
                new DateAttribute(
                  _('End date'), _('Date this status will end'),
                  'supannRessourceEtatDate_end', FALSE,
                  'Ymd', '',
                  NULL, NULL
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

    $this->loadLdapConfigurations();

    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[0]->setChoices(
      array_keys($this->configuredResources),
      array_values($this->configuredResources)
    );

    $this->attributesAccess['supannRessourceEtatDate']->setLinearRendering(FALSE);
    $this->attributesAccess['supannRessourceEtatDate']->setRequired(TRUE);
    $this->attributesAccess['supannRessourceEtatDate']->setHeaders([
      _('Resource'),
      _('State'),    
      _('Substatus'),
      _('Begin'),
      _('End'),
      ''
    ]);
    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[1]->setSubmitForm('updateFieldsChoices');
    $this->updateFieldsChoices();

    if (!empty($this->supannRessourceEtat) && empty($this->supannRessourceEtatDate)) {
      $this->supannRessourceEtatDate = array_map(
        function ($supannRessourceEtat) {
          if (strpos($supannRessourceEtat, ':') === FALSE) {
            return $supannRessourceEtat . ':::';
          } else {
            return $supannRessourceEtat . '::';
          }
        },
        $this->supannRessourceEtat
      );
    }
  }

  /* Update choices of fields which depends on other fields values */
  function updateFieldsChoices()
  {
    // Populate the 'State' dropdown (second element)
    $stateChoices = [];
    foreach ($this->configuredStatesWithSubstates as $stateName => $stateDetails) {
      $stateChoices[$stateName] = $stateDetails['label'];
    }
    if (empty($stateChoices)) {
      $stateChoices[''] = _('-- No States Defined --');
    }
    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[1]->setChoices(
      array_keys($stateChoices),
      array_values($stateChoices)
    );

    // Get the current lines (rows)
    $lines = $this->attributesAccess['supannRessourceEtatDate']->getValue();
    if (!is_array($lines)) {
      $lines = [];
    }

    // Get the substatus SelectAttribute (third element at index 2)
    $substatusAttribute = $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[2];

    // Default choice
    $choices = ['' => _('-- Select State or No Substates --')];

    // If editing a row, set choices for that row's state
    if (count($lines) == 1) {
      $line = $lines[0];
      $selectedStateName = $line[1] ?? '';
      if ($selectedStateName && isset($this->configuredStatesWithSubstates[$selectedStateName]['substates'])) {
        $substates = $this->configuredStatesWithSubstates[$selectedStateName]['substates'];
        if (!empty($substates)) {
          $choices = $substates;
        }
      }
    }
    $substatusAttribute->setChoices(array_keys($choices), array_values($choices));
  }

  protected function prepare_save (): array
  {
    /* Copy active states over to supannRessourceEtat */
    $this->supannRessourceEtat = array_map(
      function ($supannRessourceEtatDate) {
        list($labelstate, $substate) = explode(':', $supannRessourceEtatDate);
        if (empty($substate)) {
          return $labelstate;
        } else {
          return $labelstate . ':' . $substate;
        }
      },
      $this->supannRessourceEtatDate
    );
    return parent::prepare_save();
  }

  function check (): array
  {
    $errors = parent::check();
    $resources = [];
    foreach ($this->supannRessourceEtatDate as $line) {
      $resource = preg_replace('/^\{([^}]+)\}.*$/', '\1', $line);
      if (in_array($resource, $resources)) {
        $errors[] = new SimplePluginCheckError(
          $this->attributesAccess['supannRessourceEtatDate'],
          htmlescape(sprintf(_('Resource "%s" is present more than once in supannRessourceEtatDate'), $resource))
        );
      } else {
        $resources[] = $resource;
      }
    }
    return $errors;
  }

  protected function post_save ()
  {
    foreach ($this->supannRessourceEtatDate as $line) {
      list($label, $state, $substate, $start, $end) = $this->attributesAccess['supannRessourceEtatDate']->attribute->readValues($line);
      if ($label === 'COMPTE') {
        $this->processAccountState($state);
      }
    }
    parent::post_save();
  }

  protected function processAccountState (string $state): void
  {
    if (!$this->is_template) {
      if ($state === 'A') {
        userManagement::lockUser('unlock', $this->parent->getBaseObject()->userPassword, $this->dn);
      } elseif (in_array($state, ['S', 'I'], TRUE)) {
        userManagement::lockUser('lock', $this->parent->getBaseObject()->userPassword, $this->dn, FALSE);
      }
    }
  }

  public function fillLockingLDAPAttrs (string $mode, array &$modify)
  {
    if (empty($this->supannRessourceEtatDate)) {
      return;
    }
    $modify['supannRessourceEtatDate'] = [];
    $modify['supannRessourceEtat']     = [];
    foreach ($this->supannRessourceEtatDate as $line) {
      list ($label, $state, $substate, $start, $end) = $this->attributesAccess['supannRessourceEtatDate']->attribute->readValues($line);
      if (($state == 'A') && ($mode == 'LOCK')) {
        $modify['supannRessourceEtatDate'][] = '{' . $label . '}S:SupannVerrouille:' . date('Ymd') . ':';
        $modify['supannRessourceEtat'][]     = '{' . $label . '}S:SupannVerrouille';
      } elseif ((($state == 'S') || ($state == 'I')) && ($mode == 'UNLOCK')) {
        $modify['supannRessourceEtatDate'][] = '{' . $label . '}A:SupannActif:' . date('Ymd') . ':';
        $modify['supannRessourceEtat'][]     = '{' . $label . '}A:SupannActif';
      } else {
        $modify['supannRessourceEtatDate'][] = $line;
        $modify['supannRessourceEtat'][]     = '{' . $label . '}' . $state . (empty($substate) ? '' : ':' . $substate);
      }
    }
  }

  /**
   * Loads all fdSupannRessource, fdSupannRessourceState, and linked fdSupannRessourceSubState objects from LDAP.
   * Populates $this->configuredResources and $this->configuredStatesWithSubstates.
   */
  protected function loadLdapConfigurations(): void
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $this->configuredResources = [];
    $this->configuredStatesWithSubstates = [];

    // 1. Fetch Resources
    $ressourcesOu = get_ou('SupannRessourceRDN') . $config->current['BASE'];
    $ressourceAttributes = ['fdSupannRessourceName', 'fdSupannRessourceLabel'];
    if ($ldap->search('(objectClass=fdSupannRessource)', $ressourceAttributes, $ressourcesOu)) {
      while ($ressourceEntry = $ldap->fetch()) {
        if ($ressourceEntry && isset($ressourceEntry['fdSupannRessourceName'][0])) {
          $name = $ressourceEntry['fdSupannRessourceName'][0];
          $label = $ressourceEntry['fdSupannRessourceLabel'][0] ?? $name;
          $this->configuredResources[$name] = $label;
        }
      }
    }

    // 2. Fetch States and their linked Substates
    $statesOu = get_ou('SupannStateRDN') . $config->current['BASE'];
    $stateAttributes = ['fdSupannStateName', 'fdSupannRessourceLabel', 'fdSupannSubStateList'];
    if ($ldap->search('(objectClass=fdSupannRessourceState)', $stateAttributes, $statesOu)) {
      while ($stateEntry = $ldap->fetch()) {
        if (!$stateEntry || !isset($stateEntry['fdSupannStateName'][0])) {
          continue;
        }
        $stateName = $stateEntry['fdSupannStateName'][0];
        $stateLabel = $stateEntry['fdSupannRessourceLabel'][0] ?? $stateName;
        $linkedSubstates = [];

        if (isset($stateEntry['fdSupannSubStateList'])) {
          // Handle different possible return formats from LDAP
          if (isset($stateEntry['fdSupannSubStateList']['count'])) {
            for ($i = 0; $i < $stateEntry['fdSupannSubStateList']['count']; $i++) {
              $this->fetchSubstate($ldap, $stateEntry['fdSupannSubStateList'][$i], $linkedSubstates);
            }
          } else if (is_array($stateEntry['fdSupannSubStateList'])) {
            foreach ($stateEntry['fdSupannSubStateList'] as $key => $subStateDN) {
              if ($key !== 'count' && !is_numeric($key)) {
                $this->fetchSubstate($ldap, $subStateDN, $linkedSubstates);
              }
            }
          } else {
            $this->fetchSubstate($ldap, $stateEntry['fdSupannSubStateList'], $linkedSubstates);
          }
        }

        $this->configuredStatesWithSubstates[$stateName] = [
          'label' => $stateLabel,
          'substates' => $linkedSubstates
        ];
      }
    }
  }

  /**
   * Helper method to fetch a substate by DN and add it to the linkedSubstates array
   *
   * @param LDAP $ldap The LDAP connection
   * @param string $subStateDN The DN of the substate to fetch
   * @param array &$linkedSubstates Reference to the array of linked substates to populate
   */
  private function fetchSubstate($ldap, string $subStateDN, array &$linkedSubstates): void
  {
    if (empty($subStateDN) || strpos($subStateDN, '=') === false) {
      return;
    }
    $subStateAttributes = ['fdSupannSubStateName', 'fdSupannRessourceLabel'];
    if ($ldap->cat($subStateDN, $subStateAttributes)) {
      $subStateLdapEntry = $ldap->fetch();
      if ($subStateLdapEntry && isset($subStateLdapEntry['fdSupannSubStateName'][0])) {
        $subStateNameKey = $subStateLdapEntry['fdSupannSubStateName'][0];
        $subStateLabelValue = isset($subStateLdapEntry['fdSupannRessourceLabel'][0]) ?
          $subStateLdapEntry['fdSupannRessourceLabel'][0] : $subStateNameKey;
        $linkedSubstates[$subStateNameKey] = $subStateLabelValue;
      }
    }
  }
}
