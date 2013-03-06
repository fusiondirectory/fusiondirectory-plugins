<?php

/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2009-2010 Bas Tichelaar
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


class zarafagroup extends simplePlugin
{
  var $displayHeader = TRUE;
  var $objectclasses              = array("zarafa-group");

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"    => _("Zarafa"),
          "plDescription"  => _("Zarafa group"),
          "plSelfModify"   => FALSE,
          "plDepends"      => array(),
          "plPriority"     => 10,
          "plSection"      => array("administration"),
          "plCategory"     => array("groups"),
          "plProvidedAcls" => parent::generatePlProvidedAcls(self::getAttributesInfo())
          ));
  }


  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesInfo['section1']['attrs']['zarafaSecurityGroup']->setManagedAttributes(
      array(
        'erase' => array(
          FALSE => array(
            'section1' => array(
              'zarafaHidden',
            )
          )
        )
      )
    );
  }


   static function getAttributesInfo ()
  {
    return array(
      'section1' => array(
        'name'  => _('Zarafa Settings'),
        'attrs' => array(
          new BooleanAttribute(
            _('Make this group a Zarafa Securitygroup'),
            _('Make this group a Zarafa Securitygroup'),
            'zarafaSecurityGroup',
            FALSE,
            FALSE,
            '',
            1,
            0
          ),
          new BooleanAttribute(
            _('Hide from Zarafa addressbook'),
            _('Hide from Zarafa addressbook'),
            'zarafaHidden',
            FALSE,
            FALSE,
            '',
            1,
            0
          )
        )
      )
    );
  }
}
