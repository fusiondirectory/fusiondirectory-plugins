<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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


class dhcpNewSectionDialog extends plugin
{
  /* attribute list for save action */
  var $ignore_account     = TRUE;
  var $attributes         = array();
  var $objectclasses      = array("whatever");

  /* Mapping array */
  /*  ajout de dhcpTsigKey et dhcpDnsZone */
  var $types= array();
  var $classtype= "";
  var $sectionMap= array(  "dhcpService" => array("dhcpSharedNetwork", "dhcpSubnet", "dhcpGroup", "dhcpHost", "dhcpClass","dhcpTSigKey","dhcpDnsZone"),
                              "dhcpClass" => array("dhcpSubClass"),
                              "dhcpSubClass" => array(),
                              "dhcpHost" => array(),
                              "dhcpGroup" => array("dhcpHost"),
                              "dhcpPool" => array(),
                              "dhcpSubnet" => array("dhcpPool", "dhcpGroup", "dhcpHost", "dhcpClass","dhcpTSigKey","dhcpDnsZone"),
                              "dhcpSharedNetwork" => array("dhcpSubnet", "dhcpPool","dhcpTsigKey","dhcpDnsZone"),
                              "dhcpTSigKey"=> array(), "dhcpDnsZone" => array());


  function dhcpNewSectionDialog($type)
  {
    $this->types= array(  "dhcpService" => _("Global options"),
                          "dhcpClass" => _("Class"),
                          "dhcpSubClass" => _("Subclass"),
                          "dhcpHost" => _("Host"),
                          "dhcpGroup" => _("Group"),
                          "dhcpPool" => _("Pool"),
                          "dhcpSubnet" => _("Subnet"),
                          "dhcpSharedNetwork" => _("Shared network"),
                          "dhcpTSigKey" => _("DNS update key"),
                          "dhcpDnsZone" => _("DNS update zones"));

    $this->classtype= $type;
  }

  function execute()
  {
    /* Fill templating stuff */
    $smarty = get_smarty();
    $display= "";

    $sections= $this->sectionMap[$this->classtype];
    $t_sections= array();
    foreach ($sections as $section){
      $t_sections[$section]= $this->types[$section]; 
    }
    asort($t_sections);
    $ui = get_userinfo();
    $smarty->assign("sections", $t_sections);
    $display.= $smarty->fetch(get_template_path('dhcpNewSection.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }

  /* Get posts and set class name 
   */ 
  function save_object()
  {
  }

  /* Check given class name */
  function check()
  {
    /* Call common method to give check the hook */
    $message= "";
 
    return ($message);
  }


  /* Return the class name */
  function save()
  {
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
