<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2020-2021 FusionDirectory

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

class schacAccount extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SCHAC'),
      'plDescription' => _('SCHema for ACademia'),
      'plIcon'        => 'geticon.php?context=applications&icon=schac&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=schac&size=16',
      'plSelfModify'  => TRUE,
      'plObjectClass' => ['schacLinkageIdentifiers'],
      'plObjectType'  => ['user'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'student' => [
        'name'  => _('Linkage identifiers'),
        'attrs' => [
          new SetAttribute(
            new StringAttribute(
              _('Personal unique code'), _('Unique code for the subject'),
              'schacPersonalUniqueCode', FALSE
            )
          ),
        ]
      ],
    ];
  }

  public function update (): bool
  {
    parent::update();

    if (($supannTab = $this->parent->getTabOrServiceObject('supannAccount')) !== FALSE) {
      $ine = $supannTab->supannCodeINE;
      $uniquecodes = $this->schacPersonalUniqueCode;
      foreach ($uniquecodes as $key => $uniquecode) {
        if (preg_match('/^urn:schac:personalUniqueCode:int:esi:fr:/', $uniquecode)) {
          unset($uniquecodes[$key]);
        }
      }
      if (!empty($ine)) {
        $uniquecodes[] = 'urn:schac:personalUniqueCode:int:esi:fr:'.$ine;
      }
      $this->schacPersonalUniqueCode = array_values($uniquecodes);
    }

    return TRUE;
  }
}
