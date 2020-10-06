<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2019-2020 FusionDirectory

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

class archiveConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Archive configuration'),
      'plDescription'   => _('FusionDirectory archive plugin configuration'),
      'plObjectClass'   => ['fdArchivePluginConf'],
      'plCategory'      => ['configuration'],
      'plObjectType'    => ['smallConfig'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Archive'),
        'attrs' => [
          new StringAttribute(
            _('Archive RDN'), _('Branch in which archived objects will be stored'),
            'fdArchiveRDN', TRUE,
            'ou=archive'
          ),
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Use this to hide some menu entry to specific groups of users'),
              'fdArchivedTypesAndFields',
              [
                new SelectAttribute(
                  _('Type'), _('Object type'),
                  'fdArchivedTypesAndFields_type', TRUE,
                  /* People usually archive users, so set it as default */
                  ['USER'], 'USER'
                ),
                new StringAttribute(
                  _('Fields'), _('Comma separated list of fields to archive for history purposes'),
                  'fdArchivedTypesAndFields_fields', FALSE
                ),
                new StringAttribute(
                  _('Unique fields'), _('Comma separated list of fields to archive for history purposes and unicity checks'),
                  'fdArchivedTypesAndFields_uniquefields', FALSE
                ),
              ],
              '',
              _('Type and fields')
            ),
            // no order
            FALSE,
            []
          ),
        ]
      ],
    ];
  }

  function __construct (string $dn = NULL, $object = NULL, $parent = NULL, bool $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $choices = static::getManagedTypes();
    $this->attributesAccess['fdArchivedTypesAndFields']->attribute->attributes[0]->setChoices(array_keys($choices), array_values($choices));
    $this->attributesAccess['fdArchivedTypesAndFields']->setLinearRendering(FALSE);
  }

  public static function getManagedTypes (): array
  {
    $result = [];
    $types  = objects::types();
    foreach ($types as $type) {
      $infos = objects::infos($type);
      if (isset($infos['management'])) {
        $result[$type] = $infos['name'];
      }
    }
    asort($result);
    return $result;
  }
}
