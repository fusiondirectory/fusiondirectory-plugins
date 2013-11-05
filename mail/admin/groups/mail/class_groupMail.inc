<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2009-2010 Bas Tichelaar
  Copyright (C) 2013  FusionDirectory

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

class GroupMailsAttribute extends MailsAttribute
{
  function getForbiddenValues ()
  {
    $forbidden = array_values($this->getValue());
    $forbidden = array_merge($forbidden, array_values($this->plugin->attributesAccess['gosaMailAlternateAddress']->getValue()));
    $forbidden[] = $this->plugin->attributesAccess['mail']->getValue();
    return $forbidden;
  }

  function getFilterBlackList ()
  {
    return array('mail' => $this->getForbiddenValues());
  }
}

class mailgroup extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('gosaMailAccount');
  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'    => _('Mail'),
      'plDescription'  => _('Group mail options'),
      'plSelfModify'   => FALSE,
      'plPriority'     => 10,
      'plCategory'     => array('groups'),
      'plObjectType'   => array('group'),
      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    /* Intialize the used mailMethod */
    if (isset($object->cn)) {
      $this->cn = $object->cn;
    } elseif (isset($this->attrs['cn'])) {
      $this->cn = $this->attrs['cn'][0];
    }

    $tmp = new mailMethod($config, $this);

    $this->mailMethod = $tmp->get_method();
    $this->mailMethod->fixAttributesOnLoad();

    $this->attributesAccess['gosaMailServer']->setChoices($this->mailMethod->getMailServers());
  }


  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Generic'),
        'attrs' => array(
          new HiddenAttribute (
            'gosaMailDeliveryMode', FALSE,
            '[]'
          ),
          new MailAttribute (
            _('Primary address'), _('The primary mail address'),
            'mail', TRUE
          ),
          new SelectAttribute (
            _('Server'), _('Server mail'),
            'gosaMailServer', TRUE,
            array('')
          ),
          new SetAttribute (
            new StringAttribute (
              _('Alternative addresses'), _('Alternative mail addresses for the group'),
              'gosaMailAlternateAddress'
            )
          ),
          new GroupMailsAttribute (
            _('Forward messages to non group members'), _('Forward messages to non group members'),
            'gosaMailForwardingAddress'
          )
        )
      )
    );
  }
}
