<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019  FusionDirectory

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

class auditSecurity extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Security'),
      'plDescription' => _('Security audit'),
      'plObjectType'  => ['user'],
      'plSelfModify'  => TRUE,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Security audit'),
        'class' => ['fullwidth'],
        'attrs' => [
          new AuditLogAttribute(
            '', _('Important events involving your account'),
            'fdAuditSecurityLog'
          ),
        ],
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['fdAuditSecurityLog']->setHeaders([
      _('Event'),
      _('Author'),
      _('Origin'),
      _('Time'),
      _('Result'),
    ]);
  }

  protected function loadEvents()
  {
    $events = objects::ls(
      'auditEvent',
      [
        'fdAuditAction'     => 1,
        'fdAuditDateTime'   => 1,
        'fdAuditAuthorDN'   => 1,
        'fdAuditAuthorIP'   => 1,
        'fdAuditObjectType' => 1,
        'fdAuditObject'     => 1,
        'fdAuditAttributes' => '*',
        'fdAuditResult'     => 1
      ],
      NULL,
      '(|'.
        '(&'.
          '(fdAuditAction=security)'.
          '(|'.
            '(fdAuditObject='.ldap_escape_f($this->getUid()).')'.
            '(fdAuditObject='.ldap_escape_f($this->dn).')'.
            '(fdAuditAuthorDN='.ldap_escape_f($this->dn).')'.
          ')'.
        ')'.
        '(&'.
          '(fdAuditAttributes=userPassword)'.
          '(fdAuditObject='.ldap_escape_f($this->dn).')'.
        ')'.
      ')'
    );
    uasort(
      $events,
      function ($event1, $event2)
      {
        return $event2['fdAuditDateTime'] <=> $event1['fdAuditDateTime'];
      }
    );
    $this->fdAuditSecurityLog = $events;
  }

  public function execute (): string
  {
    $this->loadEvents();
    return parent::execute();
  }

  protected function getUid (): string
  {
    if (isset($this->parent)) {
      $baseobject = $this->parent->getBaseObject();
      return $baseobject->uid;
    }
    if (isset($this->attrs['uid'][0])) {
      return $this->attrs['uid'][0];
    }
    return '';
  }

  function check (): array
  {
    return [];
  }

  function save (): array
  {
    return [];
  }

  function remove (bool $fulldelete = FALSE): array
  {
    return [];
  }
}
