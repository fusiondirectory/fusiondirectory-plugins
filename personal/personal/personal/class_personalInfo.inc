<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016  FusionDirectory

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

class SocialAccountAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = "")
  {
    $handlers = socialHandler::listHandlers();
    parent::__construct(
      $description, $ldapName,
      [
        new SelectAttribute(
          _('Site'), _('Website the account is on'),
          'socialSite', TRUE,
          array_keys($handlers), '',
          array_values($handlers)
        ),
        new StringAttribute(
          _('Id'), _('Id of this user on this website'),
          'socialId', TRUE
        )
      ],
      '/^([^:]+):(.*)$/',
      '%s:%s',
      $acl, $label
    );
  }

  function getArrayValue ()
  {
    $handlerClass = 'socialHandler_'.$this->attributes[0]->getValue();
    if (!class_available($handlerClass)) {
      return [$handlerClass, $this->attributes[1]->getValue()];
    }
    $handler      = new $handlerClass();
    return [$handlerClass::getName(), ['html' => $handler->linkify($this->attributes[1]->getValue())]];
  }

  function check (): array
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    $handlerClass = 'socialHandler_'.$this->attributes[0]->getValue();
    if (!class_available($handlerClass)) {
      return 'Social handler "'.$handlerClass.'" does not exists';
    }
    $handler      = new $handlerClass();
    $value = $this->attributes[1]->getValue();
    try {
      $value = $handler->validate($this->attributes[1]->getValue());
      $this->attributes[1]->setValue($value);
    } catch (socialHandlerInvalidValueException $e) {
      return $e->getMessage();
    }
  }
}

class personalInfo extends simplePlugin
{
  var $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Personal'),
      'plDescription'   => _('Personal information'),
      'plFilter'        => '(objectClass=fdPersonalInfo)',
      'plSelfModify'    => TRUE,
      'plObjectClass'   => ['fdPersonalInfo'],
      'plObjectType'    => ['user'],
      'plIcon'          => 'geticon.php?context=applications&icon=user-info&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=user-info&size=16',
      'plPriority'      => 3,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Personal info'),
        'attrs' => [
          new StringAttribute(
            _('Personal title'), _('Personal title - Examples of personal titles are "Ms", "Dr", "Prof" and "Rev"'),
            'personalTitle', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              _('Nickname'), _('Nicknames for this user'),
              'fdNickName'
            )
          ),
          new StringAttribute(
            _('Badge Number'), _('Company badge number'),
            'fdBadge', FALSE
          ),
          new DateAttribute(
            _('Date of birth'), _('Date of birth'),
            'dateOfBirth', FALSE,
            'Y-m-d', '',
            '30 years ago'
          ),
          new SelectAttribute(
            _('Sex'), _('Gender'),
            'gender', FALSE,
            ['', 'M', 'F'], '', ['', 'male', 'female']
          ),
          new StringAttribute(
            _('Country'), _('Country'),
            'co', FALSE
          ),
          new GeneralizedTimeDateAttribute(
            _('Start date'), _('Date this user joined the company'),
            'fdContractStartDate', FALSE,
            ''
          ),
          new GeneralizedTimeDateAttribute(
            _('End date'), _('Date this user is supposed to leave the company'),
            'fdContractEndDate', FALSE,
            ''
          ),
          new BooleanAttribute(
            _('Photo Visible'), _('Should the photo of the user be visible on external tools'),
            'fdPhotoVisible', FALSE, TRUE
          ),
        ]
      ],
      'contact' => [
        'name'  => _('Contact'),
        'attrs' => [
          new OrderedArrayAttribute(
            new SocialAccountAttribute(
              _('Social account'), _('Social accounts of this user'),
              'fdSocialAccount'
            ),
            FALSE
          ),
          new SetAttribute(
            new MailAttribute(
              _('Private email'), _('Private email addresses of this user'),
              'fdPrivateMail'
            )
          )
        ]
      ]
    ];
  }
}
