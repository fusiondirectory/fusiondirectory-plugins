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

  // The main function : information about attributes
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

    // Load all configured resources, states, and their linked substates from LDAP
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
    // Ensure the second dropdown (State) also triggers an update for the third (Substatus)
    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[1]->setSubmitForm('updateFieldsChoices');
    $this->updateFieldsChoices();

    /* Import from supannRessourceEtat if needed */
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

    // Update 'Substatus' choices based on the selected 'State'
    $lines = $this->attributesAccess['supannRessourceEtatDate']->getValue();
    if (!is_array($lines)) {
        $lines = [];
    }

    // Get the substatus SelectAttribute (third element at index 2)
    $substatusAttribute = $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[2];

    // First, set a default empty choice
    $substatusAttribute->setChoices(
        [''], 
        [_('-- Select State or No Substates --')]
    );

    // Now update the substatus selections based on the current values
    foreach ($lines as $i => $line) {
        if (isset($line[1]) && !empty($line[1])) {
            $selectedStateName = $line[1];
            
            if (isset($this->configuredStatesWithSubstates[$selectedStateName]['substates'])) {
                $substates = $this->configuredStatesWithSubstates[$selectedStateName]['substates'];
                
                // For the selected state we'll now update or add available substates
                if (!empty($substates)) {
                    $substatusAttribute->addChoices(
                        array_keys($substates),
                        array_values($substates)
                    );
                }
            }
        }
    }
  }

  /**
   * Callback function for OrderedArrayAttribute to dynamically update substatus choices
   * 
   * @param array $values The array of values from the attribute
   * @param integer $key The current index/key being processed
   * @return array Updated values with proper choices for substatus
   */
  public function updateSubstatusChoices(array $values, int $key): array
  {
    // Get the selected state from the current row
    $selectedStateName = $values[$key][1] ?? null;
    
    // Set up substatus choices based on the selected state
    $substatusChoices = [];
    if ($selectedStateName && isset($this->configuredStatesWithSubstates[$selectedStateName]['substates'])) {
        $substatusChoices = $this->configuredStatesWithSubstates[$selectedStateName]['substates'];
    }
    
    if (empty($substatusChoices)) {
        $substatusChoices[''] = _('-- Select State or No Substates --');
    }
    
    // Update the choices for the substatus dropdown
    // Use direct access to update the select attribute for this specific row
    $substatusAttribute = $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[2];
    $substatusAttribute->setChoices(
        array_keys($substatusChoices),
        array_values($substatusChoices)
    );
    
    return $values;
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

  /**
   * @return void
   */
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

  /**
   * @param string $state
   * @return void
   */
  protected function processAccountState (string $state): void
  {
    // This should only be verified under non templating logic
    if (!$this->is_template) {
      if ($state === 'A') {
        // Unlock account
        userManagement::lockUser('unlock', $this->parent->getBaseObject()->userPassword, $this->dn);
      } elseif (in_array($state, ['S', 'I'], TRUE)) {
        // Lock account
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
        /* Lock all resources */
        $modify['supannRessourceEtatDate'][] = '{' . $label . '}S:SupannVerrouille:' . date('Ymd') . ':';
        $modify['supannRessourceEtat'][]     = '{' . $label . '}S:SupannVerrouille';
      } elseif ((($state == 'S') || ($state == 'I')) && ($mode == 'UNLOCK')) {
        /* Unlock account */
        $modify['supannRessourceEtatDate'][] = '{' . $label . '}A:SupannActif:' . date('Ymd') . ':';
        $modify['supannRessourceEtat'][]     = '{' . $label . '}A:SupannActif';
      } else {
        /* Do not touch other fields */
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
            
            // Process substates DNs from the state object
            if (!empty($stateEntry['fdSupannSubStateList'])) {
                $subStateDNs = is_array($stateEntry['fdSupannSubStateList']) ? $stateEntry['fdSupannSubStateList'] : [$stateEntry['fdSupannSubStateList']];
                
                foreach ($subStateDNs as $subStateDN) {
                    // Skip numeric 'count' entry that LDAP operations sometimes include
                    if (is_int($subStateDN) || $subStateDN === 'count') {
                        continue;
                    }
                    
                    $subStateAttributes = ['fdSupannSubStateName', 'fdSupannRessourceLabel'];
                    $subStateFilter = '(objectClass=fdSupannRessourceSubState)';
                    // Use cat() instead of search() with 'base' scope
                    if ($ldap->cat($subStateDN, $subStateAttributes, $subStateFilter)) {
                        $subStateLdapEntry = $ldap->fetch();
                        if ($subStateLdapEntry && isset($subStateLdapEntry['fdSupannSubStateName'][0])) {
                            $subStateNameKey = $subStateLdapEntry['fdSupannSubStateName'][0];
                            $subStateLabelValue = $subStateLdapEntry['fdSupannRessourceLabel'][0] ?? $subStateNameKey;
                            $linkedSubstates[$subStateNameKey] = $subStateLabelValue;
                        }
                    }
                }
            }
            
            $this->configuredStatesWithSubstates[$stateName] = [
                'label' => $stateLabel,
                'substates' => $linkedSubstates
            ];
        }
    }
  }
}
