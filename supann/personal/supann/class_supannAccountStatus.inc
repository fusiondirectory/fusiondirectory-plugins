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

class supannAccountStatus extends simplePlugin
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
            new CharSeparatedCompositeAttribute(
              _('State of the account and associated resources'),
              'supannRessourceEtatDate',
              [
                new SelectAttribute(
                  _('Resource'), _('Which resource this state concerns'),
                  'supannRessourceEtatDate_label', TRUE,
                  ['{COMPTE}','{MAIL}'], '',
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
              ':',
              '',
              ''
            ),
            FALSE,
            [],
            TRUE
          )
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->substatus = [
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
  }

  /* Update choices of fields which depends on other fields values */
  function updateFieldsChoices ()
  {
    $substatusChoices = $this->substatus[$this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[1]->getValue()];
    $this->attributesAccess['supannRessourceEtatDate']->attribute->attributes[2]->setChoices(
      array_keys($substatusChoices),
      array_values($substatusChoices)
    );
  }
}
