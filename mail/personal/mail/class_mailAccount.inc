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
  static private $flags = array(
    'flag_ownmailbox'     => 'advanced',
    'flag_vacation'       => 'options1',
    'flag_spamlevel'      => 'options2',
    'flag_spamsize'       => 'options2',
    'flag_localonly'      => 'advanced',
    'flag_sieveownscript' => 'advanced'
  );

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "", $regexp = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setVisible(FALSE);
  }

  function setValue ($value)
  {
    parent::setValue($value);
    foreach (self::$flags as $attr => $section) {
      $trueValue = $this->plugin->attributesInfo[$section]['attrs'][$attr]->trueValue;
      $this->plugin->attributesInfo[$section]['attrs'][$attr]->setValue(preg_match("/$trueValue/", $this->value));
    }
  }

  function getValue()
  {
    $value = '[';
    foreach (self::$flags as $attr => $section) {
      $value .= $this->plugin->attributesInfo[$section]['attrs'][$attr]->computeLdapValue();
    }
    $value .= ']';
    return $value;
  }
}

class MailQuotaAttribute extends IntAttribute
{
  private $quotaUsage;

  function setMailMethod($mailMethod)
  {
    /* Read quota */
    $this->value = $mailMethod->getQuota($this->value);
    $this->quotaUsage = $mailMethod->getQuotaUsage();
    if ($mailMethod->is_error()) {
      msg_dialog::display(_("Mail error"), sprintf(_("Cannot read quota settings: %s"),
            $mailMethod->get_error()), ERROR_DIALOG);
    }
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    parent::renderAttribute($attributes, $readOnly);
    $attributes[$this->getLdapName().'_usage'] = array(
      'htmlid'      => $this->getHtmlId().'_usage',
      'label'       => '{literal}'._('Quota usage').'{/literal}',
      'description' => _('Part of the quota which is used'),
      'input'       => mailMethod::quota_to_image($this->quotaUsage, $this->value),
    );
  }
}


class mailAccount extends simplePlugin
{
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
      'main' => array (
        'name'  => _('Generic'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('Primary address'), _('Primary mail address'),
            'mail', TRUE
          ),
          new SelectAttribute (
            _('Server'), _('Specify the mail server where the user will be hosted on'),
            'gosaMailServer', FALSE,
            array()
          ),
          new MailQuotaAttribute (
            _('Quota size'), '',
            'gosaMailQuota', FALSE,
            0, FALSE
          )
        )
      ),
      'other_addresses' => array(
        'name'  => _('Other addresses and redirections'),
        'icon'  => get_template_path('plugins/mail/images/alternatemail.png'),
        'attrs' => array(
          new SetAttribute (
            new MailAttribute (
              _('Alternative addresses'), _('List of alternative mail addresses'),
              'gosaMailAlternateAddress'
            )
          ),
          new MailsAttribute (
            _('Forward messages to'), _('Addresses to which messages should be forwarded'),
            'gosaMailForwardingAddress'
          )
        )
      ),
      'options1' => array(
        'name' => _('Vacation message'),
        'attrs' => array(
          new BooleanAttribute (
            _('Activate vacation message'),
            _('Select to automatically response with the vacation message defined below'),
            'flag_vacation', FALSE, FALSE, '',
            'V', ''
          ),
          new DateAttribute (
            _('from'), '',
            'gosaVacationStart', FALSE,
            'U'
          ),
          new DateAttribute (
            _('till'), '',
            'gosaVacationStop', FALSE,
            'U'
          ),
          new TextAreaAttribute (
            _('Vacation message'), '',
            'gosaVacationMessage'
          ),
        )
      ),
      'options2' => array(
        'name' => _('Spam options'),
        'attrs' => array(
          new BooleanAttribute (
            _('Move mails depending on spam level'), '',
            'flag_spamlevel', FALSE, FALSE, '',
            'S', ''
          ),
          new SelectAttribute (
            _('Minimum spam level'), _('Spam level above which a mail will be moved'),
            'gosaSpamSortLevel', FALSE,
            array(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20), 4
          ),
          new SelectAttribute (
            _('Move spam to folder'), _('Folder in which spam will be moved'),
            'gosaSpamMailbox', FALSE,
            array('INBOX')
          ),
          new BooleanAttribute (
            _('Reject mail depending on their size'), _('Messages bigger than the chosen size will be rejected'),
            'flag_spamsize', FALSE, FALSE, '',
            'R', ''
          ),
          new IntAttribute (
            _('Minimum size to reject a mail (MB)'), _('Minimum size to reject a mail (MB)'),
            'gosaMailMaxSize',
            FALSE, FALSE, FALSE,
            50
          ),
          new mailFlagsAttribute ('', '', 'gosaMailDeliveryMode')
        )
      ),
      'advanced' => array (
         'name' => _('Advanced mail options'),
         'icon' => 'images/false.png',
         'attrs' => array (
          new BooleanAttribute (
            _('Use custom sieve script').' <b>('._('disables all Mail options!').')<b>',
            _('Use your sieve script with personnal customization'),
            'flag_sieveownscript', FALSE, FALSE, '',
            'C', ''
          ),
          new BooleanAttribute (
            _('User is only allowed to send and receive local mails'),
            _('Select if user can only send and receive inside his own domain'),
            'flag_localonly', FALSE, FALSE, '',
            'L', ''
          ),
          new BooleanAttribute (
            _('No delivery to own mailbox'),
            _('Select if you want to forward mails without getting own copies of them'),
            'flag_ownmailbox', FALSE, FALSE, '',
            'I', ''
          ),
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

    $this->cn = $object->cn;

    /* Intialize the used mailMethod */
    $tmp = new mailMethod($config, $this);

    $this->mailMethod = $tmp->get_method();
    $this->mailMethod->fixAttributesOnLoad();

    $this->attributesInfo['main']['attrs']['gosaMailServer']->setChoices($this->mailMethod->getMailServers());
    $this->attributesInfo['options2']['attrs']['gosaSpamSortLevel']->setChoices($this->mailMethod->getSpamLevels());

    /* Initialize configured values */
    if ($this->is_account) {
      if ($this->mailMethod->connect() && $this->mailMethod->account_exists()) {

        /* Read quota */
        $this->attributesInfo['main']['attrs']['gosaMailQuota']->setMailMethod($this->mailMethod);

        /* Read mailboxes */
        $mailBoxes = $this->mailMethod->getMailboxList();
        $this->attributesInfo['options2']['attrs']['gosaSpamMailbox']->setChoices($mailBoxes);
        if ($this->mailMethod->is_error()) {
          msg_dialog::display(_("Mail error"), sprintf(_("Cannot get list of mailboxes: %s"),
                $this->mailMethod->get_error()), ERROR_DIALOG);
        }
      } elseif(!$this->mailMethod->is_connected()) {
        msg_dialog::display(_("Mail error"), sprintf(_("Mail method cannot connect: %s"),
              $this->mailMethod->get_error()), ERROR_DIALOG);
      } elseif(!$this->mailMethod->account_exists()) {
        msg_dialog::display(_("Mail error"), sprintf(_("Mailbox '%s' doesn't exists on mail server: %s"),
              $this->mailMethod->get_account_id(),$this->gosaMailServer), ERROR_DIALOG);
      }

      if ($this->initially_was_account) {
        $this->attributesInfo['main']['attrs']['mail']->setDisabled(!$this->mailMethod->isModifyableMail());
        $this->attributesInfo['main']['attrs']['gosaMailServer']->setDisabled(!$this->mailMethod->isModifyableServer());
      }
    }

    $this->attributesInfo['advanced']['attrs']['flag_ownmailbox']->setInLdap(FALSE);
    $this->attributesInfo['options1']['attrs']['flag_vacation']->setInLdap(FALSE);
    $this->attributesInfo['options2']['attrs']['flag_spamlevel']->setInLdap(FALSE);
    $this->attributesInfo['options2']['attrs']['flag_spamsize']->setInLdap(FALSE);
    $this->attributesInfo['advanced']['attrs']['flag_localonly']->setInLdap(FALSE);
    $this->attributesInfo['advanced']['attrs']['flag_sieveownscript']->setInLdap(FALSE);

    if (!$this->mailMethod->vacationRangeEnabled()) {
      unset($this->attributesInfo['options1']['attrs']['gosaVacationStart']);
      unset($this->attributesInfo['options1']['attrs']['gosaVacationStop']);
    } else {
      $this->attributesInfo['options1']['attrs']['flag_vacation']->setManagedAttributes (
        array (
          'erase' => array (
            FALSE => array (
              'gosaVacationStart','gosaVacationStop','gosaVacationMessage'
            )
          )
        )
      );
    }

    $this->attributesInfo['options2']['attrs']['flag_spamlevel']->setManagedAttributes (
      array (
        'erase' => array (
          FALSE => array (
            'gosaSpamMailbox','gosaSpamSortLevel'
          )
        )
      )
    );
    $this->attributesInfo['options2']['attrs']['flag_spamsize']->setManagedAttributes (
      array (
        'erase' => array (
          FALSE => array ('gosaMailMaxSize')
        )
      )
    );

    $this->attributesInfo['advanced']['attrs']['flag_sieveownscript']->setManagedAttributes (
      array (
        'erase' => array (
          TRUE => array (
            'flag_vacation',
            'gosaVacationStart','gosaVacationStop','gosaVacationMessage',
            'flag_spamlevel','flag_spamsize','gosaMailMaxSize',
            'gosaSpamMailbox','gosaSpamSortLevel',
            'flag_ownmailbox',
            'flag_localonly'
          )
        )
      )
    );
    /* sieve submit the form so that managed attributes are correctly updated */
    $this->attributesInfo['advanced']['attrs']['flag_sieveownscript']->setSubmitForm(TRUE);

    $this->updateAttributesValues();
  }

}

?>
