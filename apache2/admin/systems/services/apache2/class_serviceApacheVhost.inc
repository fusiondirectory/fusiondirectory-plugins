<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class serviceApache extends simpleService
{
  var $objectclasses = array("whatever");

  function __construct ($config, $dn = NULL, $parent = NULL)
  {
    parent::__construct ($config, $dn, $parent, array());

    if (($dn != 'new') && ($dn != NULL)) {

      $dn_without_base = preg_replace("/,".preg_quote($this->config->current['BASE'], '/')."$/", "", $this->dn);

      $this->vhostList = new serviceApacheVhostManagement($this->config, get_userinfo(), $dn_without_base);

      /* If there is at least one entry in this -> types, we have apache vhosts enabled */

      if ($this->vhostList->count() == 0) {
        $this->is_account = FALSE;
      } else {
        $this->is_account = TRUE;
      }
    } else {
      $this->is_account = FALSE;
    }

    $this->initially_was_account = $this->is_account;
  }


  function execute()
  {
    if (isset($_POST['SaveVhostChanges'])) {
      $_POST['edit_finish'] = $_POST['SaveVhostChanges']; // So that the management execute save the entry
    }
    if (isset($_POST['CancelVhostChanges'])) {
      $_POST['edit_cancel'] = $_POST['CancelVhostChanges']; // So that the management execute cancel the entry
    }
    if (isset($_POST['vhost_delete_confirmed'])) {
      $_POST['delete_confirmed'] = $_POST['vhost_delete_confirmed']; // So that the management execute delete the entry
    }
    if (isset($_POST['vhost_delete_canceled'])) {
      $_POST['delete_canceled'] = $_POST['vhost_delete_canceled'];
    }
    return $this->vhostList->execute();
  }

  function save()
  {
  }

  /*  \brief Return plugin information used by the ACL handling.
      @return Array ACL infos.
   */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Apache service"),
      "plDescription" => _("Apache virtual host service")." ("._("Services").")",
      "plIcon"        => "plugins/apache2/images/iconMini.png",
      "plSelfModify"  => FALSE,
      "plCategory"    => array("server"),

      "plProvidedAcls"  => array(
        "VirtualHost" => _("Virtual hosts")
      )
    );
  }
}
?>
