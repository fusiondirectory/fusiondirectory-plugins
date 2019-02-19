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

class publicForm extends simplePlugin
{
  public static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Public form'),
      'plDescription' => _('Public form which creates objects inside FusionDirectory'),
      'plIcon'        => '',
      'plObjectClass' => array('fdPublicForm'),
      'plObjectType'  => array('publicForm' => array(
        'name'    => _('Public form'),
        'ou'      => get_ou('publicFormRDN'),
        'icon'    => '',
      )),
      //~ 'plForeignKeys'   => array(
        //~ 'fdPublicFormTemplateDN' => '?',
      //~ ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    global $config;

    return array(
      'main' => array(
        'name'  => _('Form'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('publicFormRDN')),
          new HostNameAttribute(
            _('Name'), _('Name used to identify this form'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Title'), _('Title of this form'),
            'fdPublicFormTitle', TRUE
          ),
          new TextAreaAttribute(
            _('Text'), _('Text of the public form, shown before the form'),
            'fdPublicFormText', FALSE
          ),
          new SelectAttribute(
            _('Object type'), _('Type of objects you wish to create from this form'),
            'fdPublicFormTemplateType', TRUE,
            array(), 'USER', NULL
          ),
          new SelectAttribute(
            _('Template'), _('Object template used by this form'),
            'fdPublicFormTemplateDN', TRUE,
            array(), '', NULL
          ),
          new TextAreaAttribute(
            _('Confirmation text'), _('Text shown once the object is created'),
            'fdPublicFormFinalText', FALSE,
            'Success'
          ),
          new BooleanAttribute(
            _('Private'), _('Restrict this form to invitations'),
            'fdPublicFormPrivate', FALSE,
            FALSE
          ),
          new StringAttribute(
            _('Creation base'), _('Base in which objects will get created'),
            'fdPublicFormCreationBase', TRUE,
            $config->current['BASE']
          ),
          new URLAttribute(
            _('TOS URL'), _('Terms of service URL. If filled, this adds a mandatory checkbox to agree to the terms at this URL.'),
            'fdPublicFormTosUrl', FALSE
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    if ($attributesInfo === NULL) {
      $attributesInfo = $this->getAttributesInfo();
    }
    $choices = template::getTemplatedTypes();
    $attributesInfo['main']['attrs'][4]->setChoices(array_keys($choices), array_values($choices));

    if (!class_available('invitationManagement')) {
      $attributesInfo['main']['attrs'][7]->setVisible(FALSE);
    }

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    $this->typeChanged();
    $this->attributesAccess['fdPublicFormTemplateType']->setSubmitForm('typeChanged');
  }

  function typeChanged ()
  {
    $templates = objects::getTemplates($this->fdPublicFormTemplateType);
    $this->attributesAccess['fdPublicFormTemplateDN']->setChoices(array_keys($templates), array_values($templates));
  }
}
?>
