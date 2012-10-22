<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2003-2010 GONICUS GmbH
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011-2012 FusionDirectory project
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

/* Dummy attribute class in order to give stats information to the template */
class FakeAttribute extends Attribute
{
  private $var_name;

  function __construct ($var)
  {
    parent::__construct("Fake one", "", $var, FALSE, "", "noacl");
    $this->setInLdap(FALSE);
    $this->var_name = $var;
  }

  function renderAttribute(&$attributes)
  {
    $var = $this->var_name;
    $attributes[$this->getLdapName()] = $this->plugin->$var;
  }
}

class board extends simplePlugin
{
  /* Definitions */
  var $plHeadline     = "Board";
  var $plDescription  = "Statistics and various informations";

  /* Generic */
  var $view_logged    = FALSE;

  var $stats  = array();

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->stats = $this->overview_stats();
  }

    /* Compute some statistics */
  function overview_stats()
  {
    $ldap = $this->config->get_ldap_link();

    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(!(objectClass=gosaUserTemplate))(objectClass=gosaAccount))", array("cn"));
    $nb_accounts = $ldap->count();

    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=posixGroup)", array("cn"));
    $nb_groups = $ldap->count();
    $ldap->search("(objectClass=gosaGroupOfNames)", array("cn"));
    $nb_ogroups = $ldap->count();

    $stats = array(
      array('name' => 'Accounts',      'nb' => $nb_accounts, 'img' => get_template_path("plugins/users/images/select_user.png")),
      array('name' => 'Groups',        'nb' => $nb_groups,   'img' => get_template_path("plugins/groups/images/groups.png")),
      array('name' => 'Object groups', 'nb' => $nb_ogroups,  'img' => get_template_path("plugins/ogroups/images/ogroup.png")),
    );

    if (class_available("systemManagement")) {
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(|(objectClass=gotoWorkstation)(objectClass=goServer)(objectClass=sambaSamAccount)(objectClass=gotoTerminal)(objectClass=gotoPrinter)(objectClass=goFonHardware)(objectClass=ieee802Device))", array("cn"));
      $nb_systems = $ldap->count();
      array_unshift(
        $stats,
        array('name' => 'Systems', 'nb' => $nb_systems, 'img' => get_template_path("plugins/systems/images/select_server.png"))
      );
    }

    return $stats;
  }

  static function getAttributesInfo()
  {
    return array(
      'stats' => array(
        'name'  => _('Statistics'),
        'attrs' => array(new FakeAttribute('stats')),
        'template' => get_template_path('main_stats.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Board"),
      "plDescription" => _("Statistics and various informations"),
      "plIcon"        => "plugins/board/images/plugin.png",
      "plSelfModify"  => FALSE,
      "plPriority"    => 0,
      "plSection"     => array("addon" => _("Addons")),
      "plCategory"    => array("board" => array("objectClass" => "inetOrgPerson", "description" => _("board"))),

      "plProvidedAcls"    => array()
    );
  }
}
?>
