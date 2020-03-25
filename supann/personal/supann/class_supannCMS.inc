<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2018-2020 FusionDirectory

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

class supannCMS extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Multiservice card'),
      'plDescription' => _('SupAnn CMS management'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 16,
      'plObjectClass' => ['supannCMS'],
      'plObjectType'  => ['user'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'status' => [
        'name'  => _('Cards'),
        'class' => ['fullwidth'],
        'attrs' => [
          new HiddenArrayAttribute('supannCMSType', FALSE, []),
          new TaggedAttribute(new StringAttribute('', '', 'supannCMSId', FALSE)),
          new HiddenArrayAttribute('supannCMSIdEtiquette', FALSE, []),
          new HiddenArrayAttribute('supannCMSSource', FALSE, []),
          new HiddenArrayAttribute('supannCMSDateFin', FALSE, []),
          new SupannOrderedArrayAttribute(
            new SupannCMSAffectationAttribute(
              _('Cards'), _('SupAnn CMS Affectation'),
              'supannCMSAffectation'
            ),
            // no order
            FALSE,
            [],
            // edit button
            TRUE
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['supannCMSId']->setVisible(FALSE);
    $this->attributesAccess['supannCMSAffectation']->setLinearRendering(FALSE);
    $this->attributesAccess['supannCMSAffectation']->setHeaders([
      _('Type'),
      _('Format'),
      _('Id'),
      _('Valid'),
      _('Source'),
      _('End date'),
      ''
    ]);
  }

  protected function prepare_save (): array
  {
    $this->attributesAccess['supannCMSAffectation']->supannPrepareSave();
    return parent::prepare_save();
  }
}
