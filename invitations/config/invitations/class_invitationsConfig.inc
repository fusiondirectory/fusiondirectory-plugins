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

class invitationsConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Invitations configuration'),
      'plDescription' => _('FusionDirectory invitations plugin configuration'),
      'plObjectClass' => array('fdInvitationsPluginConf'),
      'plCategory'    => array('configuration'),
      'plObjectType'  => array('smallConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'  => _('Invitations'),
        'attrs' => array(
          new StringAttribute(
            _('Invitations RDN'), _('Branch in which invitations will be stored'),
            'fdInvitationRDN', TRUE,
            'ou=invitations'
          ),
          new StringAttribute(
            _('Registrations RDN'), _('Branch in which registrations will be stored'),
            'fdRegistrationRDN', TRUE,
            'ou=registrations'
          ),
          new StringAttribute(
            _('Email expeditor'), _('The email address used to send invitation emails. Be careful to select an address that your server can send from without being flagged as spam.'),
            'fdInvitationFromEmailAddress', TRUE,
            'no-reply@example.com'
          ),
        )
      ),
      'default' => array(
        'name'  => _('Invitations'),
        'attrs' => array(
          new TextAreaAttribute(
            _('Default email content'), _('Default content of the email sent invitations'),
            'fdInvitationDefaultEmailContent', TRUE,
            'Hello,'."\n\n".
              'Please subscribe by going to https://'.$_SERVER['SERVER_NAME'].
              (($_SERVER['SERVER_PORT'] != '443') ? ':'.$_SERVER['SERVER_PORT'] : '').
              "/invitation.php?token=%s\n"
          ),
        )
      ),
    );
  }
}
?>
