<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

class spamAssassinAccount extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('SpamAssassin'),
      'plDescription'   => _('Edit user\'s spam rules'),
      'plIcon'          => 'geticon.php?context=applications&icon=spamassassin&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=spamassassin&size=16',
      'plSelfModify'    => TRUE,
      'plObjectClass'   => ['fdSpamAssassinUser'],
      'plObjectType'    => ['user'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Spam rules'),
        'class' => ['fullwidth'],
        'attrs' => [
        //~ A user object will have multiple spamassassin attributes, one for each preference setting.
        //~ The attributes themselves should be stored in the database. A spamassassin LDAP attribute should be set to the name of a SpamAssassin configuration directive followed by the value for the directive, separated by a space.
          new SetAttribute(
            new CompositeAttribute(
              _('SpamAssassin user preferences settings'),
            'fdSpamAssassinRules',
              [
                new SelectAttribute(
                  _('Directive'),    _('The name of a SpamAssassin configuration directive'),
                  'fdSpamAssassinRules_directive', TRUE,
                  ['score', 'whitelist_from', 'required_score']
                ),
                new StringAttribute(
                  _('Value'), _('The value for the directive'),
                  'fdSpamAssassinRules_value'
                ),
              ],
              '/^(\S*) (.*)$/',
              '%s %s',
              '',
              _('SpamAssassin user rules')
            )
          )
        ]
      ]
    ];
  }
}
