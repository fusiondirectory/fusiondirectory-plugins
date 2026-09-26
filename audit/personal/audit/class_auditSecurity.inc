<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2026  FusionDirectory

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
      'plShortName'    => _('Security'),
      'plDescription'  => _('Security audit'),
      'plObjectType'   => ['user'],
      'plSelfModify'   => TRUE,
      'plIcon'         => 'geticon.php?context=applications&icon=audit&size=48',
      'plSmallIcon'    => 'geticon.php?context=applications&icon=audit&size=16',
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
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
          new CompositeAttribute(
            _('Starting from'),
            'auditStart',
            [
              new GeneralizedTimeDateAttribute(
                _('Starting from'), '',
                'auditStartDate', TRUE,
              ),
              new ButtonAttribute(
                '', '',
                'auditStartButton', _('Apply filter'), 'update', TRUE
              ),
            ],
            TRUE,
            '',
            '',
            _('Starting from'),
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

  protected function loadEvents ()
  {
    $auditStartDatetime = new DateTime(
      $this->attributesAccess['auditStart']->attributes[0]->getValue(),
      timezone::utc()
    );
    $auditStartDatetime->setTimeZone(timezone::getDefaultTimeZone());

    $auditStart = LdapGeneralizedTime::toString($auditStartDatetime);

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

        '(&'.
          '(ObjectClass=fdAuditEvent)'.
          '(fdAuditDateTime>='.ldap_escape_f($auditStart).')'.
          '(fdAuditObject='.ldap_escape_f($this->dn).')'.
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

  public function update (): bool
  {
    parent::update();
    $this->loadEvents();
    return TRUE;
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
