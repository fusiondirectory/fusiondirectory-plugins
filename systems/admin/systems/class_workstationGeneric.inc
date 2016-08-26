<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class workstationGeneric extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('fdWorkstation', 'ipHost', 'ieee802Device');

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ($word = NULL, $rdn = NULL)
  {
    if ($word === NULL) {
      $word = _('workstation');
    }
    if ($rdn === NULL) {
      $rdn = get_ou('workstationRDN');
    }
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new BaseSelectorAttribute($rdn),
          new HostNameAttribute (
            _('Name'),
            sprintf(_('The name of the %s'), $word),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'),
            sprintf(_('A short description of the %s'), $word),
            'description', FALSE
          ),
          new StringAttribute (
            _('Location'),
            sprintf(_('The location of the %s'), $word),
            'l', FALSE
          ),
          new BooleanAttribute (
            sprintf(_('Lock this %s'), $word),
            sprintf(_('This will prevent the %s from being reinstalled'), $word),
            'fdMode',
            FALSE,
            FALSE,
            '',
            'locked',
            'unlocked'
          ),
        )
      ),
      'network' => array(
        'name'      => _('Network settings'),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
        'attrs'     => array(
          new SetAttribute(
            new IPAttribute(
              _('IP address'), _('IP addresses this system uses (v4 or v6)'),
              'ipHostNumber', TRUE
            )
          ),
          new MacAddressAttribute(
            _('Mac address'), _('Mac address of this system'),
            'macAddress', FALSE
          ),
        ),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique(TRUE);
    $this->attributesAccess['ipHostNumber']->setUnique('whole');
    $this->attributesAccess['macAddress']->setUnique('whole');
  }

  function remove_from_parent()
  {
    if (!$this->initially_was_account) {
      return;
    }

    if ($this->acl_is_removeable()) {
      $errors = parent::remove_from_parent();

      /* Remove all accessTo/trust dependencies */
      /* FIXME replace with foreignKeys */
      update_accessTo($this->cn, "");

      /* Clean queue form entries with this mac */
      if (class_available("supportDaemon") && tests::is_mac($this->attributesAccess['macAddress']->getInitialValue())) {
        $q = new supportDaemon();
        $q->clean_queue_from_mac($this->attributesAccess['macAddress']->getInitialValue());
      }

      return $errors;
    }
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Workstation'),
      'plDescription' => _('Workstation information'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('workstation' =>
        array(
          'name'        => _('Workstation'),
          'description' => _('Workstation'),
          'filter'      => 'objectClass=fdWorkstation',
          'ou'          => get_ou('workstationRDN'),
          'icon'        => 'geticon.php?context=devices&icon=computer&size=16',
          'mainAttr'    => 'cn',
        )
      ),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }
}
?>
