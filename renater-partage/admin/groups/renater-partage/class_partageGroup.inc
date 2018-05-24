<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017 FusionDirectory

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

class partageGroup extends simplePlugin
{
  var $displayHeader  = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'    => _('Partage'),
      'plDescription'  => _('Partage group options'),
      'plFilter'       => '(objectClass=fdRenaterPartageGroup)',
      'plSmallIcon'    => 'geticon.php?context=applications&icon=renater-partage&size=16',
      'plDepends'      => array('mailGroup'),
      'plObjectClass'  => array('fdRenaterPartageGroup'),
      'plObjectType'   => array('group', 'ogroup-user'),
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Information'),
        'attrs' => array(
          new StringAttribute (
            _('Display name'), _('Name to display for this group'),
            'displayName'
          ),
          new BooleanAttribute (
            _('Hide from global catalog'), _('Whether this group should be hidden from the global catalog'),
            'fdRenaterPartageGroupHideInGal', FALSE,
            FALSE
          ),
          new SelectAttribute (
            _('Type'), _('Type of PARTAGE group'),
            'fdRenaterPartageGroupMailStatus', TRUE,
            array('enabled','disabled'), 'enabled',
            array(_('Distribution list'), _('Group'))
          ),
          new BooleanAttribute (
            _('Alert new members'), _('Send a message to alert new members, they have joined this group'),
            'fdRenaterPartageGroupSendShareMessageToNewMembers', FALSE,
            TRUE
          ),
          new TextAreaAttribute (
            _('Notes'), _('Notes about this group'),
            'fdRenaterPartageGroupNotes', FALSE
          ),
        )
      )
    );
  }
}
