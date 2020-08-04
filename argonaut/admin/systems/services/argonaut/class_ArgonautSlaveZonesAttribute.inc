<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2020 FusionDirectory

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

class ArgonautSlaveZonesAttribute extends OrderedArrayAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $order = TRUE, array $values = [], bool $edit_enabled = FALSE, string $acl = '')
  {
    $zoneAttributeClass = 'StringAttribute';
    if (class_available('ZoneNameAttribute')) {
      $zoneAttributeClass = 'ZoneNameAttribute';
    }
    parent::__construct(
      new PipeSeparatedCompositeAttribute(
        $description,
        $ldapName,
        [
          new $zoneAttributeClass(
            _('zone'), _('DNS zone this server should be declared as slave of'),
            'Ldap2zoneSlaveZone', TRUE
          ),
          new StringAttribute(
            _('master'), _('Master IP(s) for this zone'),
            'Ldap2zoneSlaveMaster', TRUE
          ),
          new BooleanAttribute(
            _('reverses'), _('This server should be slave of reverses zones as well'),
            'Ldap2zoneSlaveReverses', TRUE,
            TRUE, '',
            'reverse', 'noreverse'
          )
        ],
        $acl,
        $label
      ),
      $order,
      $values,
      $edit_enabled
    );
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    /* If there are DNS zones from DNS plugin, replace DNS zone field with a SelectAttribute */
    try {
      $zones = objects::ls('dnsZone', NULL, NULL, '', TRUE);
    } catch (NonExistingObjectTypeException $e) {
      $zones = [];
    }
    if (!empty($zones)) {
      $attributes = $this->attribute->attributes;
      $attributes[0] = new SelectAttribute(
        _('zone'), _('DNS zone this server should be declared as slave of'),
        'Ldap2zoneSlaveZone', TRUE,
        $zones
      );
      $this->attribute->setAttributes($attributes);
    }
  }

  function foreignKeyUpdate ($oldvalue, $newvalue, array $source)
  {
    if (($source['FIELD'] == 'zoneName') && ($source['MODE'] == 'move')) {
      foreach ($this->value as $key => $value) {
        if (preg_match('/^'.preg_quote($oldvalue, '/').'\|/', $value)) {
          if ($newvalue === NULL) {
            unset($this->value[$key]);
          } else {
            $this->value[$key] = preg_replace('/^'.preg_quote($oldvalue, '/').'/', $newvalue, $value);
          }
        }
      }
    } else {
      parent::foreignKeyUpdate($oldvalue, $newvalue, $source);
    }
  }

  function foreignKeyCheck ($value, array $source): bool
  {
    if ($source['FIELD'] == 'zoneName') {
      foreach ($this->value as $myvalue) {
        if (preg_match('/^'.preg_quote($value, '/').'\|/', $myvalue)) {
          return TRUE;
        }
      }
      return FALSE;
    } else {
      return parent::foreignKeyCheck($value, $source);
    }
  }
}
