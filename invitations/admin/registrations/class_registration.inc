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
  function renderFormInput ()
  {
    $value = $this->getValue();
    if (!empty($value)) {
      list($ldapDate, $author) = explode('|', $value, 2);
      try {
        $date = LdapGeneralizedTime::fromString($ldapDate);
      } catch (LdapGeneralizedTimeBadFormatException $e) {
        msg_dialog::display(_('Error'), $e->getMessage(), ERROR_DIALOG);
        $date = new DateTime();
      }
      $date->setTimezone(timezone::getDefaultTimeZone());
      $value = sprintf(_('%s - by %s'), $date->format('Y-m-d H:i:s'), $author);
    }
    if ($this->allowSmarty) {
      return $value;
    } else {
      return '{literal}'.$value.'{/literal}';
    }
  }
}

class registration extends simplePlugin
{
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Registration'),
      'plDescription' => _('Registration sent through a public form'),
      'plIcon'        => '',
      'plObjectClass' => array('fdRegistration'),
      'plObjectType'  => array('registration' => array(
        'name'      => _('Registration'),
        'ou'        => get_ou('registrationRDN'),
        'icon'      => '',
        'mainAttr'  => 'fdRegistrationEmailAddress',
      )),
      'plForeignKeys'   => array(
        'fdRegistrationInvitationDN'  => 'invitation',
        'fdRegistrationUserDN'        => 'user',
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Registration'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('registrationRDN')),
          new DisplayLDAPAttribute(
            _('Email address'), _('Email address which the invitation was sent to'),
            'fdRegistrationEmailAddress', TRUE
          ),
          new DisplayLDAPAttribute(
            _('Status'), _('State of this registration'),
            'fdRegistrationState', FALSE
          ),
          new RegistrationLastChangeAttribute(
            _('Last change'), _('Date and author of the last state change'),
            'fdRegistrationLastChange', FALSE
          ),
          new DisplayLDAPAttribute(
            _('Invitation'), _('Invitation object used to create this registration'),
            'fdRegistrationInvitationDN', FALSE
          ),
          new DisplayLDAPAttribute(
            _('User object'), _('User object created by this registration'),
            'fdRegistrationUserDN', FALSE
          ),
          new HiddenAttribute('fdRegistrationToken'),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
  }
}
