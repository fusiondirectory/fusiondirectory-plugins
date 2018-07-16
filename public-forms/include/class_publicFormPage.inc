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

  function readLdapConfig()
  {
    global $config;

    return TRUE;
  }

  function save_object()
  {
    if (!$this->activated) {
      return;
    }

    if (!isset($this->form) && isset($_GET['form'])) {
      /* Search for the form object */
      $forms = objects::ls('publicForm', array('dn' => 'raw', 'cn' => 1, 'fdPublicFormTitle' => 1, 'fdPublicFormText' => 1, 'fdPublicFormTemplateType' => 1, 'fdPublicFormTemplateDN' => 1), NULL, '(cn='.ldap_escape_f($_GET['form']).')');
      if (count($forms) <= 0) {
        throw new FusionDirectoryException('Form "'.$_GET['form'].'" could not be found');
      } elseif (count($forms) > 1) {
        throw new FusionDirectoryException('Several forms match the name "'.$_GET['form'].'"');
      }
      $this->form     = reset($forms);
      $this->template = new template($this->form['fdPublicFormTemplateType'], $this->form['fdPublicFormTemplateDN']);
    } elseif (!isset($this->form)) {
      throw new FusionDirectoryException('No form name given');
    }

    $this->template->save_object();

    //~ $success = TRUE;
    //~ /* Do we need to show error messages? */
    //~ if (count($this->message) != 0) {
      //~ /* Show error message and continue editing */
      //~ msg_dialog::displayChecks($this->message);
      //~ $success = FALSE;
    //~ }
  }


  function execute()
  {
    if (!$this->activated) {
      return;
    }

    $this->save_object();

    $smarty = get_smarty();

    $this->assignSmartyVars();

    $templateOutput = $this->template->execute();
    if ($this->template->dialogOpened()) {
      //~ TODO
      return $templateOutput;
    } else {
      $smarty->assign('template_dialog', $templateOutput);
    }

    $smarty->assign('form', $this->form);

    $smarty->append('css_files',    get_template_path('login.css'));
    $smarty->display(get_template_path('headers.tpl'));

    $smarty->assign('activated',      $this->activated);
    $smarty->display(get_template_path('public-form.tpl'));
  }
}
?>
