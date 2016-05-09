<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2013-2016 FusionDirectory

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

class serviceOPSI extends simpleService
{
  var $objectclasses = array('opsiServer');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI service'),
      'plDescription' => _('OPSI service').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=applications&icon=opsi&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'      => _('OPSI Server informations'),
        'attrs'     => array(
          new StringAttribute(
            _('Server URI'), _('The URI to use for connection'),
            'fdOpsiServerURI', TRUE
          ),
          new StringAttribute(
            _('User'), _('The user to use for connection'),
            'fdOpsiServerUser', TRUE
          ),
          new PasswordAttribute(
            _('Password'), _('The password to use for connection'),
            'fdOpsiServerPassword', TRUE
          ),
        )
      )
    );
  }

  function __construct ($dn = NULL, $parent = NULL)
  {
    parent::__construct($dn, $parent);
    if ($parent !== NULL) {
      if (isset($parent->parent->netConfigDNS)) {
        $ip = $parent->parent->netConfigDNS->ipHostNumber;
      } elseif (isset($parent->parent)) {
        $ip = $parent->parent->getBaseObject()->netConfigDNS->ipHostNumber;
      } else {
        trigger_error("Can't find IP for OPSI service");
        $ip = "";
      }
      $this->attributesAccess['fdOpsiServerURI']->setDefaultValue('https://'.$ip.':4447/rpc');
      if ($this->fdOpsiServerURI == "") {
        $this->attributesAccess['fdOpsiServerURI']->resetToDefault();
      }
    }
  }
}
?>
