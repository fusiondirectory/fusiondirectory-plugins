<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018 FusionDirectory

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

class RegistrationLastChangeAttribute extends DisplayLDAPAttribute
{
  static function renderValue ($value)
  {
    if (empty($value)) {
      return $value;
    }

    list($ldapDate, $author) = explode('|', $value, 2);
    try {
      $date = new DateTime($ldapDate);
    } catch (Exception $e) {
      msg_dialog::display(_('Error'), $e->getMessage(), ERROR_DIALOG);
      $date = new DateTime();
    }
    $date->setTimezone(timezone::getDefaultTimeZone());
    try {
      $author = objects::link($author, 'user');
    } catch (NonExistingLdapNodeException $e) {
      $author = '<img src="geticon.php?context=types&amp;icon=application&amp;size=16" alt="" class="center"/>&nbsp;'.$author;
    }
    return sprintf(_('%s - by %s'), $date->format('Y-m-d H:i:s'), $author);
  }

  function renderFormInput ()
  {
    $value = static::renderValue($this->getValue());
    if ($this->allowSmarty) {
      return $value;
    } else {
      return '{literal}'.$value.'{/literal}';
    }
  }
}

class RegistrationStateAttribute extends DisplayLDAPAttribute
{
  static function getStates ()
  {
    return [
      'sent'      => [_('Sent'),     'geticon.php?context=status&icon=sent&size=16'],
      'filled'    => [_('Filled'),   'geticon.php?context=status&icon=filled&size=16'],
      'accepted'  => [_('Accepted'), 'geticon.php?context=status&icon=accepted&size=16'],
      'rejected'  => [_('Rejected'), 'geticon.php?context=status&icon=rejected&size=16'],
    ];
  }

  static function renderValue ($value)
  {
    $values = static::getStates();
    if (isset($values[$value])) {
      return sprintf('<img src="%2$s" alt="" class="center"/>&nbsp;%1$s', $values[$value][0], htmlentities($values[$value][1], ENT_COMPAT, 'UTF-8'));
    }

    return $value;
  }

  function renderFormInput ()
  {
    $value = static::renderValue($this->getValue());
    if ($this->allowSmarty) {
      return $value;
    } else {
      return '{literal}'.$value.'{/literal}';
    }
  }
}

class registration extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Registration'),
      'plDescription' => _('Registration sent through a public form'),
      'plObjectClass' => ['fdRegistration'],
      'plObjectType'  => ['registration' => [
        'name'      => _('Registration'),
        'ou'        => get_ou('registrationRDN'),
        'icon'      => 'geticon.php?context=types&icon=registration&size=16',
        'mainAttr'  => 'fdRegistrationEmailAddress',
      ]],
      'plForeignKeys'   => [
        'fdRegistrationInvitationDN'  => 'invitation',
        'fdRegistrationUserDN'        => 'user',
      ],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Registration'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('registrationRDN')),
          new DisplayLDAPAttribute(
            _('Email address'), _('Email address which the invitation was sent to'),
            'fdRegistrationEmailAddress', TRUE
          ),
          new RegistrationStateAttribute(
            _('Status'), _('State of this registration'),
            'fdRegistrationState', FALSE
          ),
          new RegistrationLastChangeAttribute(
            _('Last change'), _('Date and author of the last state change'),
            'fdRegistrationLastChange', FALSE
          ),
          new ObjectLinkAttribute(
            _('Invitation'), _('Invitation object used to create this registration'),
            'fdRegistrationInvitationDN', FALSE,
            'invitation'
          ),
          new ObjectLinkAttribute(
            _('User object'), _('User object created by this registration'),
            'fdRegistrationUserDN', FALSE,
            'user'
          ),
          new HiddenAttribute('fdRegistrationToken'),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
  }
}
