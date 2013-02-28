<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2012 FusionDirectory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class simpleSecurityObject extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('organizationalRole', 'top', 'simpleSecurityObject');

  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Simple security object'),
      'plDescription' => _('Simple security object'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('dsa'),
      'plObjectType'  => array('simpleSecurityObject' => array('name'  => _('Simple security object'),
                                                              'filter'  => 'objectClass=simpleSecurityObject')),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('Simple security object'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('dsaRDN')),
          new HostNameAttribute (_('Entry name'), _('Account name'), 'cn', TRUE),
          //~ new TextAreaAttribute (
            //~ _('Description'), _('Description of this redirection'),
            //~ 'description', FALSE
          //~ ),
          new PasswordAttribute (
            _('Password'), _('Password'),
            'userPassword', TRUE
          )
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesInfo['main']['attrs']['cn']->setUnique(TRUE);
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'cn='.$this->cn.','.get_ou('dsaRDN').$this->base;
  }
}

?>
