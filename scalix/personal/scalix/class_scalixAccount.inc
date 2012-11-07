<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2006 Jörn Dreyer
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

/*! \brief   scalix plugin
  \author  Jörn Dreyer <gosa.jfd@butonic.de>,
           based on work by Cajus Pollmeier <pollmeier@gonicus.de>
  \author  Jeremy Williame <williamejeremy@hotmail.com>
  \version 0.01
  \date    20.10.2006

  This class provides the functionality to read and write all attributes
  relevant for scalixAccounts from/to the LDAP. It does syntax checking
  and displays the formulars required.
 */

class scalixAccount extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array("scalixUserClass");
  var $plHeadline     = "Scalix";
  var $plDescription  = "This save and read the scalix account data from ldap";

  static function plInfo()
  {
    return array(
      "plHeadline"    => _("Scalix"),
      "plShortName"   => _("Scalix"),
      "plDescription" => _("Scalix account"),
      "plIcon"        => _("plugins/scalix/images/plugin.png"),
      "plSelfModify"  => TRUE,
      "plCategory"    => array("users"),
      "plObjectType"  => array("user"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _('Generic'),
        'attrs' => array(
          new StringAttribute (
            _('Scalix mailnode'),
            _(''),
            'scalixMailnode',
            TRUE,
            ''
          ),
          new SelectAttribute (
            _('Scalix mailbox class'),
            _(''),
            'scalixMailboxClass',
            FALSE,
            array('LIMITED','FULL'),
            '',
            array('LIMITED','FULL')
          ),
          new SelectAttribute (
            _('Scalix server language'),
            _('Select the language of your scalix server'),
            'scalixServerLanguage',
            FALSE,
            array('ENGLISH','GERMAN'),
            '',
            array('ENGLISH','GERMAN')
          ),
          new BooleanAttribute (
            _('Scalix Administrator'),
            _('Select for admin capability'),
            'scalixAdministrator',
            FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Scalix Mailbox Administrator'),
            _('Select for mailbox admin capability'),
            'scalixMailboxAdministrator',
            FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Hide this user entry in Scalix'),
            _('Hide user entry from addressbook'),
            'scalixHideUserEntry',
            FALSE,
            FALSE
          ),
          new IntAttribute (
              _('Limit mailbox size')._(" (MB)"),
              _(''),
              'scalixLimitMailboxSize',
              FALSE,
              0,
              FALSE,
              ''
          ),
          new BooleanAttribute (
            _('Limit Outbound Mail'),
            _('As sanction on mailbox quota overuse, stop user from sending mail'),
            'scalixLimitOutboundMail',
            FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Limit Inbound Mail'),
            _('As sanction on mailbox quota overuse, stop user from receiving mail'),
            'scalixLimitInboundMail',
            FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Notify User'),
            _('As sanction on mailbox quota overuse, notify the user by eMail'),
            'scalixLimitNotifyUser',
            FALSE,
            FALSE
          ),
          new SetAttribute (
            new StringAttribute (
              _('List of scalix email addresses'),
              _(''),
              'scalixEmailAddress',
              FALSE,
              '',
              '',
              "/^[^@]*@[^@]*$/"
            )
          )
        )
      )
    );
  }
}

?>
