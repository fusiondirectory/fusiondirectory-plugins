<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class debconfStartup extends simplePlugin
{
  var $displayHeader = TRUE;

  var $objectclasses = array('debconfStartup');

  var $inheritance = array('gosaGroupOfNames' => 'member');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Debconf'),
      'plDescription'   => _('Debconf preseed startup'),
      'plPriority'      => 9,
      'plObjectType'    => array('workstation', 'server', 'ogroup-dynamic'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Debconf settings'),
        'attrs' => array(
          new SelectAttribute (
            _('Profile'), _('Debconf preseed profile to be used for installation'),
            'debconfProfile', TRUE
          ),
          new SelectAttribute (
            _('Release'), _('Debian release to install'),
            'debconfDebianRelease', TRUE,
            array('squeeze', 'unstable', 'wheezy')
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    /* Fill profile select box */
    $ldap = $config->get_ldap_link();
    $ldap->ls(
      'objectClass=organizationalUnit',
      get_ou('debconfRDN').$config->current['BASE'],
      array('ou')
    );
    $profiles = array();
    while ($attrs = $ldap->fetch()) {
      $profiles[] = $attrs['ou'][0];
    }
    $this->attributesInfo['main']['attrs']['debconfProfile']->setChoices($profiles);
  }
}
?>
