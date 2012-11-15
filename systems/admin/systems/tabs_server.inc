<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class servtabs extends tabs
{
  public $was_activated = FALSE;

  function servtabs($config, $data, $dn, $category, $copied_object = NULL)
  {
    parent::tabs($config, $data, $dn, $category, $copied_object);

    /* Add references/acls/snapshots */
    $this->addSpecialTabs();
  }


  /*! \brief Reinitializes the tab classes with fresh ldap values.
    This maybe usefull if for example the apply button was pressed.
   */
  function re_init()
  {
    $baseobject = NULL;
    foreach ($this->by_object as $name => $object) {
      $class = get_class($object);
      if (in_array($class, array("reference","acl"))) {
        continue;
      }
      if ($baseobject === NULL) {
        $baseobject = new $class($this->config, $this->dn, NULL, $this);
        $baseobject->enable_CSN_check();
        $this->by_object[$name] = $baseobject;
      } else {
        $this->by_object[$name] = new $class($this->config, $this->dn, $baseobject, $this);
      }
      $this->by_object[$name]->parent = &$this;
      $this->by_object[$name]->set_acl_category($this->acl_category);
    }
  }


  function save_object($save_current = FALSE)
  {
    parent::save_object($save_current);

    $baseobject                   = $this->getBaseObject();
    $baseobject->netConfigDNS->cn = $baseobject->cn;
  }


  function save($ignore_account = FALSE)
  {
    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    $baseobject = $this->getBaseObject();
    $this->dn   = "cn=".$baseobject->cn.",".get_ou('serverRDN').$baseobject->base;

    /* cn is not case sensitive for ldap, but for php it is!! */
    if ($baseobject->orig_dn != "new") {
      if ($this->config->get_cfg_value("accountPrimaryAttribute") == "cn") {
        if (strtolower($baseobject->orig_dn) != (strtolower($this->dn))) {
          $baseobject->recursive_move($baseobject->orig_dn, $this->dn);
        }
      } else {
        if ($baseobject->orig_dn != $this->dn) {
          $baseobject->recursive_move($baseobject->orig_dn, $this->dn);
        }
      }
    }

    $baseobject->dn = $this->dn;

    foreach ($this->by_object as $key => $obj) {
      $this->by_object[$key]->dn = $this->dn;
      $this->by_object[$key]->cn = $baseobject->cn;
    }

    parent::save(FALSE);

    /* Reload server settings after saving */
    $config = session::get('config');
    $config->load_servers();
    session::set('config', $config);
  }

}

?>
