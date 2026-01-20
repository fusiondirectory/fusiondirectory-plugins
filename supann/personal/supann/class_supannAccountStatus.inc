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
    global $config;

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
                  'supannRessourceEtatDate_label', TRUE, [],
                  '', NULL, '', $config->get_cfg_value('SupannRessourceEtatDateIntegrity', 'TRUE')
                ),
                new SelectAttribute(
                  _('Status'), _('Active status'),
                  'supannRessourceEtatDate_status', TRUE, [],
                  '', NULL, '', $config->get_cfg_value('SupannRessourceEtatDateIntegrity', 'TRUE')
                ),
                new SelectAttribute(
                  _('Substatus'), _('Substatus'),
                  'supannRessourceEtatDate_substatus', FALSE,
                  [], '', NULL, '', $config->get_cfg_value('SupannRessourceEtatDateIntegrity', 'TRUE')
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
    global $config, $ui;

    parent::__construct($dn, $object, $parent, $mainTab);

    /* Get substates from LDAP */
    $this->substatus = static::getConfiguredSubstates();

    // Get resources from LDAP */
    $resources = static::getResources();

    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[0]->setChoices(
      array_keys($resources),
      array_values($resources)
    );

    /* Get states from LDAP */
    $states = static::getStates();

    /* Set choices for states */
    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[1]->setChoices(
      array_keys($states),
      array_values($states)
    );

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
   * The concept is to allow the trigger of password locking when {account} is set to lock or unlock.
   * The same mechanism is present for mail allowing to lock the mail account within mail tab for linked web service.
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
   * Note : The concept of locking account is based on the modification of password hash.
   * The passwordMethod class will not trigger fillLockingLDAPAttrs methods of each plugin to lock everything.
   * It only locked the account access by password modification - fillLockingLDAPAttrs is only available via general lock.
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
    global $config;
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
        $delayDays = $config->get_cfg_value('SupannLockingDays', 365);
        $modify['supannRessourceEtatDate'][] = '{' . $label . '}A:SupannActif:' . date('Ymd') . ':' . date('Ymd', strtotime("+$delayDays days"));
        $modify['supannRessourceEtat'][]     = '{' . $label . '}A:SupannActif';
      } else {
        /* Do not touch other fields */
        $modify['supannRessourceEtatDate'][] = $line;
        $modify['supannRessourceEtat'][]     = '{' . $label . '}' . $state . (empty($substate) ? '' : ':' . $substate);
      }
    }
  }

  static function getResources (): array
  {
    global $config, $ui;

    /* LDAP object */
    $ldap = $config->get_ldap_link();
    // FIXME when testing it felt in base, it must be in an OU normally
    $ldap->cd($config->current['BASE']);

    /* Get resources from LDAP */
    $resources = [];
    $ldap->search('(objectClass=fdSupannRessource)', ['fdSupannRessourceName','fdSupannRessourceLabel'], 'subtree');
    while ($attrs = $ldap->fetch()) {
      if (!preg_match('/r/', $ui->get_permissions($attrs['dn'], 'supannRessource/supannRessource', 'fdSupannRessourceName'))) {
        continue;
      }
      if (!preg_match('/r/', $ui->get_permissions($attrs['dn'], 'supannRessource/supannRessource', 'fdSupannRessourceLabel'))) {
        continue;
      }
      $resources += [$attrs['fdSupannRessourceName'][0] => $attrs['fdSupannRessourceLabel'][0]];
    }

    /* Value if no ressources are found */
    if (count($resources) < 1) {
      $resources = ['' => '-- No ressource found --'];
    }

    /* Sort */
    asort($resources);

    return $resources;
  }

  static function getStates (): array
  {
    global $config, $ui;

    /* LDAP object */
    $ldap = $config->get_ldap_link();
    // FIXME when testing it felt in base, it must be in an OU normally
    $ldap->cd($config->current['BASE']);

    /* Get resources from LDAP */
    $states = [];
    // FIXME wrong attribut for label fdSupannRessourceLabel instead of fdSupannStateLabel
    $ldap->search('(objectClass=fdSupannRessourceState)', ['fdSupannStateName','fdSupannRessourceLabel'], 'subtree');
    while ($attrs = $ldap->fetch()) {
      if (!preg_match('/r/', $ui->get_permissions($attrs['dn'], 'supannRessourceState/supannRessourceState', 'fdSupannStateName'))) {
        continue;
      }
      if (!preg_match('/r/', $ui->get_permissions($attrs['dn'], 'supannRessourceState/supannRessourceState', 'fdSupannRessourceLabel'))) {
        continue;
      }
      // FIXME wrong attribut for label fdSupannRessourceLabel instead of fdSupannStateLabel
      $states += [$attrs['fdSupannStateName'][0] => $attrs['fdSupannRessourceLabel'][0]];
    }

    /* Value if no ressources are found */
    if (count($states) < 1) {
      $states = ['' => '-- No states found --'];
    }

    /* Sort */
    asort($states);

    return $states;
  }

  static function getSubstates (): array
  {
    global $config, $ui;

    /* LDAP object */
    $ldap = $config->get_ldap_link();
    /* FIXME when testing it felt in base, it must be in an OU normally */
    $ldap->cd($config->current['BASE']);

    /* Get states from LDAP */
    $states = [];
    $ldap->search('(objectClass=fdSupannRessourceState)', ['fdSupannStateName','fdSupannStateLabel', 'fdSupannSubStateList'], 'subtree');
    while ($attrs = $ldap->fetch()) {
      if (!preg_match('/r/', $ui->get_permissions($attrs['dn'], 'supannRessourceState/supannRessourceState', 'fdSupannStateName'))) {
        continue;
      }
      if (!preg_match('/r/', $ui->get_permissions($attrs['dn'], 'supannRessourceState/supannRessourceState', 'fdSupannSubStateList'))) {
        continue;
      }

      // FIXME better way to remove the $attrs['fdSupannSubStateList']['count']
      unset($attrs['fdSupannSubStateList']['count']);

      /* Prepare the state part
       * [
       *    'statePart' => [
       *        'substate1' => 'substate label'
       *     ]
       * ]
       */
      $states[$attrs['fdSupannStateName'][0]] = [];

      if (isset($attrs['fdSupannSubStateList'])) {
        foreach ($attrs['fdSupannSubStateList'] as $dn) {
          $states[$attrs['fdSupannStateName'][0]][] = $dn;
        }
      }
    }

    /* Sort */
    asort($states);

    /* Get substates from LDAP based on $states */
    $substates = [];
    foreach ($states as $stateName => $substateDNs) {
      $substates[$stateName] = [];
      foreach ($substateDNs as $substateDN) {
        // FIXME wrong attribute for label fdSupannRessourceLabel instead of fdSupannSubStateLabel
        $ldap->cat($substateDN, ['fdSupannSubStateName','fdSupannRessourceLabel']);
        while ($attrs = $ldap->fetch()) {
          if (!preg_match('/r/', $ui->get_permissions($attrs['dn'], 'supannRessourceSubState/supannRessourceSubState', 'fdSupannSubStateName'))) {
            continue;
          }
          // FIXME wrong attribute for label fdSupannRessourceLabel instead of fdSupannSubStateLabel
          if (!preg_match('/r/', $ui->get_permissions($attrs['dn'], 'supannRessourceSubState/supannRessourceSubState', 'fdSupannRessourceLabel'))) {
            continue;
          }
          /* Make the substate part
           * [
           *    'statePart' => [
           *        'substate1' => 'substate label'
           *      ]
           *  ]
           */
          // FIXME wrong attribute for label fdSupannRessourceLabel instead of fdSupannSubStateLabel
          $substates[$stateName] += [$attrs['fdSupannSubStateName'][0] => $attrs['fdSupannRessourceLabel'][0]];
        }
      }

      /* Sort */
      asort($substates[$stateName]);
    }

    /* Value if no states are found */
    if (count($states) < 1) {
      $substates = ['-- No substates found -- ' => ['' => '']];
    }

    return $substates;
  }

  static function getConfiguredSubstates (): array
  {
    global $config;

    $substates = static::getSubstates();
    return $substates;
  }
}
