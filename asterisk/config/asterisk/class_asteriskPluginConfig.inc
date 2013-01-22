<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013 FusionDirectory

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

class asteriskPluginConfig extends simplePlugin
{
  var $objectclasses = array('fdAsteriskPluginConf');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Asterisk'),
      'plDescription' => _('Asterisk plugin configuration'),
      'plCategory'    => array('configuration'),
      'plObjectType'  => array('smallConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'rdns' => array(
        'name'  => _('LDAP tree'),
        'attrs' => array(
          new StringAttribute (
            _('Phone RDN'), _('Branch in which phones will be stored'),
            'fdPhoneRDN', TRUE,
            'ou=phones,ou=systems'
          ),
          new StringAttribute (
            _('Phone Macro RDN'), _('Branch in which phone macros will be stored'),
            'fdPhoneMacroRDN', TRUE,
            'ou=macros,ou=asterisk,ou=configs,ou=systems'
          ),
          new StringAttribute (
            _('Phone Conference RDN'), _('Branch in which phone conferences will be stored'),
            'fdPhoneConferenceRDN', TRUE,
            'ou=conferences,ou=asterisk,ou=configs,ou=systems'
          ),
        )
      ),
      'config' => array(
        'name'  => _('Miscellaneous'),
        'attrs' => array(
          new SetAttribute (
            new StringAttribute (
              _('SIP contexts'), _('Available SIP contexts'),
              'fdSipContexts', TRUE
            ),
            array('default','internal','external')
          ),
          new SetAttribute (
            new StringAttribute (
              _('Voicemail contexts'), _('Available voicemail contexts'),
              'fdVoicemailContexts', TRUE
            ),
            array('voicemail')
          ),
        )
      )
    );
  }
}
?>
