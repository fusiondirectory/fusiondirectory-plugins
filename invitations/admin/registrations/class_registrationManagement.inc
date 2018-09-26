<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018 FusionDirectory

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
  function renderCell(ListingEntry $entry)
  {
    $value = $this->getAttributeValue($entry);
    if ($value == '') {
      return '&nbsp;';
    } else {
      if ($this->attributes[0] == 'fdRegistrationState') {
        $value = RegistrationStateAttribute::renderValue($value);
      } elseif ($this->attributes[0] == 'fdRegistrationLastChange') {
        $value = RegistrationLastChangeAttribute::renderValue($value);
      } else {
        $value = htmlentities($value, ENT_COMPAT, 'UTF-8');
      }
      return '<a href="?plug='.$_GET['plug'].'&amp;PID='.$entry->getPid().'&amp;act=listing_edit_'.$entry->row.'" title="'.$entry->dn.'">'.$value.'</a>';
    }
  }
}

class registrationManagement extends management
{
  /* Deactivate copy/paste and snapshots */
  protected $skipCpHandler      = TRUE;
  public static $skipSnapshots  = TRUE;

  public static $columns = array(
    array('ObjectTypeColumn',   array()),
    array('LinkColumn',         array('attributes' => 'nameAttr',                 'label' => 'Email')),
    array('RegistrationColumn', array('attributes' => 'fdRegistrationState',      'label' => 'Status')),
    array('RegistrationColumn', array('attributes' => 'fdRegistrationLastChange', 'label' => 'Last change')),
    array('ActionsColumn',      array('label' => 'Actions')),
  );

  public $neededAttrs = array(
    'fdRegistrationUserDN'        => '1',
    'fdRegistrationInvitationDN'  => '1',
    'fdRegistrationState'         => '1',
  );

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Registrations'),
      'plTitle'       => _('Registrations management'),
      'plDescription' => _('Manage registrations'),
      'plIcon'        => 'geticon.php?context=types&icon=contact&size=48',
      'plSection'     => 'accounts',
      'plManages'     => array('registration'),

      'plProvidedAcls' => array()
    );
  }

  function __construct()
  {
    parent::__construct();

    $this->filter->addElement(new RegistrationStateFilterElement($this->filter));
  }

  protected function configureActions()
  {
    parent::configureActions();

    $this->registerAction(
      new Action(
        'accept', _('Accept'), 'geticon.php?context=actions&icon=add&size=16',
        '+', 'acceptRegistration',
        array('registration/registration/fdRegistrationState:w')
      )
    );
    $this->registerAction(
      new Action(
        'reject', _('Reject'), 'geticon.php?context=actions&icon=remove&size=16',
        '+', 'rejectRegistration',
        array('registration/registration/fdRegistrationState:w')
      )
    );
    $this->actions['accept']->setSeparator(TRUE);
    $this->actions['reject']->setEnableFunction(array($this, 'enableAcceptReject'));
    $this->actions['accept']->setEnableFunction(array($this, 'enableAcceptReject'));
  }

  /* !\brief Accept registration
   */
  function acceptRegistration(array $action)
  {
    global $config, $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Accept');

    foreach ($action['targets'] as $dn) {
      // Lock registration object
      if ($user = get_lock($dn)) {
        msg_dialog::display(
          _('Error'),
          sprintf(_('Cannot accept %s. It has been locked by %s.'), $dn, $user),
          ERROR_DIALOG
        );
        continue;
      }
      add_lock ($dn, $ui->dn);

      $entry = $this->listing->getEntry($dn);

      if (!$this->enableAcceptReject('accept', $entry)) {
        msg_dialog::display(
          _('Error'),
          sprintf(_('Cannot accept registration %s, it’s not in status "Filled".'), $dn),
          ERROR_DIALOG
        );
        del_lock($dn);
        continue;
      }

      // Lock user object
      if ($user = get_lock($entry['fdRegistrationUserDN'])) {
        msg_dialog::display(
          _('Error'),
          sprintf(_('Cannot move %s. It has been locked by %s.'), $entry['fdRegistrationUserDN'], $user),
          ERROR_DIALOG
        );
        del_lock($dn);
        continue;
      }
      add_lock ($entry['fdRegistrationUserDN'], $ui->dn);

      // TODO Détermine la date de peremption
      // Open the user
      $userTabObject = objects::open($entry['fdRegistrationUserDN'], 'user');
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
      del_lock($entry['fdRegistrationUserDN']);

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
      del_lock($dn);
    }
  }

  /* !\brief Reject registration
   */
  function rejectRegistration(array $action)
  {
    global $config, $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Reject');

    foreach ($action['targets'] as $dn) {
      if ($user = get_lock($dn)) {
        msg_dialog::display(
          _('Error'),
          sprintf(_('Cannot reject %s. It has been locked by %s.'), $dn, $user),
          ERROR_DIALOG
        );
        continue;
      }
      add_lock ($dn, $ui->dn);

      $entry = $this->listing->getEntry($dn);

      if (!$this->enableAcceptReject('reject', $entry)) {
        msg_dialog::display(
          _('Error'),
          sprintf(_('Cannot reject registration %s, it’s not in status "Filled".'), $dn),
          ERROR_DIALOG
        );
        del_lock($dn);
        continue;
      }

      // Delete the user
      if ($user = get_lock($entry['fdRegistrationUserDN'])) {
        msg_dialog::display(
          _('Error'),
          sprintf(_('Cannot delete %s. It has been locked by %s.'), $entry['fdRegistrationUserDN'], $user),
          ERROR_DIALOG
        );
        del_lock($dn);
        continue;
      }
      add_lock ($entry['fdRegistrationUserDN'], $ui->dn);

      // Delete the object
      $userTabObject = objects::open($entry['fdRegistrationUserDN'], 'user');
      $success = $userTabObject->delete();

      // Remove the lock for the user object
      del_lock($entry['fdRegistrationUserDN']);
      if ($success) {
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
      del_lock($dn);
    }
  }

  /*
   * \brief Whether accept and reject actions should be enabled for an entry
   */
  function enableAcceptReject($action, ListingEntry $entry = NULL)
  {
    if ($entry !== NULL) {
      /* For entries */
      return ($entry['fdRegistrationState'] == 'filled');
    } else {
      /* For action menu */
      return TRUE;
    }
  }

  protected function getFormFromRegistration(ListingEntry $entry)
  {
    $invitations = objects::ls(
      'invitation',
      array(
        'fdInvitationFormDN'  => 1
      ),
      $entry['fdRegistrationInvitationDN'],
      '', FALSE, 'base'
    );
    if (count($invitations) <= 0) {
      throw new FusionDirectoryException('Could not find invitation "'.$entry['fdRegistrationInvitationDN'].'"');
    } elseif (count($invitations) > 1) {
      throw new FusionDirectoryException('Found several invitations "'.$entry['fdRegistrationInvitationDN'].'"');
    }
    $invitation = reset($invitations);
    /* Search for the form object */
    $forms = objects::ls(
      'publicForm',
      array(
        'dn'                        => 'raw',
        'cn'                        => 1,
        'fdPublicFormTitle'         => 1,
        'fdPublicFormText'          => 1,
        'fdPublicFormFinalText'     => 1,
        'fdPublicFormTemplateType'  => 1,
        'fdPublicFormTemplateDN'    => 1,
        'fdPublicFormPrivate'       => 1,
        'fdPublicFormCreationBase'  => 1
      ),
      $invitation['fdInvitationFormDN'],
      '', FALSE, 'base'
    );
    if (count($forms) <= 0) {
      throw new FusionDirectoryException('Form "'.$invitation['fdInvitationFormDN'].'" could not be found');
    } elseif (count($forms) > 1) {
      throw new FusionDirectoryException('Found several forms named "'.$invitation['fdInvitationFormDN'].'"');
    }
    return reset($forms);
  }
}
