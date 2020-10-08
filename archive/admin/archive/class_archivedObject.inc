<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

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

class archivedObject extends simplePlugin
{
  public $base;

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Archived Object'),
      'plDescription' => _('An object which have been archived through archive plugin'),
      'plObjectClass' => ['fdArchivedObject'],
      'plObjectType'  => ['archivedObject' => [
        'name'        => _('Archived Object'),
        'icon'        => 'geticon.php?context=mimetypes&icon=application-x-archive&size=16',
        'ou'          => get_ou('archiveRDN'),
        'mainAttr'    => FALSE,
      ]],
      'plSearchAttrs' => [],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'fields' => [
        'name'  => _('Data'),
        'attrs' => [
          new ArchivedFieldsAttribute(
            _('Fields'), _('Archived fields'),
            'fdArchivedField', FALSE
          ),
          new ArchivedFieldsAttribute(
            _('Unique fields'), _('Archived unique fields'),
            'fdArchivedUniqueField', FALSE
          ),
        ],
      ],
      'origin' => [
        'name'  => _('Origin'),
        'attrs' => [
          new GeneralizedTimeDisplayAttribute(
            _('Time'), _('Date and time this object was archived'),
            'fdArchivedDateTime', TRUE
          ),
          new DisplayObjectTypeAttribute(
            _('Object type'), _('Object type'),
            'fdArchivedObjectType', TRUE
          ),
          new DisplayLDAPAttribute(
            _('Origin DN'), _('DN of the object before deletion'),
            'fdArchivedOriginDn', TRUE
          ),
        ],
      ],
    ];
  }

  public function compute_dn (): string
  {
    return $this->create_unique_dn('fdArchivedDateTime', get_ou('archiveRDN').$this->base);
  }

  public static function listArchiveTypes (): array
  {
    global $config;

    $types          = [];
    $typesAndFields = $config->get_cfg_value('ArchivedTypesAndFields', []);
    foreach ($typesAndFields as $typeAndFields) {
      list($configType, , ) = explode('|', $typeAndFields);
      $types[] = strtoupper($configType);
    }

    return array_unique($types);
  }

  /*!
   * \brief Test if archive is active for a given type
   */
  public static function isArchiveActive (string $type): bool
  {
    global $config;

    $typesAndFields = $config->get_cfg_value('ArchivedTypesAndFields', []);
    foreach ($typesAndFields as $typeAndFields) {
      list($configType, $fields, $uniquefields) = explode('|', $typeAndFields);
      if (strtoupper($configType) == strtoupper($type)) {
        return TRUE;
      }
    }

    return FALSE;
  }

  /*!
   * \brief Returns an action for a management class, or null
   */
  public static function getManagementAction (array $types, $callable)
  {
    $types = array_filter(
      $types,
      function ($type) {
        $infos = objects::infos($type);
        return (isset($infos['archiveActive']) && $infos['archiveActive']);
      }
    );
    if (empty($types)) {
      return NULL;
    } else {
      return new Action(
        'archive', _('Archive'), 'geticon.php?context=actions&icon=archive&size=16',
        '+', $callable,
        ['/archive/c', 'r'],
        TRUE,
        TRUE,
        $types
      );
    }
  }

  /*!
   * \brief Archive object data and delete it
   */
  public static function archiveObject (string $type, string $dn): array
  {
    global $config;

    $type = strtoupper($type);
    $typesAndFields = $config->get_cfg_value('ArchivedTypesAndFields', []);
    foreach ($typesAndFields as $typeAndFields) {
      list($configType, $fields, $uniquefields) = explode('|', $typeAndFields);
      if (strtoupper($configType) == $type) {
        $fields       = explode(',', $fields);
        $uniquefields = explode(',', $uniquefields);
        $attrs = [];
        foreach ($fields as $field) {
          $attrs[$field] = '*';
        }
        foreach ($uniquefields as $field) {
          $attrs[$field] = '*';
        }
        $objects = objects::ls('user', $attrs, $dn, '', FALSE, 'base');

        $fdArchivedField = [];
        foreach ($fields as $field) {
          if (isset($objects[$dn][$field])) {
            foreach ($objects[$dn][$field] as $value) {
              $fdArchivedField[] = $field.':'.$value;
            }
          }
        }
        $fdArchivedUniqueField = [];
        foreach ($uniquefields as $field) {
          if (isset($objects[$dn][$field])) {
            foreach ($objects[$dn][$field] as $value) {
              $fdArchivedUniqueField[] = $field.':'.$value;
            }
          }
        }

        /* Archive data */
        $archivedObject = objects::create('archivedObject');
        $baseObject = $archivedObject->getBaseObject();
        $baseObject->fdArchivedDateTime     = new DateTime('now');
        $baseObject->fdArchivedObjectType   = $type;
        $baseObject->fdArchivedOriginDn     = $dn;
        $baseObject->fdArchivedField        = $fdArchivedField;
        $baseObject->fdArchivedUniqueField  = $fdArchivedUniqueField;
        $baseObject->base                   = dn2base($dn);
        $errors = $archivedObject->save();
        if (!empty($errors)) {
          return $errors;
        }
        /* Delete object */
        $errors = objects::delete($dn, $type, FALSE);
        if (!empty($errors)) {
          /* Deletion failed */
          $archivedObject->delete(FALSE);
          return $errors;
        }
        /* No errors */
        return [];
      }
    }
    return [new FusionDirectoryError(htmlescape(sprintf(_('No configuration found for archiving type %s'), $type)))];
  }
}
