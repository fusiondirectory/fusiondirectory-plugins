<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class sudoManagement extends simpleManagement
{
  protected $objectTypes = array('sudo');

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'sudoUser', 'sudoHost');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Sudo"),
      "plDescription" => _("Sudo management"),
      "plIcon"        => "plugins/sudo/images/icon.png",
      "plSection"     => "admin",
      "plPriority"    => 5,
      "plManages"     => array("sudo"),

      "plProvidedAcls" => array()
    );
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();
    $this->registerAction('new_default', 'newEntry');
    $this->headpage->xmlData['actionmenu']['action'][0]['action'][] = array(
      'name'  => 'new_default',
      'type'  => 'entry',
      'image' => 'plugins/sudo/images/iconMiniDefault.png',
      'label' => 'Default',
      'acl'   => 'sudo/sudo[c]',
    );
  }

  function newEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    $str = parent::newEntry('new_SUDO', $target, $all, $altTabClass, $altTabType, $altAclCategory);

    /* This entry will become the default entry */
    if ($action == 'new_default') {
      $this->tabObject->set_default(TRUE);
    }
    if (!empty($str)) {
      return $str;
    }
  }
}
?>
