<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016 FusionDirectory

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

class auditConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return array(
      'plShortName'     => _('Audit configuration'),
      'plDescription'   => _('FusionDirectory audit plugin configuration'),
      'plObjectClass'   => array('fdAuditPluginConf'),
      'plCategory'      => array('configuration'),
      'plObjectType'    => array('smallConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'  => _('Audit'),
        'attrs' => array(
          new StringAttribute(
            _('Audit RDN'), _('Branch in which audit events will be stored'),
            'fdAuditRDN', TRUE,
            'ou=audit'
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Actions to audit'), _('Which actions should be stored in LDAP audit log'),
              'fdAuditActions', TRUE,
              logging::$validActions
            ),
            array('modify','create','remove')
          ),
          new IntAttribute(
            _('Days to keep'), _('Number of days of audit to keep in the LDAP when cleaning'),
            'fdAuditRotationDelay', TRUE,
            1, FALSE, 120
          ),
        )
      ),
    );
  }
}
