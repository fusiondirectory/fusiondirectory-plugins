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

class invitation extends simplePlugin
{
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Invitation'),
      'plDescription' => _('Invitation to fill a public form'),
      'plIcon'        => '',
      'plObjectClass' => array('fdInvitation'),
      'plObjectType'  => array('invitation' => array(
        'name'    => _('Invitation'),
        'ou'      => get_ou('invitationRDN'),
        'icon'    => '',
      )),
      'plForeignKeys'   => array(
        'fdInvitationFormDN' => 'publicForm',
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    global $ui;
    return array(
      'main' => array(
        'name'  => _('Invitation'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('invitationRDN')),
          new HostNameAttribute(
            _('Name'), _('Name used to identify this invitation'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Email subject'), _('Subject of the email sent for this invitation'),
            'fdInvitationEmailSubject', TRUE
          ),
          new TextAreaAttribute(
            _('Email content'), _('Content of the email sent for this invitation'),
            'fdInvitationEmailContent', TRUE
          ),
          new SelectAttribute(
            _('Form'), _('Public form this invitation should link to'),
            'fdInvitationFormDN', TRUE
          ),
          new DisplayLDAPAttribute(
            _('Author'), _('Author of this invitation'),
            'fdInvitationAuthor', FALSE,
            $ui->dn
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    if ($attributesInfo === NULL) {
      $attributesInfo = $this->getAttributesInfo();
    }
    $choices = objects::ls('publicForm');
    $attributesInfo['main']['attrs'][4]->setChoices(array_keys($choices), array_values($choices));

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
  }
}
?>
