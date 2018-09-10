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

class publicFormPage extends standAlonePage
{
  protected $form;
  protected $registration;
  protected $invitation;
  protected $template;
  protected $done;

  function readLdapConfig()
  {
    global $config;

    return TRUE;
  }

  function save_object()
  {
    global $ui;

    if (!$this->activated) {
      return;
    }

    if ($this->done) {
      return;
    }

    if (!isset($this->form)) {
      if (isset($_GET['token'])) {
        if (isset($_GET['form'])) {
          throw new FusionDirectoryException('Invalid link, both token and form are provided');
        }
        $registrations = objects::ls(
          'registration',
          array(
            'dn'                          => 'raw',
            'fdRegistrationInvitationDN'  => 1
          ),
          NULL,
          '(fdRegistrationToken='.ldap_escape_f($_GET['token']).')'
        );
        if (count($registrations) <= 0) {
          throw new FusionDirectoryException('Could not find any invitation matching token "'.$_GET['token'].'"');
        } elseif (count($registrations) > 1) {
          throw new FusionDirectoryException('Found several invitations matching token "'.$_GET['token'].'"');
        }
        $this->registration = reset($registrations);
        $invitations = objects::ls(
          'invitation',
          array(
            'dn'                  => 'raw',
            'fdInvitationFormDN'  => 1
          ),
          $this->registration['fdRegistrationInvitationDN'],
          '', FALSE, 'base'
        );
        if (count($invitations) <= 0) {
          throw new FusionDirectoryException('Could not find any invitation matching registration with token "'.$_GET['token'].'"');
        } elseif (count($invitations) > 1) {
          throw new FusionDirectoryException('Found several invitations matching token "'.$_GET['token'].'"');
        }
        $this->invitation = reset($registrations);
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
            'fdPublicFormPrivate'       => 1
          ),
          $this->invitation['fdInvitationFormDN'],
          '', FALSE, 'base'
        );
      } elseif (isset($_GET['form'])) {
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
            'fdPublicFormPrivate'       => 1
          ),
          NULL,
          '(cn='.ldap_escape_f($_GET['form']).')'
        );
      } else {
        throw new FusionDirectoryException('Form name or token missing from URL');
      }
      if (count($forms) <= 0) {
        throw new FusionDirectoryException('Form "'.$_GET['form'].'" could not be found');
      } elseif (count($forms) > 1) {
        throw new FusionDirectoryException('Found several forms named "'.$_GET['form'].'"');
      }
      $this->form = reset($forms);
      if (!isset($this->registration) && isset($this->form['fdPublicFormPrivate']) && ($this->form['fdPublicFormPrivate'] == 'TRUE')) {
        // Private form and no token
        throw new FusionDirectoryException('Form "'.$this->form['cn'].'" is private and requires an invitation');
      }
      $this->template = new template($this->form['fdPublicFormTemplateType'], $this->form['fdPublicFormTemplateDN']);
      /* TODO this is only for POC */
      $this->template->alterAttributes(
        ['user' => ['telephoneNumber']],
        ['user' => ['mobile']],
        ['user' => ['base']]
      );
      /* TODO Remplir les champs depuis les headers HTTP */
      /* TODO Remplir la base en fonction de la base de l’invitation */
    }

    $this->template->save_object();

    if (isset($_POST['form_submit'])) {
      // Save values for later
      $values = $this->template->getValues();
      // Apply the template and try to save the object
      $userTabObject  = $this->template->apply();
      $errors         = $userTabObject->save();
      if (empty($errors)) {
        $this->done = TRUE;
        if (isset($this->registration)) {
          $tabObject  = objects::open($this->registration['dn'], 'registration');
          $baseObject = $tabObject->getBaseObject();

          $baseObject->fdRegistrationUserDN     = $userTabObject->dn;
          $baseObject->fdRegistrationState      = 'filled';
          $baseObject->fdRegistrationLastChange = date('c').':'.$ui->dn;

          $messages = $tabObject->save();
          msg_dialog::displayChecks($messages);
        }
      } else {
        msg_dialog::displayChecks($errors);
        $this->template->reset();
        /* After the reset we need to call alterAttributes again */
        /* TODO this is only for POC */
        $this->template->alterAttributes(
          ['user' => ['telephoneNumber']],
          ['user' => ['mobile']],
          ['user' => ['base']]
        );
        // This fill the saved values into the resetted template
        $this->template->setValues($values);
      }
    }
  }


  function execute()
  {
    if (!$this->activated) {
      return;
    }

    $this->save_object();

    $smarty = get_smarty();

    $this->assignSmartyVars();

    if (!$this->done) {
      $templateOutput = $this->template->execute();
      if ($this->template->dialogOpened()) {
        //~ TODO dialogs should be disabled for public forms
        $smarty->assign('template_dialog', $templateOutput);
      } else {
        $smarty->assign('template_dialog', $templateOutput);
      }
    }

    $smarty->assign('done', $this->done);

    $smarty->assign('form', $this->form);

    $smarty->append('css_files',    get_template_path('login.css'));
    $smarty->display(get_template_path('headers.tpl'));

    $smarty->assign('activated',      $this->activated);
    $smarty->display(get_template_path('public-form.tpl'));
  }
}
?>
