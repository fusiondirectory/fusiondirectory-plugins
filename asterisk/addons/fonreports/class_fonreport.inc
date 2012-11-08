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

class fonreport extends plugin
{
  /* For internal use */
  var $start          = 0;
  var $search_for     = "*";
  var $search_base    = "";
  var $fields         = array("calldate", "src", "dst", "channel", "lastapp", "disposition", "duration");
  var $year           = "";
  var $month          = "";
  var $sort           = 0;
  var $sort_direction = "down";
  var $report_list    = array();
  var $userfilter     = "";
  var $ui             = NULL;
  var $range          = 20;
  var $EntryPerPage   = 20;

  var $selected_server  = "";

  /* attribute list for save action */
  var $attributes_SO  = array("start","search_for","search_base","range","month","sort_direction","sort","year");
  var $objectclasses  = array();
  var $view_logged    = FALSE;

  /* Construct class */
  function fonreport (&$config, $ui)
  {
    /* Include config object */
    $this->config       = $config;
    $this->ui           = $ui;
    $this->search_base  = get_base_from_people($ui->dn);

    $this->month = date("m");
    $this->year  = date("Y");

    /* Use filter settings if we have already searched */
    if (!session::is_set("fonfilter")) {
      $fonfilter = array();
      foreach ($this->attributes_SO as $name) {
        $fonfilter[$name] = $this->$name;
      }
      session::set("fonfilter", $fonfilter);
    } else {
      $fonfilter = session::get("fonfilter");
      foreach ($this->attributes_SO as $name) {
        $this->$name = $fonfilter[$name];
      }
    }
  }


  /* Save ui interactions and store results in session,
      to remember settings */
  function save_object()
  {
    $fonfilter = session::get("fonfilter");
    if (isset($_POST['EntryPerPage'])) {
      $this->range = $_POST['EntryPerPage'];
    }
    if (isset($_GET['start'])) {
      $this->start = (int)$_GET['start'];
    }
    foreach (array("year", "month", "search_for", "search_base","selected_server") as $type) {
      if (isset($_POST[$type])) {
        $this->$type = get_post($type);
      }
    }

    /* Adapt sorting */
    if (isset($_GET['sort'])) {
      if ($this->sort == (int)$_GET['sort']) {
        if ($this->sort_direction == "down") {
          $this->sort_direction = "up";
        } else {
          $this->sort_direction = "down";
        }
      }
      $this->sort = (int)$_GET['sort'];
      if ($this->sort < 0 || $this->sort > 6) {
        $this->sort = 0;
      }
    }

    /* remove unwanted tags */
    $this->search_for = stripslashes(preg_replace("/[^0-9a-z\*\+ \-\/]/i", "", $this->search_for));

    foreach ($this->attributes_SO as $name) {
      $fonfilter[$name] = $this->$name;
    }
    session::set("fonfilter", $fonfilter);
  }

  /* Search & display results */
  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* GVet template engine */
    $smarty= get_smarty();

    /* Log view */
    if (!$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view", "fonreport/".get_class($this), $this->dn);
    }

    /*****************
      Variable Init
     *****************/

    $fields_str = "";

    $months= array();
    for ($i = 1 ; $i <= 12 ; $i ++ ) {
      $months[$i] = _(date("F",gmmktime(0,0,0,$i)));
    }

    /* Prepare template */
    $current= date("Y");
    $years= array();
    for ($y= $current - 5; $y<=$current; $y++) {
      $years[$y]= $y;
    }

    /*****************
      Smarty
     *****************/
    $bases      = array();
    $cat_bases  = $this->ui->get_module_departments("fonreport");
    foreach ($this->config->idepartments as $dn => $name) {
      if (in_array_ics($dn,$cat_bases)) {
        $bases[$dn] = $name;
      }
    }

    if (!isset($bases[$this->search_base])) {
      $this->search_base = key($bases);
    }

    $smarty->assign("plug",           "?plug=".validate($_GET['plug']));
    $smarty->assign("launchimage",    get_template_path('images/launch.png'));
    $smarty->assign("search_image",   get_template_path('images/lists/search.png'));
    $smarty->assign("search_for",     htmlentities($this->search_for));
    $smarty->assign("bases",          $bases);
    $smarty->assign("base_select",    $this->search_base);
    $smarty->assign("months",         $months);
    $smarty->assign("month_select",   $this->month);
    $smarty->assign("years",          $years);
    $smarty->assign("year_select",    $this->year);
    $smarty->assign("search_result",  "");


    /*****************
      Check Database , Table , Connection
     *****************/

    // Collect servers and allow to select the server in the ui.
    $servers = array();
    foreach($this->config->data['SERVERS']['FON'] as $key => $server) {
      $servers[$server['SERVER']] = $server['SERVER'];
    }
    $smarty->assign("servers",  $servers);
    $smarty->assign("selected_server",  $this->selected_server);

    /* Connecting, selecting database */
    if (!isset($this->config->data['SERVERS']['FON'][0])) {
      msg_dialog::display(_("Configuration error"), msgPool::noserver(_("GOfon")), WARNING_DIALOG);
      return($smarty->fetch(get_template_path('contents.tpl', TRUE)));
    } else {
      // Get CFG for the selected server, if empty use first.
      $cfg = $this->config->data['SERVERS']['FON'][0];
      foreach ($this->config->data['SERVERS']['FON'] as $server) {
        if($server['SERVER'] == $this->selected_server){
          $cfg = $server;
          break;
        }
      }
      $cfg['DB'] = "asteriskcdrdb";
    }


    /*****************
      Get Query String && Search
     *****************/

    $query  = $this->CreateQuerySyntax();
    $link = databaseManagement::connectDatabase($cfg);
    if (PEAR::isError($link)) {
      new log("debug","GOfon/".get_class($this),"",array(),$link->getMessage());
      return msgPool::dbconnect("GOfon",$link->getMessage(),$cfg['SERVER']);
    }

    @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,$query, "Database query");

    $result = $link->query($query);
    if (PEAR::isError($result)){
      msg_dialog::display(_("Error"), msgPool::dbquery(_("GOfon"),$result->getMessage(),$cfg['SERVER']),ERROR_DIALOG);
      return($smarty->fetch(get_template_path('contents.tpl', TRUE)));
    }

    /*****************
      Fetch results
     *****************/

    $report_list= array();

    /* Restricted attributes will not be displayed, this will be displayed instead */
    $no_acl = "<img class='center' src='images/lists/locked.png'
      title='".msgPool::permView()."' alt='"._("Insufficient permissions")."'>";

    $no_acl = "&nbsp;";

    while ($line = $result->fetchRow(MDB2_FETCHMODE_ASSOC)) {

      foreach($line as $attr => $value){

        if($attr == "duration") continue;

        $acl = $this->ui->get_permissions($this->search_base,"fonreport/fonreport",$attr);
        if(!preg_match("/r/",$acl)){
          $line[$attr] = $no_acl;
        }
      }

      if($this->ui->get_permissions($this->search_base,"fonreport/fonreport","calldate")){
        $hour=  substr($line["calldate"], 11, 2);
        $minute=substr($line["calldate"], 14, 2);
        $format= _("Y-M-D");
        $date= preg_replace("/Y/", substr($line["calldate"], 0, 4), $format);
        $date= preg_replace("/M/", substr($line["calldate"], 5, 2), $date);
        $date= preg_replace("/D/", substr($line["calldate"], 8, 2), $date);
        $date_str = $date." ".$hour.":".$minute;
      }else{
        $date_str = $no_acl;
      }

      $append_str = "";
      $append_str .=  "<td>".$date_str."</td>";

      foreach(array("src","dst","channel","lastapp","disposition") as $atr){
        if(isset($line[$atr])){
          $append_str .=  "<td>".$line[$atr]."</td>";
        }
      }

      if($this->ui->get_permissions($this->search_base,"fonreport/fonreport","duration")){
        $append_str .=  "<td>".$this->gen_duration($line["duration"])."</td>";
      }else{
        $append_str .=  "<td>".$no_acl."</td>";
      }
      $report_list[] = $append_str;
    }

    $this->report_list= $report_list;
    $link->disconnect();


    /*****************
      Create list of results
     *****************/

    /* Generate output */
    $mod      = 0;
    $output   = "";
    if(count($this->report_list) < $this->start){
      $this->start = 0;
    }
    foreach($this->report_list as $val){
      if ($mod < $this->start) {
        $mod++;
        continue;
      }
      if ($mod >= ($this->start + $this->range)){
        $mod++;
        break;
      }
      if ( ($mod++) & 1){
        $col= "background-color: #ECECEC;";
      } else {
        $col= "background-color: #F5F5F5;";
      }
      $output.= "<tr style=\"height:22px; $col\">$val</tr>";
    }

    /*****************
      Tell smarty what we have found
     *****************/

    if ($output != ""){
      $smarty->assign("search_result", $output);
      $smarty->assign("range_selector", range_selector(count($this->report_list), $this->start,$this->range,"EntryPerPage"));
    } else {
      $smarty->assign("search_result", "");
    }

    /* Show main page */
    for($i= 0; $i<7; $i++){
      $smarty->assign("mode$i", "");
    }
    $smarty->assign("mode".$this->sort, "<img alt=\"\" src=\"images/sort_".$this->sort_direction.".png\" border=0 align=middle>");

    return($smarty->fetch(get_template_path('contents.tpl', TRUE)));
  }


  /* Create duration string   12'11" */
  function gen_duration($seconds)
  {
    if ($seconds / 60 > 1){
      $minutes= (int)($seconds / 60);
      $seconds= $seconds % 60;
      return ("$minutes&rsquo;$seconds&rdquo;");
    }
    return ("$seconds&rdquo;");
  }


  /* Create WHERE part for our search */
  function GetUidMatchingFilter()
  {
    $ldap       = $this->config->get_ldap_link();
    $ldap->cd     ($this->search_base);
    $s          = $this->search_for;

    $s          = preg_replace("/\%/","",$s);
    $s2         = preg_replace("/\*/","%",$s);

    $filter = "(&(objectClass=gosaAccount)(!(objectClass=gosaUserTemplate))".
        "(|(uid=$s)(l=$s)(homePhone=$s)(telephoneNumber=$s)(facsimileTelephoneNumber=$s)(mobile=$s)".
        "(pager=$s)(cn=$s)(givenName=$s)(sn=$s)(personalTitle=$s)(title=$s)))";

    $attrs  = array("uid");
    $res    = get_sub_list($filter,"fonreport/fonreport",get_people_ou(),$this->search_base,$attrs);

    $str = " AND (";
    $fields = array("dstchannel","channel");
    if(count($res)){
      foreach($res as $attrs){
        $uid =  $attrs["uid"][0];
        foreach($fields as $name){
          $str .= $name." like '%".$uid."%' OR ";
        }
      }
    }
    $str .= " channel     like '%".$s."%' OR
              dstchannel  like '%".$s."%' OR
              dst         like '".$s2."' OR
              src         like '".$s2."' OR
              lastapp     like '".$s2."')";
    return($str);
  }


  /* Create query string */
  function CreateQuerySyntax()
  {
    /* Get extended search filter which contain uids and so on */
    $uidstring = $this->GetUidMatchingFilter();

    /* Create string with all fields seperated by ,*/
    $fields_str ="";
    foreach($this->fields as $field){
      if($field == "calldate") {
        continue;
      }
      $fields_str .= $field.", ";
    }
    $fields_str = preg_replace("/, $/","",$fields_str);

    /* Create Sort tag */
    if ($this->sort_direction == "down"){
      $desc= "DESC";
    } else {
      $desc= "ASC";
    }
    /* Create times */
    $start= date ("YmdHis", mktime(0,0,0,$this->month,1,$this->year));
    $end=   date ("YmdHis", mktime(23,59,59,($this->month +1),0,$this->year));
    $query = "SELECT ".$fields_str.",calldate FROM cdr ".
      "WHERE
      calldate <= $end
      AND
      calldate >= $start
      ". $uidstring."
      ORDER BY ".$this->fields[$this->sort]." $desc;";
    return($query);
  }


  /* Return plugin informations for acl handling
     #FIXME You can only read attributes within this report plugin */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Phone reports"),
      "plDescription" => _("Phone reports")."&nbsp;<i>"._("All entries are readonly")."</i>",
      "plIcon"        => "plugins/gofon/images/phonereport.png",
      "plSelfModify"  => TRUE,
      "plPriority"    => 0,
      "plCategory"    => array("fonreport" => array( "description" => _("Phone reports"),
                                                      "objectClass" => "None")),
      "plSection"     => "addons",

      "plProvidedAcls" => array(
        "calldate"    => _("Date"),
        "src"         => _("Source"),
        "dst"         => _("Destination"),
        "channel"     => _("Channel"),
        "lastapp"     => _("Application called"),
        "disposition" => _("Disposition"),
        "duration"    => _("Duration")
      )
    );
  }
}
?>
