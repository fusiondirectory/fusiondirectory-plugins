<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

class newsletterSubscriptions extends simplePlugin
{
  var $displayHeader = TRUE;

  var $objectclasses = array('fdNewsletterAccount');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Newsletter'),
      'plDescription'   => _('Newsletter subscriptions'),
      'plSelfModify'    => TRUE,
      'plObjectType'    => array('user'),
      'plIcon'          => 'geticon.php?context=applications&icon=newsletter&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=newsletter&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    global $config;
    return array(
      'main' => array(
        'name'  => _('Personal info'),
        'attrs' => array(
          new SetAttribute(
            new SelectAttribute (
              _('Subscriptions'), _('Newsletter you are subscribed to'),
              'fdNewsletterSubscriptions', FALSE,
              $config->get_cfg_value('newsletterChoices', array())
            )
          ),
        )
      ),
    );
  }

  function __construct ($unused, $dn = NULL, $object = NULL)
  {
    parent::__construct($unused, $dn, $object);
  }
}
?>
