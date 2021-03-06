<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019 FusionDirectory

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
 * \brief Column rendering registration state and last change
 */
class RegistrationColumn extends LinkColumn
{
  protected function renderSingleValue (ListingEntry $entry, string $value): string
  {
    if ($value == '') {
      return '&nbsp;';
    } else {
      if ($this->attributes[0] == 'fdRegistrationState') {
        $value = RegistrationStateAttribute::renderValue($value);
      } elseif ($this->attributes[0] == 'fdRegistrationLastChange') {
        $value = RegistrationLastChangeAttribute::renderValue($value);
      } elseif ($this->attributes[0] == 'fdRegistrationUserDN') {
        try {
          $value = objects::link($value, 'user', '', NULL, FALSE, FALSE);
        } catch (NonExistingLdapNodeException $e) {
          $value = htmlescape(_('invalid'));
        }
      } elseif ($this->attributes[0] == 'fdRegistrationInvitationDN') {
        try {
          $value = objects::link($value, 'invitation', '', NULL, FALSE, FALSE);
        } catch (NonExistingLdapNodeException $e) {
          $value = htmlescape(_('invalid'));
        }
      } else {
        $value = htmlescape($value);
      }
      return $this->renderLink($entry, $value);
    }
  }
}

class registrationManagement extends management
{
  /* Deactivate copy/paste and snapshots */
  protected $skipCpHandler      = TRUE;
  public static $skipSnapshots  = TRUE;

  public static $columns = [
    ['ObjectTypeColumn',   []],
    ['LinkColumn',         ['attributes' => 'nameAttr',                   'label' => 'Email']],
    ['RegistrationColumn', ['attributes' => 'fdRegistrationUserDN',       'label' => 'User object']],
    ['RegistrationColumn', ['attributes' => 'fdRegistrationState',        'label' => 'Status']],
    ['RegistrationColumn', ['attributes' => 'fdRegistrationLastChange',   'label' => 'Last change']],
    ['RegistrationColumn', ['attributes' => 'fdRegistrationInvitationDN', 'label' => 'Invitation']],
    ['ActionsColumn',      ['label' => 'Actions']],
  ];

  public $neededAttrs = [
    'fdRegistrationUserDN'        => '*',
    'fdRegistrationInvitationDN'  => '*',
    'fdRegistrationState'         => '*',
  ];

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Registrations'),
      'plTitle'       => _('Registrations management'),
      'plDescription' => _('Manage registrations'),
      'plIcon'        => 'geticon.php?context=types&icon=registration&size=48',
      'plSection'     => 'accounts',
      'plManages'     => ['registration'],
    ];
  }

  function __construct ()
  {
    parent::__construct();

    $this->filter->addElement(new RegistrationStateFilterElement($this->filter));
  }

  protected function configureActions ()
  {
    parent::configureActions();

    $this->registerAction(
      new Action(
        'accept', _('Accept'), 'geticon.php?context=actions&icon=accept&size=16',
        '+', 'acceptRegistration',
        ['registration/registration/fdRegistrationState:w']
      )
    );
    $this->registerAction(
      new Action(
        'reject', _('Reject'), 'geticon.php?context=actions&icon=reject&size=16',
        '+', 'rejectRegistration',
        ['registration/registration/fdRegistrationState:w']
      )
    );
    $this->actions['accept']->setSeparator(TRUE);
    $this->actions['reject']->setEnableFunction([$this, 'enableAcceptReject']);
    $this->actions['accept']->setEnableFunction([$this, 'enableAcceptReject']);
  }

  /* !\brief Accept registration
   */
  function acceptRegistration (array $action)
  {
    global $config, $ui;
    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Accept');

    foreach ($action['targets'] as $dn) {
      // Lock registration object
      if (!empty($locks = Lock::get($dn))) {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Cannot accept %s. It has been locked by %s.'),
            $dn,
            $locks[0]->userDn
          ))
        );
        $error->display();
        continue;
      }
      Lock::add($dn);

      $entry = $this->listing->getEntry($dn);

      if (!$this->enableAcceptReject('accept', $entry)) {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Cannot accept registration %s, it\'s not in status "Filled".'),
            $dn
          ))
        );
        $error->display();
        Lock::deleteByObject($dn);
        continue;
      }

      // Lock user object
      if (!empty($locks = Lock::get($entry['fdRegistrationUserDN'][0]))) {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Cannot move %s. It has been locked by %s.'),
            $entry['fdRegistrationUserDN'][0],
            $locks[0]->userDn
          ))
        );
        $error->display();
        Lock::deleteByObject($dn);
        continue;
      }
      Lock::add($entry['fdRegistrationUserDN'][0]);

      // TODO Détermine la date de peremption
      // Open the user
      $userTabObject = objects::open($entry['fdRegistrationUserDN'][0], 'user');
      // Move it with the other users
      $form = $this->getFormFromRegistration($entry);
      $userTabObject->getBaseObject()->base = preg_replace(
        '/'.preg_quote($form['fdPublicFormCreationBase'], '$/').'/',
        $config->current['BASE'],
        $userTabObject->getBaseObject()->base
      );
      // Deactivate registration tab
      $userTabObject->by_object['userRegistration']->is_account = FALSE;
      $errors = $userTabObject->save();

      // Remove the lock for the user object
      Lock::deleteByObject($entry['fdRegistrationUserDN'][0]);

      // Unlock the user password if needed
      $pwds = objects::ls('user', 'userPassword', $userTabObject->dn, '(userPassword=*)', FALSE, 'base');
      if (!empty($pwds)) {
        $pwd    = reset($pwds);
        $method = passwordMethod::get_method($pwd, $entry['fdRegistrationUserDN'][0]);
        if (
            ($method instanceOf passwordMethod) &&
            $method->is_lockable() &&
            $method->is_locked($entry['fdRegistrationUserDN'][0])
          ) {
          $success = $method->unlock_account($entry['fdRegistrationUserDN'][0]);

          // Check if everything went fine.
          if (!$success) {
            $hn = $method->get_hash_name();
            if (is_array($hn)) {
              $hn = $hn[0];
            }
            $error = new FusionDirectoryError(
              htmlescape(sprintf(
                _('Unlocking failed using password method "%s". Account "%s" has not been unlocked!'),
                $hn,
                $entry['fdRegistrationUserDN'][0]
              ))
            );
            $error->display();
          }
        }
      }

      if (empty($errors)) {
        // Upon success, update registration object status and history
        $tabObject  = objects::open($dn, 'registration');
        $baseObject = $tabObject->getBaseObject();

        $baseObject->fdRegistrationUserDN     = $userTabObject->dn;
        $baseObject->fdRegistrationState      = 'accepted';
        $baseObject->fdRegistrationLastChange = date('c').'|'.$ui->dn;

        $messages = $tabObject->save();
        msg_dialog::displayChecks($messages);
      } else {
        msg_dialog::displayChecks($errors);
      }
      // Remove the lock for the registration object
      Lock::deleteByObject($dn);
    }
  }

  /* !\brief Reject registration
   */
  function rejectRegistration (array $action)
  {
    global $config, $ui;
    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Reject');

    foreach ($action['targets'] as $dn) {
      if (!empty($locks = Lock::get($dn))) {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Cannot reject %s. It has been locked by %s.'),
            $dn,
            $locks[0]->userDn
          ))
        );
        $error->display();
        continue;
      }
      Lock::add($dn);

      $entry = $this->listing->getEntry($dn);

      if (!$this->enableAcceptReject('reject', $entry)) {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Cannot reject registration %s, it\'s not in status "Filled".'),
            $dn
          ))
        );
        $error->display();
        Lock::deleteByObject($dn);
        continue;
      }

      // Delete the user
      if (!empty($locks = Lock::get($entry['fdRegistrationUserDN'][0]))) {
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Cannot delete %s. It has been locked by %s.'),
            $entry['fdRegistrationUserDN'][0],
            $locks[0]->userDn
          ))
        );
        $error->display();
        Lock::deleteByObject($dn);
        continue;
      }
      Lock::add($entry['fdRegistrationUserDN'][0]);

      // Delete the object
      $errors = objects::delete($entry['fdRegistrationUserDN'][0], 'user');
      msg_dialog::displayChecks($errors);

      // Remove the lock for the user object
      Lock::deleteByObject($entry['fdRegistrationUserDN'][0]);
      if (empty($errors)) {
        // Upon success, update registration object status and history
        $tabObject  = objects::open($dn, 'registration');
        $baseObject = $tabObject->getBaseObject();

        $baseObject->fdRegistrationUserDN     = '';
        $baseObject->fdRegistrationState      = 'rejected';
        $baseObject->fdRegistrationLastChange = date('c').'|'.$ui->dn;

        $messages = $tabObject->save();
        msg_dialog::displayChecks($messages);
      }
      // Remove the lock for the registration object
      Lock::deleteByObject($dn);
    }
  }

  /*
   * \brief Whether accept and reject actions should be enabled for an entry
   */
  function enableAcceptReject ($action, ListingEntry $entry = NULL): bool
  {
    if ($entry !== NULL) {
      /* For entries */
      return ($entry['fdRegistrationState'][0] == 'filled');
    } else {
      /* For action menu */
      return TRUE;
    }
  }

  protected function getFormFromRegistration (ListingEntry $entry): array
  {
    $invitations = objects::ls(
      'invitation',
      [
        'fdInvitationFormDN'  => 1
      ],
      $entry['fdRegistrationInvitationDN'][0],
      '', FALSE, 'base'
    );
    if (count($invitations) <= 0) {
      throw new FusionDirectoryException(sprintf(_('Could not find invitation "%s"'), $entry['fdRegistrationInvitationDN'][0]));
    } elseif (count($invitations) > 1) {
      throw new FusionDirectoryException(sprintf(_('Found several invitations "%s"'), $entry['fdRegistrationInvitationDN'][0]));
    }
    $invitation = reset($invitations);
    /* Search for the form object */
    $forms = objects::ls(
      'publicForm',
      [
        'dn'                        => 'raw',
        'cn'                        => 1,
        'fdPublicFormTitle'         => 1,
        'fdPublicFormText'          => 1,
        'fdPublicFormFinalText'     => 1,
        'fdPublicFormTemplateType'  => 1,
        'fdPublicFormTemplateDN'    => 1,
        'fdPublicFormPrivate'       => 1,
        'fdPublicFormCreationBase'  => 1
      ],
      $invitation['fdInvitationFormDN'],
      '', FALSE, 'base'
    );
    if (count($forms) <= 0) {
      throw new FusionDirectoryException(sprintf(_('Form "%s" could not be found'), $invitation['fdInvitationFormDN']));
    } elseif (count($forms) > 1) {
      throw new FusionDirectoryException(sprintf(_('Found several forms named "%s"'), $invitation['fdInvitationFormDN']));
    }
    return reset($forms);
  }
}
