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



class boardSystems extends plugin
{
  /* Generic */
  var $ui;
  var $view_logged    = FALSE;

  /* default values*/
  var $default_start_computer_id        = 0;

  function __construct (&$config, $dn = NULL)
  {
    $this->config = &$config;
    $this->ui     = get_userinfo();
  }

  function execute()
  {
    /* Call parent execute */
    parent::execute();

    $smarty = get_smarty();
    // Get list of rsyslog servers
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    /* Statistics */
    $stats = array(
      array('name' => _('workstations'), 'oc' => 'gotoWorkstation',  'img' => get_template_path("plugins/systems/images/select_workstation.png")),
      array('name' => _('servers'),      'oc' => 'goServer',         'img' => get_template_path("plugins/systems/images/select_server.png")),
      array('name' => _('winstations'),  'oc' => 'sambaSamAccount',  'img' => get_template_path("plugins/systems/images/select_winstation.png")),
      array('name' => _('terminals'),    'oc' => 'gotoTerminal',     'img' => get_template_path("plugins/systems/images/select_terminal.png")),
      array('name' => _('printers'),     'oc' => 'gotoPrinter',      'img' => get_template_path("plugins/systems/images/select_printer.png")),
      array('name' => _('phones'),       'oc' => 'goFonHardware',    'img' => get_template_path("plugins/systems/images/select_phone.png")),
      array('name' => _('components'),   'oc' => 'ieee802Device',    'img' => get_template_path("plugins/systems/images/select_component.png")),
    );

    $ldap->cd(get_ou("systemRDN").$this->config->current['BASE']);
    foreach ($stats as &$stat) {
      $ldap->search("(objectClass=".$stat['oc'].")", array("cn"));
      $stat['nb'] = $ldap->count();
    }
    $ldap->search("(objectClass=argonautServer)", array('cn','ipHostNumber','argonautProtocol','argonautPort'));
    $nb_argonaut_server = $ldap->count();
    $argonaut_server = array();
    if ($nb_argonaut_server == 1) {
      $attrs = $ldap->fetch();
      foreach (array('cn','ipHostNumber','argonautProtocol','argonautPort') as $key) {
        $argonaut_server[$key] = $attrs[$key][0];
      }
    }
    $ldap->search("(objectClass=argonautClient)", array('cn'));
    $nb_argonaut_clients = $ldap->count();


    /*
    * Begin of code for selecting next computer IDs
    * Global variable is
    * board_next_computer_ids_rule
    * syntax is :
    * 7 : nb total of digit
    * LYP3 : prefix to use
    * each prefix is followed by the first item flagged by an equal sign
    * ex : "7;LYP0=3;LYP1=6;LYP2;LYP3"
    * if to item is doned it's 0 by default
    *
    */

    $output_next_computer_ids = "";
    $computer_ids_rules = $this->config->get_cfg_value("board_next_computer_ids_rule", "5;PC");

    /* $get_cfg_value return the OU like ou=computer .. */
    $winstation_ou  = mb_substr($this->config->get_cfg_value("sambaMachineAccountRDN"), 3); //FIXME use get_ou
    $array_rules    = explode(";", $computer_ids_rules);
    $nb_digits      = $array_rules[0];

    /* running all the table */
    for ($i = 1;$i < sizeof($array_rules);$i++) {
      /* aray initialization*/
      $array_complete_list  = array();
      $array_real_list      = array();
      $unused_computer_ids  = array();
      /* get computer ids configuration */
      $config_ids = explode("=", $array_rules[$i]);
      /* fist is is the prefix */
      $prefix = $config_ids[0];
      /* second (if specified) is the first item */
      if (!isset($config_ids[1])) {
        $start_id = $this->default_start_computer_id;
      } else {
        $start_id = $config_ids[1];
      }

      $output_next_computer_ids = $output_next_computer_ids."<tr><td><b>".$prefix."</b></td>";
      $nb_digits_suffix         = $nb_digits - strlen($prefix);

      /* generation of list of suffixe */
      for ($d = $start_id;$d < pow(10, $nb_digits_suffix);$d++) {
        /* padding : 34 on 4 digit become : 0034*/
        $array_complete_list[] = str_pad($d, $nb_digits_suffix, "0", STR_PAD_LEFT);
      }

      /* request of all computer beginning by the prefix */
      $request = "(&(|(objectClass=sambaSamAccount)(objectClass=GOhard))(cn=$prefix*)(|(ou:dn:=$winstation_ou)(ou:dn:=systems)))";
      $ldap->search($request, array("cn"));
      while ($attrs = $ldap->fetch()) {
        /* if a computer is a windows host, we have to delete the $ at the end */
        $computer_id        = str_replace("$", "", $attrs["cn"][0]);
        $array_real_list[]  = substr($computer_id, -$nb_digits_suffix);
      }
      /* make dfference between real and complete list */
      $unused_computer_ids = array_diff($array_complete_list, $array_real_list);
      asort($unused_computer_ids);
      $unused_computer_ids = array_values($unused_computer_ids);
      /* we take the 5 first */

      for ($r = 0;$r <= 5;$r++) {
        $output_next_computer_ids .= "<td>".$prefix.$unused_computer_ids[$r]."</td>";
      }
      $output_next_computer_ids .= "</tr>";
    }
    $smarty->assign("output_next_computer_ids", $output_next_computer_ids);
    $smarty->assign("stats", $stats);
    $smarty->assign("argonaut_server", $argonaut_server);
    $smarty->assign("nb_argonaut_server", $nb_argonaut_server);
    $smarty->assign("nb_argonaut_clients", $nb_argonaut_clients);

    return $smarty->fetch(get_template_path('contents.tpl', TRUE));
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Board - Systems"),
      "plDescription" => _("Statistics and informations about systems"),
      "plIcon"        => "plugins/board/images/plugin.png",
      "plSelfModify"  => FALSE,
      "plPriority"    => 0,
      "plSection"     => array("addon" => _("Addons")),
      "plCategory"    => array("board"),

      "plProvidedAcls"    => array()
    );
  }
}
?>
