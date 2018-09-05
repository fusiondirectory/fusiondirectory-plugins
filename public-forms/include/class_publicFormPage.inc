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
  protected $template;
  protected $done;
  // TODO fill this var by link/GET/LLNG
  protected $registrationToken;

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

    if (!isset($this->form) && isset($_GET['form'])) {
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
          'fdPublicFormTemplateDN'    => 1
        ),
        NULL,
        '(cn='.ldap_escape_f($_GET['form']).')'
      );
      if (count($forms) <= 0) {
        throw new FusionDirectoryException('Form "'.$_GET['form'].'" could not be found');
      } elseif (count($forms) > 1) {
        throw new FusionDirectoryException('Several forms match the name "'.$_GET['form'].'"');
      }
      $this->form     = reset($forms);
      $this->template = new template($this->form['fdPublicFormTemplateType'], $this->form['fdPublicFormTemplateDN']);
      /* TODO this is only for POC */
      $this->template->alterAttributes(
        ['user' => ['telephoneNumber']],
        ['user' => ['mobile']],
        ['user' => ['base']]
      );
    } elseif (!isset($this->form)) {
      throw new FusionDirectoryException('No form name given');
    }

    $this->template->save_object();

    if (isset($_POST['form_submit'])) {
      $userTabObject  = $this->template->apply();
      $errors         = $userTabObject->save();
      if (empty($errors)) {
        $this->done = TRUE;
        if (isset($this->registrationToken)) {
          $registrations = objects::ls('registration', NULL, NULL, '(fdRegistrationToken='.$this->registrationToken.')');
          if (count($registrations) == 1) {
            $tabObject  = objects::open(key($registrations), 'registration');
            $baseObject = $tabObject->getBaseObject();

            $baseObject->fdRegistrationUserDN     = $userTabObject->dn;
            $baseObject->fdRegistrationState      = 'filled';
            $baseObject->fdRegistrationLastChange = date('c').':'.$ui->dn;

            $messages = $tabObject->save();
            msg_dialog::displayChecks($messages);
          }
        }
      } else {
        msg_dialog::displayChecks($errors);
        // TODO - Find a way to refill previous values in the fields
        $this->template->reset();
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
