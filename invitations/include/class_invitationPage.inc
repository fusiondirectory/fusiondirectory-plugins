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

class invitationPage extends standAlonePage
{
  protected $token;

  function readLdapConfig ()
  {
    global $config;

    return TRUE;
  }

  function save_object ()
  {
    global $ui;

    if (!$this->activated) {
      return;
    }

    if (isset($_GET['token'])) {
      $this->token = $_GET['token'];
      static::checkToken($this->token);
    } else {
      throw new FusionDirectoryException(_('Token is missing from URL'));
    }
  }

  static function checkToken ($token)
  {
    if (is_array($token)) {
      $registration = $token;
    } else {
      $registrations = objects::ls(
        'registration',
        array(
          'dn'                          => 'raw',
          'fdRegistrationEmailAddress'  => 1,
          'fdRegistrationInvitationDN'  => 1,
          'fdRegistrationState'         => 1,
        ),
        NULL,
        '(fdRegistrationToken='.ldap_escape_f($token).')'
      );
      if (count($registrations) <= 0) {
        throw new FusionDirectoryException(sprintf(_('Could not find any invitation matching token "%s"'), $token));
      } elseif (count($registrations) > 1) {
        throw new FusionDirectoryException(sprintf(_('Found several invitations matching token "%s"'), $token));
      }
      $registration = reset($registrations);
      if (empty($registration['fdRegistrationInvitationDN'])) {
        throw new FusionDirectoryException(sprintf(_('The invitation matching token "%s" does not exist anymore'), $_GET['token']));
      }
    }
    if ($registration['fdRegistrationState'] != 'sent') {
      switch ($registration['fdRegistrationState']) {
        case 'filled':
          throw new FusionDirectoryException(_('This invitation has already been filled and is waiting approval'));
        case 'accepted':
          throw new FusionDirectoryException(_('This invitation has already been filled and approved'));
        case 'rejected':
          throw new FusionDirectoryException(_('This invitation has already been filled and rejected. Please contact your referent to get a new invitation if needed.'));
        default:
          throw new FusionDirectoryException(sprintf(_('This invitation is in unknown state "%s"'), $registration['fdRegistrationState']));
      }
    }
  }

  function execute ()
  {
    if (!$this->activated) {
      return;
    }

    $errorMessage = FALSE;

    try {
      $this->save_object();
    } catch (FusionDirectoryException $e) {
      $errorMessage = $e->getMessage();
    }

    $smarty = get_smarty();

    $this->assignSmartyVars();

    $smarty->append('css_files',    get_template_path('login.css'));
    $smarty->assign('title',        _('Invitation'));
    $smarty->display(get_template_path('headers.tpl'));

    $smarty->assign('errorMessage', $errorMessage);
    $smarty->assign('activated',    $this->activated);
    $smarty->assign('token',        $this->token);

    $smarty->display(get_template_path('invitation.tpl'));
  }
}
?>
