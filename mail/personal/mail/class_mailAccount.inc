<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hicker
  Copyright (C) 2011-2015  FusionDirectory

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
  var $objectclasses    = array('gosaMailAccount');

  /* Used by the mail method */
  var $cn;
  var $uid;
  var $uidNumber;
  var $gidNumber;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Mail'),
      'plDescription'   => _('Mail settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=internet-mail&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=internet-mail&size=16',
      'plSelfModify'    => TRUE,
      'plPriority'      => 4,
      'plObjectType'    => array('user'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
  *  \brief The main function : information about attributes
  */
  static function getAttributesInfo ()
  {
    return array (
      'main' => array (
        'name'  => _('Mail account'),
        'attrs' => array (
          new MailAttribute (
            _('Primary address'), _('Primary mail address'),
            'mail', TRUE
          ),
          new SelectAttribute (
            _('Server'), _('Specify the mail server where the user will be hosted on'),
            'gosaMailServer', TRUE,
            array()
          ),
          new MailQuotaAttribute (
            _('Quota size'), 'Define quota size in MB',
            'gosaMailQuota', FALSE,
            0, FALSE
          )
        )
      ),
      'other_addresses' => array(
        'name'  => _('Other addresses and redirections'),
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
            _('from'), 'Starting date for vacation message',
            'gosaVacationStart', FALSE,
            'U'
          ),
          new DateAttribute (
            _('till'), 'Ending date for vacation message',
            'gosaVacationStop', FALSE,
            'U'
          ),
          new TextAreaAttribute (
            _('Vacation message'), 'The message you wish be sent during your absence',
            'gosaVacationMessage'
          ),
        )
      ),
      'options2' => array(
        'name' => _('Spam options'),
        'attrs' => array(
          new BooleanAttribute (
            _('Move mails depending on spam level'), 'Check if you want to move mails depending on spam level',
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
          new FlagsAttribute ('gosaMailDeliveryMode', array(
            'flag_ownmailbox',
            'flag_vacation',
            'flag_spamlevel',
            'flag_spamsize',
            'flag_localonly',
            'flag_sieveownscript'
          ))
        )
      ),
      'advanced' => array (
         'name' => _('Advanced mail options'),
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

    foreach (array('cn','uid','uidNumber','gidNumber') as $var) {
      if (isset($object->$var)) {
        $this->$var = $object->$var;
      } elseif (isset($this->attrs[$var])) {
        $this->$var = $this->attrs[$var][0];
      }
    }

    /* Intialize the used mailMethod */
    $tmp = new mailMethod($config, $this);

    $this->mailMethod = $tmp->get_method();
    $this->mailMethod->fixAttributesOnLoad();

    $this->attributesAccess['gosaMailServer']->setChoices($this->mailMethod->getMailServers());
    $this->attributesAccess['gosaSpamSortLevel']->setChoices($this->mailMethod->getSpamLevels());

    /* Initialize configured values */
    if ($this->is_account) {
      if ($this->mailMethod->connect() && $this->mailMethod->account_exists()) {

        /* Read quota */
        $this->attributesAccess['gosaMailQuota']->setMailMethod($this->mailMethod);

        /* Read mailboxes */
        $mailBoxes = $this->mailMethod->getMailboxList();
        $this->attributesAccess['gosaSpamMailbox']->setChoices($mailBoxes);
        if ($this->mailMethod->is_error()) {
          msg_dialog::display(_("Mail error"), sprintf(_("Cannot get list of mailboxes: %s"),
                $this->mailMethod->get_error()), ERROR_DIALOG);
        }
      } elseif (!$this->mailMethod->is_connected()) {
        msg_dialog::display(_("Mail error"), sprintf(_("Mail method cannot connect: %s"),
              $this->mailMethod->get_error()), ERROR_DIALOG);
      } elseif (!$this->mailMethod->account_exists()) {
        msg_dialog::display(_("Mail error"), sprintf(_("Mailbox '%s' doesn't exists on mail server: %s"),
              $this->mailMethod->get_account_id(), $this->gosaMailServer), ERROR_DIALOG);
      }

      if ($this->initially_was_account) {
        $this->attributesAccess['mail']->setDisabled(!$this->mailMethod->isModifyableMail());
        $this->attributesAccess['gosaMailServer']->setDisabled(!$this->mailMethod->isModifyableServer());
      }
    }

    $this->attributesAccess['flag_ownmailbox']->setInLdap(FALSE);
    $this->attributesAccess['flag_vacation']->setInLdap(FALSE);
    $this->attributesAccess['flag_spamlevel']->setInLdap(FALSE);
    $this->attributesAccess['flag_spamsize']->setInLdap(FALSE);
    $this->attributesAccess['flag_localonly']->setInLdap(FALSE);
    $this->attributesAccess['flag_sieveownscript']->setInLdap(FALSE);

    if (!$this->mailMethod->vacationRangeEnabled()) {
      $this->attributesAccess['gosaVacationStart']->setDisabled(TRUE);
      $this->attributesAccess['gosaVacationStart']->setVisible(FALSE);
      $this->attributesAccess['gosaVacationStop']->setDisabled(TRUE);
      $this->attributesAccess['gosaVacationStop']->setVisible(FALSE);
      $this->attributesAccess['flag_vacation']->setManagedAttributes (
        array (
          'erase' => array (
            FALSE => array (
              'gosaVacationMessage'
            )
          )
        )
      );
      $this->attributesAccess['flag_sieveownscript']->setManagedAttributes (
        array (
          'erase' => array (
            TRUE => array (
              'flag_vacation',
              'gosaVacationMessage',
              'flag_spamlevel','flag_spamsize','gosaMailMaxSize',
              'gosaSpamMailbox','gosaSpamSortLevel',
              'flag_ownmailbox',
              'flag_localonly'
            )
          )
        )
      );
    } else {
      $this->attributesAccess['flag_vacation']->setManagedAttributes (
        array (
          'erase' => array (
            FALSE => array (
              'gosaVacationStart','gosaVacationStop','gosaVacationMessage'
            )
          )
        )
      );
      $this->attributesAccess['flag_sieveownscript']->setManagedAttributes (
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
    }

    $this->attributesAccess['flag_spamlevel']->setManagedAttributes (
      array (
        'erase' => array (
          FALSE => array (
            'gosaSpamMailbox','gosaSpamSortLevel'
          )
        )
      )
    );
    $this->attributesAccess['flag_spamsize']->setManagedAttributes (
      array (
        'erase' => array (
          FALSE => array ('gosaMailMaxSize')
        )
      )
    );

    /* sieve submit the form so that managed attributes are correctly updated */
    $this->attributesAccess['flag_sieveownscript']->setSubmitForm(TRUE);

    $this->updateAttributesValues();
  }

  /* We need $mailAccount->uid to return the uid for mailMethod */
  public function __get($name)
  {
    if (($name == 'uid') && isset($this->parent)) {
      return $this->parent->getBaseObject()->uid;
    } else {
      /* Calling default behaviour */
      return parent::__get($name);
    }
  }

  public function save()
  {
    parent::save();

    /* Only do IMAP actions if we are not a template */
    if (!$this->is_template) {
      $this->mailMethod->connect();
      if (!$this->mailMethod->is_connected()) {
        msg_dialog::display(_("Mail error"), sprintf(_("Mail method cannot connect: %s"),
              $this->mailMethod->get_error()), ERROR_DIALOG);
      } else {
        if (!$this->mailMethod->updateMailbox()) {
          msg_dialog::display(_("Mail error"), sprintf(_("Cannot update mailbox: %s"),
                $this->mailMethod->get_error()), ERROR_DIALOG);
        }
        if (!$this->mailMethod->setQuota($this->gosaMailQuota)) {
          msg_dialog::display(_("Mail error"), sprintf(_("Cannot write quota settings: %s"),
                $this->mailMethod->get_error()), ERROR_DIALOG);
        }

        if (!is_integer(strpos($this->gosaMailDeliveryMode, "C"))) {

          /* Do not write sieve settings if this account is new and
             doesn't seem to exist. */
          if (!$this->initially_was_account && !$this->mailMethod->account_exists()) {
            @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,
                "Skipping sieve settings, the account doesn't seem to be created already.</b>", "");
          } else {
            if (!$this->mailMethod->saveSieveSettings()) {
              msg_dialog::display(_("Mail error saving sieve settings"), $this->mailMethod->get_error(), ERROR_DIALOG);
            }
          }
        } else {
          if ($this->sieve_management) {
            @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,
                "User uses an own sieve script, skipping sieve update.".$str."</b>", "");
            $this->sieve_management->save();
          }
        }
      }
    }
    $this->mailMethod->disconnect();
  }
}

?>
