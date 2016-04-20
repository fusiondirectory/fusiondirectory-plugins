<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hicker
  Copyright (C) 2011-2016  FusionDirectory

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

  This class provides the functionality to read and write all attributes
  relevant for mail accounts from/to the LDAP.

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
            'gosaMailServer', FALSE,
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
      'advanced' => array (
         'name' => _('Advanced mail options'),
         'attrs' => array (
          new FlagsAttribute ('gosaMailDeliveryMode', array(
            'flag_ownmailbox',
            'flag_vacation',
            'flag_localonly'
          )),
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

    if (isset($object->parent)) {
      $this->parent = $object->parent;
    }

    $this->attributesAccess['gosaMailServer']->setChoices(array_keys(mailMethod::getMailServers()));
    $this->attributesAccess['gosaMailServer']->setSubmitForm('mailServerChanged');
    $this->mailServerChanged();

    $this->attributesAccess['flag_ownmailbox']->setInLdap(FALSE);
    $this->attributesAccess['flag_vacation']->setInLdap(FALSE);
    $this->attributesAccess['flag_localonly']->setInLdap(FALSE);
  }

  /* We need $mailAccount->uid to return the uid for mailMethod */
  public function __get($name)
  {
    if (in_array($name, array('cn','uid')) && isset($this->parent)) {
      return $this->parent->getBaseObject()->$name;
    } elseif (in_array($name, array('uidNumber','gidNumber')) && isset($this->parent)) {
      return $this->parent->by_object['posixAccount']->$name;
    } else {
      /* Calling default behaviour */
      return parent::__get($name);
    }
  }

  public function __isset($name)
  {
    if (in_array($name, array('cn','uid')) && isset($this->parent)) {
      return isset($this->parent->getBaseObject()->$name);
    } elseif (in_array($name, array('uidNumber','gidNumber')) && isset($this->parent)) {
      return isset($this->parent->by_object['posixAccount']->$name);
    } else {
      /* Calling default behaviour */
      return parent::__isset($name);
    }
  }

  public function mailServerChanged()
  {
    /* Intialize the used mailMethod */
    if ($this->gosaMailServer == '') {
      $method = 'mailMethod';
    } else {
      $method = mailMethod::getServerMethod($this->gosaMailServer);
    }
    $this->mailMethod = mailMethod::getInstance($method, $this);
    $this->mailMethod->fixAttributesOnLoad();

    /* Initialize configured values */
    if ($this->is_account) {
      if ($this->mailMethod->connect() && $this->mailMethod->account_exists()) {
        /* Read quota */
        $this->attributesAccess['gosaMailQuota']->setMailMethod($this->mailMethod);
      } elseif (!$this->mailMethod->is_connected()) {
        msg_dialog::display(_('Mail error'), sprintf(_('Mail method cannot connect: %s'),
              $this->mailMethod->get_error()), ERROR_DIALOG);
      } elseif (!$this->mailMethod->account_exists()) {
        msg_dialog::display(_('Mail error'), sprintf(_('Mailbox "%s" doesn\'t exists on mail server: %s'),
              $this->mailMethod->get_account_id(), $this->gosaMailServer), ERROR_DIALOG);
      }

      if ($this->initially_was_account) {
        $this->attributesAccess['mail']->setDisabled(!$this->mailMethod->isModifyableMail());
        $this->attributesAccess['gosaMailServer']->setDisabled(!$this->mailMethod->isModifyableServer());
      }
    }

    if (!$this->mailMethod->vacationRangeEnabled()) {
      $this->attributesAccess['gosaVacationStart']->setDisabled(TRUE);
      $this->attributesAccess['gosaVacationStart']->setVisible(FALSE);
      $this->attributesAccess['gosaVacationStart']->setValue('');
      $this->attributesAccess['gosaVacationStop']->setDisabled(TRUE);
      $this->attributesAccess['gosaVacationStop']->setVisible(FALSE);
      $this->attributesAccess['gosaVacationStop']->setValue('');
      $this->attributesAccess['flag_vacation']->setManagedAttributes (
        array (
          'erase' => array (
            FALSE => array (
              'gosaVacationMessage'
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

  function post_remove()
  {
    /* Let the mailMethod remove this mailbox */
    if (!$this->is_template) {
      if (!$this->mailMethod->connect()) {
        msg_dialog::display(_('Mail error'), sprintf(_('Cannot remove mailbox, mail method cannot connect: %s'),
              $this->mailMethod->get_error()), ERROR_DIALOG);
      } else {
        if (!$this->mailMethod->deleteMailbox()) {
          msg_dialog::display(_('Mail error'), sprintf(_('Cannot remove mailbox: %s'),
                $this->mailMethod->get_error()), ERROR_DIALOG);
        }
      }
    }
    $this->mailMethod->disconnect();
    parent::post_remove();
  }
}

?>
