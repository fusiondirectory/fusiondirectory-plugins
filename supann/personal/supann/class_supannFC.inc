<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2014-2021  FusionDirectory

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

class supannFC extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('France Connect'),
      'plDescription' => _('France Connect identity'),
      'plSelfModify'  => TRUE,
      'plObjectClass' => ['supannFCPerson'],
      'plObjectType'  => ['user'],
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann&size=16',
      'plPriority'    => 17,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'civil' => [
        'name'  => _('Civil status'),
        'attrs' => [
          new StringAttribute(
            _('Birth name'), _('Last name from birth'),
            'supannNomDeNaissance', TRUE
          ),
          new StringAttribute(
            _('First names'), _('Space separated first names'),
            'supannPrenomsEtatCivil', TRUE
          ),
          new DateAttribute(
            _('Date of birth'), _('Date of birth'),
            'supannOIDCDateDeNaissance', TRUE,
            'Y-m-d', '',
            // min and max:
            NULL, 'now'
          ),
          new SelectAttribute(
            _('Gender'), _('Gender'),
            'supannOIDCGenre', TRUE,
            ['other', 'female', 'male'], '', [_('Other'), _('Female'), _('Male')]
          ),
          new StringAttribute(
            _('Country of birth'), _('INSEE code of country of birth'),
            'supannCodeINSEEPaysDeNaissance', TRUE,
            '',
            '',
            '/^\d{5}$/',
            '99134'
          ),
          new StringAttribute(
            _('City of birth'), _('INSEE code of city of birth, if in France'),
            'supannCodeINSEEVilleDeNaissance', FALSE,
            '',
            '',
            '/^[\dAB]{5}$/',
            '06088'
          ),
        ],
      ],
      'id' => [
        'name'  => _('OpenID Connect'),
        'attrs' => [
          new SetAttribute(
            new StringAttribute(
              _('Subject'), _('Subject from OpenID Connect standard in the France Connect system'),
              'supannFCSub', TRUE,
              '',
              '',
              '/^[[:ascii:]]{1,255}$/',
            )
          ),
        ],
      ],
    ];
  }

  function __construct (string $dn = NULL, $object = NULL, $parent = NULL, bool $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['supannFCSub']->setUnique('whole');
  }
}
