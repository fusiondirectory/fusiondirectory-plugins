<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2015-2018 FusionDirectory

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

class GeneralizedTimeDisplayAttribute extends GeneralizedTimeDateAttribute
{
  function getValue ()
  {
    return $this->computeLdapValue();
  }

  function renderFormInput ()
  {
    $date = $this->getDateValue();
    $date->setTimezone(timezone::getDefaultTimeZone());
    return htmlentities($date->format('Y-m-d, H:i:s'), ENT_COMPAT, 'UTF-8');
  }
}

class UserLinkAttribute extends DisplayLDAPAttribute
{
  function renderFormInput ()
  {
    try {
      return objects::link($this->value, 'user');
    } catch (NonExistingLdapNodeException $e) {
      return '<a><img src="geticon.php?context=status&amp;icon=dialog-warning&amp;size=16" alt="warning" class="center"/>&nbsp;'.sprintf(_('Unknown user: %s'), $this->value).'</a>';
    }
  }
}

class auditEvent extends simplePlugin
{
  public $base;

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Audit event'),
      'plDescription' => _('An event like ldap modification which was registered by audit plugin'),
      'plObjectClass' => array('fdAuditEvent'),
      'plObjectType'  => array('auditEvent' => array(
        'name'        => _('Audit event'),
        'icon'        => 'geticon.php?context=applications&icon=audit&size=16',
        'ou'          => get_ou('auditRDN'),
        'mainAttr'    => FALSE,
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Event'),
        'attrs' => array(
          new HiddenAttribute ('fdAuditId'),
          new GeneralizedTimeDisplayAttribute(
            _('Time'), _('Date and time this event happened'),
            'fdAuditDateTime', TRUE
          ),
          new DisplayLDAPAttribute  (_('Action'),       _('Action type'),     'fdAuditAction',      TRUE),
          new UserLinkAttribute     (_('Author'),       _('Action author'),   'fdAuditAuthorDN',    TRUE),
          new DisplayLDAPAttribute  (_('Object type'),  _('Object type'),     'fdAuditObjectType',  TRUE),
          new DisplayLDAPAttribute  (_('Object'),       _('Target object'),   'fdAuditObject',      TRUE),
          new DisplayLDAPArrayAttribute(_('Attributes'), _('Target attributes'), 'fdAuditAttributes', FALSE),
          new DisplayLDAPAttribute  (_('Result'),       _('Result or error'), 'fdAuditResult',      FALSE),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
    $this->fdAuditId = mt_rand();
  }

  function compute_dn()
  {
    return $this->create_unique_dn('fdAuditDateTime', get_ou('auditRDN').$this->base);
  }
}
?>
