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
  protected $request;
  protected $errorMessage;
  protected $prefill = FALSE;

  protected $providedAttributes;
  protected $providedMultiValueAttributes;
  protected $alteredAttributes;

  function __construct($interactive = TRUE)
  {
    parent::__construct($interactive);
    $this->request = $_SERVER['REQUEST_URI'];
  }

  function reset()
  {
    unset($this->form);
    unset($this->registration);
    unset($this->invitation);
    unset($this->template);
    $this->done = FALSE;
  }

  function init()
  {
    parent::init();
    if ($_SERVER['REQUEST_URI'] != $this->request) {
      /* Reset page if we’re called with a different token or form */
      $this->reset();
      $this->request = $_SERVER['REQUEST_URI'];
    }
  }

  function readLdapConfig()
  {
    global $config;

    return TRUE;
  }

  protected function fetchForm()
  {
    if (isset($_GET['token'])) {
      if (isset($_GET['form'])) {
        throw new FusionDirectoryException(_('Invalid link, both token and form are provided'));
      }
      $registrations = objects::ls(
        'registration',
        array(
          'dn'                          => 'raw',
          'fdRegistrationEmailAddress'  => 1,
          'fdRegistrationInvitationDN'  => 1,
          'fdRegistrationState'         => 1,
        ),
        NULL,
        '(fdRegistrationToken='.ldap_escape_f($_GET['token']).')'
      );
      if (count($registrations) <= 0) {
        throw new FusionDirectoryException(sprintf(_('Could not find any invitation matching token "%s"'), $_GET['token']));
      } elseif (count($registrations) > 1) {
        throw new FusionDirectoryException(sprintf(_('Found several invitations matching token "%s"'), $_GET['token']));
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
        throw new FusionDirectoryException(sprintf(_('Could not find any invitation matching registration with token "%s"'), $_GET['token']));
      } elseif (count($invitations) > 1) {
        throw new FusionDirectoryException(sprintf(_('Found several invitations matching token "%s"'), $_GET['token']));
      }
      $this->invitation = reset($invitations);
      /* Search for the form object */
      $forms = objects::ls(
        'publicForm',
        array(
          'dn'                            => 'raw',
          'cn'                            => 1,
          'fdPublicFormTitle'             => 1,
          'fdPublicFormText'              => 1,
          'fdPublicFormFinalText'         => 1,
          'fdPublicFormTemplateType'      => 1,
          'fdPublicFormTemplateDN'        => 1,
          'fdPublicFormPrivate'           => 1,
          'fdPublicFormCreationBase'      => 1,
          'fdPublicFormTosUrl'            => 1,
          'fdPublicFormAlteredAttributes' => '*',
        ),
        $this->invitation['fdInvitationFormDN'],
        '', FALSE, 'base'
      );
      $formName = $this->invitation['fdInvitationFormDN'];
    } elseif (isset($_GET['form'])) {
      /* Search for the form object */
      $forms = objects::ls(
        'publicForm',
        array(
          'dn'                            => 'raw',
          'cn'                            => 1,
          'fdPublicFormTitle'             => 1,
          'fdPublicFormText'              => 1,
          'fdPublicFormFinalText'         => 1,
          'fdPublicFormTemplateType'      => 1,
          'fdPublicFormTemplateDN'        => 1,
          'fdPublicFormPrivate'           => 1,
          'fdPublicFormCreationBase'      => 1,
          'fdPublicFormTosUrl'            => 1,
          'fdPublicFormAlteredAttributes' => '*',
        ),
        NULL,
        '(cn='.ldap_escape_f($_GET['form']).')'
      );
      $formName = $_GET['form'];
    } else {
      throw new FusionDirectoryException(_('Form name or token missing from URL'));
    }
    if (count($forms) <= 0) {
      throw new FusionDirectoryException(sprintf(_('Form "%s" could not be found'), $formName));
    } elseif (count($forms) > 1) {
      throw new FusionDirectoryException(sprintf(_('Found several forms named "%s"'), $formName));
    }
    $this->form = reset($forms);
    if (isset($this->registration)) {
      /* We wait for form to be fetched so that form name is shown even if invitation is unusable */
      invitationPage::checkToken($this->registration);
    }
    if (!isset($this->registration) && isset($this->form['fdPublicFormPrivate']) && ($this->form['fdPublicFormPrivate'] == 'TRUE')) {
      // Private form and no token
      throw new FusionDirectoryException(sprintf(_('Form "%s" is private and requires an invitation'), $this->form['cn']));
    }
  }

  protected function prepareTemplateObject()
  {
    $this->template = new template($this->form['fdPublicFormTemplateType'], $this->form['fdPublicFormTemplateDN']);

    $values = $this->getPrefilledValues();

    $this->alteredAttributes            = array(array(),array(),array());
    $this->providedAttributes           = array();
    $this->providedMultiValueAttributes = array();
    foreach ($this->form['fdPublicFormAlteredAttributes'] as $alteredAttribute) {
      list($attribute, $mandatory, $readonly, $imported, $hidden) = explode('|', $alteredAttribute, 5);
      $tab = $this->template->getAttributeTab($attribute);
      if ($mandatory) {
        $this->alteredAttributes[0][$tab][] = $attribute;
      }
      if ($this->prefill) {
        if ($readonly) {
          /* Check that mandatory readonly attributes are prefilled */
          if (
              isset($values[$tab][$attribute]) ||
              (!$mandatory && !$this->template->getAttribute($tab, $attribute)->isRequired())
            ) {
            $this->alteredAttributes[1][$tab][] = $attribute;
          }
        }
        if ($imported) {
          $this->providedAttributes[] = array($tab, $attribute);
          if ($this->template->getAttribute($tab, $attribute) instanceof SetAttribute) {
            $this->providedMultiValueAttributes[] = $attribute;
          }
        }
      }
      if ($hidden) {
        $this->alteredAttributes[2][$tab][] = $attribute;
      }
    }

    $this->template->alterAttributes(...$this->alteredAttributes);
    $this->template->setValues($values, TRUE);
  }

  protected function submitForm()
  {
    /* Check TOS checkbox */
    if (isset($this->form['fdPublicFormTosUrl']) && !isset($_POST['tosCheckBox'])) {
      msg_dialog::display(_('Error'), _('You need to accept the terms of service to continue!'), ERROR_DIALOG);
      return;
    }
    // Save values for later
    $values = $this->template->getValues();
    // Apply the template
    $userTabObject  = $this->template->apply();
    /* Fill base depending on publicForm creation base and invitation base */
    $base = $this->form['fdPublicFormCreationBase'];
    if (isset($this->invitation)) {
      $invitationBase = preg_replace('/,?'.preg_quote($config->current['BASE'], '/').'$/', '', dn2base($this->invitation['dn'], get_ou('invitationRDN')));
      if (!empty($invitationBase)) {
        $base = $invitationBase.','.$base;
      }
    }
    $userTabObject->getBaseObject()->base = $base;
    // Try to save the object
    $errors         = $userTabObject->save();
    if (empty($errors)) {
      $this->done = TRUE;
      if (isset($this->registration)) {
        $tabObject  = objects::open($this->registration['dn'], 'registration');
        $baseObject = $tabObject->getBaseObject();

        $baseObject->fdRegistrationUserDN     = $userTabObject->dn;
        $baseObject->fdRegistrationState      = 'filled';
        $baseObject->fdRegistrationLastChange = date('c').'|'.$ui->dn;

        $messages = $tabObject->save();
        msg_dialog::displayChecks($messages);
      }
    } else {
      msg_dialog::displayChecks($errors);
      $this->template->reset();
      /* After the reset we need to call alterAttributes again */
      $this->template->alterAttributes(...$this->alteredAttributes);
      // This fill the saved values into the resetted template
      $this->template->setValues($values);
    }
  }

  protected function getPrefilledValues($values = array())
  {
    if (isset($this->registration['fdRegistrationEmailAddress']) && !isset($values['mailAccount']['mail'])) {
      /* Fill email field with target email if empty */
      if (!isset($values['mailAccount'])) {
        $values['mailAccount'] = array();
      }
      $values['mailAccount']['mail'] = $this->registration['fdRegistrationEmailAddress'];
    }

    return $values;
  }

  function save_object()
  {
    global $config, $ui;

    if (!$this->activated) {
      return;
    }

    if ($this->done) {
      return;
    }

    if (!isset($this->form)) {
      try {
        $this->fetchForm();
      } catch (FusionDirectoryException $e) {
        $this->done         = TRUE;
        $this->errorMessage = $e->getMessage();
        return;
      }
      $this->prepareTemplateObject();
    }

    $this->template->save_object();

    if (isset($_POST['form_submit'])) {
      $this->submitForm();
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
      // TODO - Check that no dialogs are opened?
      $smarty->assign('template_dialog', $templateOutput);
    }

    $smarty->assign('done',         $this->done);
    $smarty->assign('errorMessage', $this->errorMessage);

    $smarty->assign('form', $this->form);

    $smarty->append('css_files',    get_template_path('login.css'));
    if (isset($this->form['fdPublicFormTitle'])) {
      $smarty->assign('title',      sprintf(_('Form - %s'), $this->form['fdPublicFormTitle']));
    } else {
      $smarty->assign('title',      _('Form'));
    }
    $smarty->display(get_template_path('headers.tpl'));

    $smarty->assign('activated',      $this->activated);
    $smarty->display(get_template_path('public-form.tpl'));
  }
}
?>
