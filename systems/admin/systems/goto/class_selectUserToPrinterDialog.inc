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

class selectUserToPrinterDialog extends plugin
{
  /* attribute list for save action */
  var $ignore_account       = TRUE;
  var $attributes           = array();
  var $objectclasses        = array("whatever");
  var $regex                = "*";
  var $subtree              = FALSE;
  var $depselect            = "/";
  var $deplist              = "/";

  var $searchObjects        = "";
  var $searchAttrs          = "";
  var $searchAppend         = "";
  var $baseAddition         = "";
  var $type                 = "";
  var $search_cat           = "";

  var $dns_used             = array();

  function selectUserToPrinterDialog (&$config, $dn= NULL,$type=false,$already_assigned )
  {
    plugin::plugin ($config, $dn);
    $this->depselect = $this->config->current['BASE'];

    switch($type){
      case "AddUser" :
        $this->searchObjects = "(objectClass=gosaAccount)(!(uid=*$))";
        $this->searchAttrs   = array("cn","uid");
        $this->searchAppend  = "uid";
        $this->baseAddition  = get_people_ou();
        $this->search_cat    = "users";
          ;break;
      case "AddGroup" :
        $this->searchObjects = "(objectClass=posixGroup)";
        $this->searchAttrs   = array("cn","description");
        $this->searchAppend  = "cn";
        $this->baseAddition  = get_groups_ou();
        $this->search_cat    = "groups";
          ;break;
      case "AddAdminUser" :
        $this->searchObjects = "(objectClass=gosaAccount)(!(uid=*$))";
        $this->searchAttrs   = array("cn","uid");
        $this->searchAppend  = "uid";
        $this->baseAddition  = get_people_ou();
        $this->search_cat    = "users";
          ;break;
      case "AddAdminGroup" :
        $this->searchObjects = "(objectClass=posixGroup)";
        $this->searchAttrs   = array("cn","description");
        $this->searchAppend  = "cn";
        $this->baseAddition  = get_groups_ou();
        $this->search_cat    = "groups";
          ;break;
    }
    $this->type = $type;

    /* Collect already assigned object dns */
    $this->dns_used = array();
    if(isset($already_assigned[$type])){
      foreach($already_assigned[$type] as $entry){
        $this->dns_used[] = $entry['dn'];
      }
    }
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

    if(isset($_POST['dialogissubmitted'])){
      $this->regex=$_POST['regexPrinter'];
      $this->depselect = $_POST['depselectPrinter'];
    }
    $this->subtree= isset($_POST['SubSearch']);

    if((isset($_GET['search']))&&(!empty($_GET['search']))){
      $this->regex=$_GET['search']."*";
      $this->regex=preg_replace("/\*\*/","*",$this->regex);
    }


    /* Get all departments within this subtree */
    $base = $this->config->current['BASE'];
    $deps_res[] = array("dn"=>$this->config->current['BASE']);
    $deps_res= array_merge($deps_res,get_list("(&(|(ou=*)(description=*))(objectClass=gosaDepartment))", array("users","groups"), $this->config->current['BASE'],
                    array("ou", "description"), GL_SIZELIMIT | GL_SUBSEARCH));

    /* Load possible departments */
    $ui     = get_userinfo();

    /* Allowed user departments */
    $udeps  = $ui->get_module_departments("users");

    /* Allowed groups department */
    $gdeps  = $ui->get_module_departments("groups");

    /* Combine both arrays */
    $tdeps  = array_unique(array_merge($udeps,$gdeps));

    /* Create usable departments array */
    $ids = $this->config->idepartments;
    $deps = array();
    foreach($deps_res as $dep){
      if(isset($ids[$dep['dn']]) && in_array_ics($dep['dn'], $tdeps)){
        $value = $ids[$dep['dn']];
        $deps[$dep['dn']] = $value;
      }
    }
    if(!isset($deps[$this->depselect])){
      $this->depselect = key($deps);
    }


    $tmp_printers= $this->getPrinter();
    natcasesort($tmp_printers);
    $smarty->assign("regexPrinter"        , $this->regex);
    $smarty->assign("deplistPrinter"      , $deps);;//deplist);
    $smarty->assign("depselectPrinter"    , $this->depselect);
    $smarty->assign("gotoPrinters"        , array_values($tmp_printers));
    $smarty->assign("gotoPrinterKeys"     , array_keys($tmp_printers));
    $smarty->assign("apply"               , apply_filter());
    $smarty->assign("alphabet"            , generate_alphabet());
    $smarty->assign("search_image"        , get_template_path('images/lists/search.png'));
    $smarty->assign("tree_image"          , get_template_path('images/lists/search-subtree.png'));
    $smarty->assign("infoimage"           , get_template_path('images/info.png'));
    $smarty->assign("launchimage"         , get_template_path('images/lists/action.png'));
    $smarty->assign("deplist"             , $deps);
    $smarty->assign("subtree", $this->subtree?"checked":"");

    $display.= $smarty->fetch(get_template_path('selectUserToPrinterDialog.tpl', TRUE,dirname(__FILE__)));
    return($display);
  }

  function check(){
    /* Call common method to give check the hook */
    $message= plugin::check();

    if(empty($_POST['gotoPrinter'])){
      $message[] = _("Please select a printer or press cancel.");
    }
    return $message;
  }

  /* Save to LDAP */
  function save()
  {
    $a_return['type']=$this->type;

    foreach($_POST['gotoPrinter'] as $name){
      $data = $this->getPrinter(true);
      $a_return[$name]= $data[$name];
    }
    return($a_return);
  }

  /* This function generates the Printerlist
   * All printers are returned that match regex and and depselect
   */
  function getPrinter($detailed = false)
  {
    $a_return=array();

    $filter = "(&".$this->searchObjects."(cn=".$this->regex."))";
    $base   = $this->baseAddition.$this->depselect;
    $attrs  = $this->searchAttrs;
    $cat    = $this->search_cat;

    if ($this->subtree){
      $res= get_list($filter,$cat,$this->depselect,$attrs, GL_SUBSEARCH);
    } else {
      $res= get_list($filter,$cat,$base,$attrs);
    }
    foreach($res as $printer){

      /* Skip already used entries */
      if(in_array($printer['dn'],$this->dns_used)) {
        continue;
      }

      if(($detailed ==true)){
        if(isset($printer[$this->searchAppend])){
          $a_return[$printer[$this->searchAppend][0]] = $printer;
        }
      }else{
        if(isset($printer[$this->searchAppend])){
          if(isset($printer['description'][0])){
            $a_return[$printer[$this->searchAppend][0]] = $printer['cn'][0]." - ".$printer['description'][0];
          }else{
            $a_return[$printer[$this->searchAppend][0]] = $printer['cn'][0];
          }
        }
      }
    }
    return($a_return);
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
