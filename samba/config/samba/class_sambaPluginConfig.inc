<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016 FusionDirectory

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

class sambaPluginConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return array(
      'plShortName'     => _('Samba'),
      'plDescription'   => _('Samba plugin configuration'),
      'plPriority'      => 2,
      'plObjectClass'   => array('fdSambaPluginConf'),
      'plObjectType'    => array('configuration'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'samba' => array(
        'name'  => _('Samba settings'),
        'attrs' => array(
          new BooleanAttribute(
            _('Samba ID mapping'),
            _('Maintain sambaIdmapEntry objects. Depending on your setup this can drastically improve the windows login performance.'),
            'fdSambaIdMapping'
          ),
          new StringAttribute(
            _('Samba SID'),
            _('A samba SID if not available inside of the LDAP though samba schema. You can retrieve the current sid by net getlocalsid.'),
            'fdSambaSID', FALSE,
            '0-815-4711'
          ),
          new IntAttribute(
            _('Samba rid base'),
            _('The base id to add to ordinary sid calculations - if not available inside of the LDAP though samba schema.'),
            'fdSambaRidBase', FALSE,
            0, FALSE, 1
          ),
          new SelectAttribute(
            _('Expiration date synchronisaton'), _('Synchronisaton the expiration date with the POSIX one?'),
            'fdSambaExpirationSync', FALSE,
            array('', 'posix', 'samba'), '',
            array('Do not synchronise', 'Synchronise from POSIX', 'Synchronise to POSIX')
          ),
          new BooleanAttribute(
            _('Generate sambaLMPassword'), _('Needed to be compliant with Windows <= 98 or Samba < 3.2'),
            'fdSambaGenLMPassword', FALSE
          ),
          new BooleanAttribute(
            _('Activate primary group warning'), _('Issue a warning if POSIX primary group cannot be converted to Samba primary group when activating the Samba tab of a user'),
            'fdSambaPrimaryGroupWarning', FALSE
          )
        )
      ),
    );
  }
}
?>
