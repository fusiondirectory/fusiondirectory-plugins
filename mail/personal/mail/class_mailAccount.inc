<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hicker
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


/*!
  \brief   mail plugin
  \author  Fabian Hicker  <Fabian.Hickert@GONICUS.de>
  \version 2.6.2
  \date    03.12.2007

  This class provides the functionality to read and write all attributes
  relevant for gosaMailAccounts from/to the LDAP.
  It does syntax checking and displays the formulars required.
  Special handling like sieve or imap actions will be implemented
  by the mailMethods.

 */

class mailFlagsAttribute extends StringAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "", $regexp = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setVisible(FALSE);
  }

  function setValue ($value)
  {
    parent::setValue($value);
    $this->plugin->attributesInfo['section1']['attrs']['Flags1']->setValue(preg_match('/R/',$this->value));
    $this->plugin->attributesInfo['section1']['attrs']['Flags2']->setValue(preg_match('/S/',$this->value));
    $this->plugin->attributesInfo['section1']['attrs']['Flags3']->setValue(preg_match('/V/',$this->value));
    $this->plugin->attributesInfo['section1']['attrs']['Flags4']->setValue(preg_match('/I/',$this->value));
    $this->plugin->attributesInfo['section2']['attrs']['Flags5']->setValue(preg_match('/L/',$this->value));
  }

  function getValue()
  {
    return '['.$this->plugin->attributesInfo['section1']['attrs']['Flags1']->computeLdapValue().
      $this->plugin->attributesInfo['section1']['attrs']['Flags2']->computeLdapValue().
      $this->plugin->attributesInfo['section1']['attrs']['Flags3']->computeLdapValue().
      $this->plugin->attributesInfo['section1']['attrs']['Flags4']->computeLdapValue().
      $this->plugin->attributesInfo['section2']['attrs']['Flags5']->computeLdapValue().']';
  }
}


class mailAccount extends simplePlugin
{

/*
  var $attributes= array(
      "mail", "gosaMailServer","gosaMailQuota", "gosaMailMaxSize","gosaMailForwardingAddress",
      "gosaMailDeliveryMode", "gosaSpamSortLevel", "gosaSpamMailbox","gosaMailAlternateAddress",
      "gosaVacationStart","gosaVacationStop", "gosaVacationMessage", "gosaMailAlternateAddress",
      "gosaMailForwardingAddress");
*/
  var $displayHeader    = TRUE;
  var $objectclasses    = array("gosaMailAccount");

  static function plInfo()
  {
    return array(
      "plShortName"     => _("Mail"),
      "plDescription"   => _("Mail settings"),
      "plSelfModify"    => TRUE,
      "plDepends"       => array("user"),                     // This plugin depends on
      "plPriority"      => 4,                                 // Position in tabs
      "plSection"       => array("personal" => _("My account")),
      "plCategory"      => array("users"),
      "plObjectType"    => array("user"),
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
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('Primary address'),
            _('Primary mail address'),
            'mail',
            TRUE
          ),
          new SelectAttribute (
            _('Server'),
            _('Specify the mail server where the user will be hosted on'),
            'gosaMailServer',
            FALSE,
            array()
          ),
          new IntAttribute (
            _('Quota usage').'<br />'._('Quota size'),
            '',
            'gosaMailQuota',
            FALSE,
            FALSE,
            FALSE,
            12
          ),
          new SetAttribute (
            new MailAttribute (
              '<img src=plugins/mail/images/alternatemail.png /> '._('Alternative addresses'),
              _('List of alternative mail addresses'),
              'gosaMailAlternateAddress'
            )
          ),
          new BooleanAttribute (
            _('Use custom sieve script').' <b>('._('disables all Mail options!').')<b>',
            '',
            'sieve'
          ),
          new BooleanAttribute (
            _('No delivery to own mailbox'),
            _('Select if you want to forward mails without getting own copies of them'),
            'Flags1'
          ),
          new BooleanAttribute (
            _('Activate vacation message'),
            _('Select to automatically response with the vacation message defined below'),
            'Flags2'
          ),
          new DateAttribute (
          _('from'),
          '',
          'gosaVacationStart',
          FALSE,
          'U'
          ),
          new DateAttribute (
          _('till'),
          '',
          'gosaVacationStop',
          FALSE,
          'U'
          ),
          new TextAreaAttribute (
            _('Vacation message'), '',
            'gosaVacationMessage'
          ),
          new BooleanAttribute (
            _('Move mails tagged with spam level greater than'),
            '',
            'Flags3'
          ),
          new SelectAttribute (
            _('Choose spam level - smaller values are more sensitive'),'','gosaSpamSortLevel',FALSE,
            array(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20),4
          ),
          new SelectAttribute (
            _('to folder'),'','gosaSpamMailbox',FALSE,
            array('INBOX'),'INBOX'
          ),
          new BooleanAttribute (
            _('Move mails tagged with spam level greater than'),
            '',
            'Flags4'
          ),
          new IntAttribute (
            _('MB'),
            '',
            'gosaMailMaxSize',
            FALSE, FALSE, FALSE,
            50
          ),
          new MailAttribute2 (
              _('Forward messages to'),
              _(''),
              'gosaMailForwardingAddress'
          ),
          new mailFlagsAttribute ("","","gosaMailDeliveryMode")
        )
      ),
      'section2' => array (
         'name' => _('Advanced mail options'),
         'icon' => 'images/false.png',
         'attrs' => array (
            new BooleanAttribute (
            _('User is only allowed to send and receive local mails'),
            _('Select if user can only send and receive inside his own domain'),
            'Flags5'
          )
        )
      )
    );
  }



  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['Flags1']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['Flags2']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['Flags3']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['Flags4']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['Flags5']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['sieve']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['Flags3']->setManagedAttributes (
      array (
        'erase' => array (
          FALSE => array (
            'section1' => array(
              'gosaSpamMailbox','gosaSpamSortLevel'
            )
          )
        )
      )
    );
    $this->attributesInfo['section1']['attrs']['Flags4']->setManagedAttributes (
      array (
        'erase' => array (
          FALSE => array (
            'section1' => array(
              'gosaMailMaxSize'
            )
          )
        )
      )
    );
    $this->attributesInfo['section1']['attrs']['Flags2']->setManagedAttributes (
      array (
        'erase' => array (
          FALSE => array (
            'section1' => array(
              'gosaVacationStart','gosaVacationStop'
            )
          )
        )
      )
    );

    $this->attributesInfo['section1']['attrs']['sieve']->setManagedAttributes (
      array (
        'erase' => array (
          TRUE => array (
            'section1' => array(
              'gosaVacationStart','gosaVacationStop','gosaMailMaxSize','gosaSpamMailbox','gosaSpamSortLevel','gosaVacationMessage',
              'Flags1','Flags2','Flags3','Flags4'
            ),
            'section2' => array (
              'Flags5'
            )
          )
        )
      )
    );

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=goMailServer)",array('cn'));
    $server = array();
    while ($attrs = $ldap->fetch()) {
        $server[] = $attrs['cn'][0];
    }
    $this->attributesInfo['section1']['attrs']['gosaMailServer']->setChoices($server);
  }

}

?>
