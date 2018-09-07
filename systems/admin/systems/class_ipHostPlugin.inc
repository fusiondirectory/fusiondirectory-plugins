<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2018 FusionDirectory

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

/*! \brief This class is made for plugins using the ipHostNumber field
 *
 * It adds the ipHost class if the field is filled, and removes it otherwise
 * It is also used to list classes having the IP filled
 */
class ipHostPlugin extends simplePlugin
{
  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['ipHostNumber']->setRequired(
      in_array(get_class($this), $config->get_cfg_value('mandatoryIpClasses', array()))
    );
    $this->attributesAccess['ipHostNumber']->setUnique('whole', '(objectClass=ipHost)');
    if (isset($this->attributesAccess['macAddress'])) {
      $this->attributesAccess['macAddress']->setUnique('whole', '(objectClass=ieee802Device)');
    }
  }

  /* Used by prepare_save and template::apply */
  public function mergeObjectClasses(array $oc)
  {
    $objectclasses = $oc;
    if (empty($this->ipHostNumber)) {
      $objectclasses = array_remove_entries_ics(array('ipHost'), $objectclasses);
    } else {
      $objectclasses[] = 'ipHost';
    }
    return parent::mergeObjectClasses($objectclasses);
  }
}
