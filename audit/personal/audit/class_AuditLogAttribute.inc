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

class AuditLogAttribute extends OrderedArrayAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = '')
  {
    Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = FALSE;
    $this->attribute    = FALSE;
    $this->order        = FALSE;
    $this->setInLdap(FALSE);
  }

  protected function getAttributeArrayValue ($key, $event)
  {
    try {
      $author = ['html' => objects::link($event['fdAuditAuthorDN'], 'user')];
    } catch (FusionDirectoryException $e) {
      $author = $event['fdAuditAuthorDN'];
    }
    try {
      $time = static::formatDateDiff(LdapGeneralizedTime::fromString($event['fdAuditDateTime']));
    } catch (Exception $e) {
      $time = $event['fdAuditDateTime'];
    }
    switch ($event['fdAuditAction']) {
      case 'security':
        if ($event['fdAuditObjectType'] == 'login') {
          $name = _('Sign in');
        } elseif ($event['fdAuditObjectType'] == 'logout') {
          $name = _('Sign out');
        } else {
          $name = $event['fdAuditObjectType'];
        }
        break;
      case 'modify':
        if (in_array('userPassword', $event['fdAuditAttributes'])) {
          $name = _('Password modification');
        } else {
          $name = sprintf(_('Account modification (%s)'), implode(',', $event['fdAuditAttributes']));
        }
        break;
      case 'create':
      case 'remove':
      case 'copy':
      case 'snapshot':
      case 'view':
      case 'debug':
      default:
        $name = $event['fdAuditAction'].' '.$event['fdAuditObjectType'];
        break;
    }
    return [
      $name,
      $author,
      $event['fdAuditAuthorIP'] ?? '',
      $time,
      $event['fdAuditResult'],
    ];
  }

  protected function genRowIcons ($key, $value)
  {
    return ['', 0];
  }

  public function htmlIds (): array
  {
    return [];
  }

  function renderButtons ()
  {
    return '';
  }

  public static function formatDateDiff ($date)
  {
    $now = new DateTime();
    $interval = $now->diff($date);

    if ($interval->y > 0) {
      return sprintf(_('%s years(s) ago'), $interval->y);
    }
    if ($interval->m > 0) {
      return sprintf(_('%s month(s) ago'), $interval->m);
    }
    if ($interval->d > 0) {
      return sprintf(_('%s day(s) ago'), $interval->d);
    }
    if ($interval->h > 0) {
      return sprintf(_('%s hour(s) ago'), $interval->h);
    }
    if ($interval->i > 0) {
      return sprintf(_('%s minute(s) ago'), $interval->i);
    }
    if ($interval->s > 0) {
      return sprintf(_('%s second(s) ago'), $interval->s);
    }

    return _('Now');
  }
}
