<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class nisObject extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses  = array('nisObject');

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Directory'),
      'plDescription'   => _('Directory'),
      'plCategory'      => array('autofs'),
      'plObjectType'    => array('nisObject' => array('name'    => _('Directory'),
                                                      'filter'  => 'objectClass=nisObject')),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Directory'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('autofsRDN')),
          new HostNameAttribute (
            _('Name'), _('Name of this directory'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Automount entry'), _("The entry of this directory for the automount daemon.\n For instance 'auto.u' or '-fstype=nfs domaine.tld:/mount/directory'"),
            'nisMapEntry', TRUE
          ),
          new SelectAttribute (
            _('Mount point'), _('The mount point this directory will be placed in'),
            'nisMapName', TRUE,
            array()
          )
        )
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesAccess['nisMapName']->setChoices($this->getMountPoints());
  }

  function save_object()
  {
    parent::save_object();
    $this->attributesAccess['nisMapName']->setChoices($this->getMountPoints());
  }

  function getMountPoints ()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd(get_ou('autofsRDN').$this->base);
    $ldap->search("(objectClass=nisMap)", array('nisMapName'));
    $nisMaps = array();
    while ($attrs = $ldap->fetch()) {
      $nisMaps[] = $attrs['nisMapName'][0];
    }
    return $nisMaps;
  }

  /* We need to fix the base */
  function get_allowed_bases()
  {
    $this->base = preg_replace('/^nisMapName=[^,]+,/', '', $this->base);
    return parent::get_allowed_bases();
  }
}


?>
