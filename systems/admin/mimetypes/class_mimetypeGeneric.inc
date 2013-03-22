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

class mimetype extends plugin
{
  var $ignore_account   = true; // This is always a valid account
  var $objectclasses  = array("top","gotoMimeType");
  var $attributes   = array("cn","gotoMimeApplication","gotoMimeLeftClickAction","gotoMimeIcon",
                  "description","gotoMimeFilePattern","gotoMimeEmbeddedApplication",
                  "gotoMimeGroup");

  /* Class vars */
  var $ui;
  var $cn         = "";
  var $gotoMimeLeftClickAction    = "I";
  var $gotoMimeLeftClickAction_I    = true;
  var $gotoMimeLeftClickAction_E    = false;
  var $gotoMimeLeftClickAction_Q    = false;
  var $gotoMimeIcon     = "*removed*";
  var $description      = "";
  var $gotoMimeFilePattern    = array();
  var $gotoMimeApplication    = array();
  var $gotoMimeEmbeddedApplication  = array();
  var $gotoMimeGroup      = "";
  var $iconData       = NULL;
  var $base       = "";
  var $ApplicationList      = array();


  /* To prevent errors when using the 'apply' button, we use this variables
      to manage array attributes */
  var $use_gotoMimeIcon     = NULL;
  var $use_gotoMimeFilePattern    = array();
  var $use_gotoMimeApplication    = array();
  var $use_gotoMimeEmbeddedApplication  = array();

  /* divLists */
  var $DivPatterns        = NULL;
  var $DivApps          = NULL;
  var $DivEApps         = NULL;

  var $FAIstate ="";

  /* These vars will be copied too, if you use copy&paste mode */
  var $CopyPasteVars        = array("use_gotoMimeFilePattern","use_gotoMimeApplication","use_gotoMimeEmbeddedApplication","iconData",
                                    "gotoMimeLeftClickAction_I","gotoMimeLeftClickAction_E","gotoMimeLeftClickAction_Q","use_gotoMimeIcon");

  var $view_logged  = FALSE;

  /* Select options */
  var $MimeGroups           = array("application","audio","chemical","image","inode","message","model",
                                      "multipart","text","video","x-conference","x-world");

  /* Orig_Dn is used to verify if this object is new or only edited */
  var $orig_dn    = "";
  var $orig_base  = "";

  function mimetype(&$config,$dn= NULL)
  {
    plugin::plugin ($config, $dn);

    /* Save original dn */
    $this->orig_dn = $dn;

    /* get gotoMimeLeftClickActions I/E/Q */
    if(isset($this->gotoMimeLeftClickAction)){
      $str = $this->gotoMimeLeftClickAction;
      for($i = 0 ; $i < strlen($str) ; $i ++ ){
        $varna = "gotoMimeLeftClickAction_". $str[$i];
        if(isset($this->$varna)){
          $this->$varna = true;
        }
      }
    }

    /* Create list of defined applications in FusionDirectory */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gosaApplication)(cn=*))",array('cn'));
    $this->ApplicationList = array();
    while($attrs = $ldap->fetch()){
      $this->ApplicationList[$attrs['cn'][0]] = $attrs['cn'][0];
    }

    /* If both radio buttons arn't set, set option I */
    if(!$this->gotoMimeLeftClickAction_I && !$this->gotoMimeLeftClickAction_E){
      $this->gotoMimeLeftClickAction_I = true;
    }

    /* Get list of array attributes */
    foreach(array("gotoMimeFilePattern") as $attr){
      $this->$attr = array();
      if(isset($this->attrs[$attr])){
        $tmp = array();
        for($i = 0 ; $i < $this->attrs[$attr]['count'] ; $i ++){
          $str = $this->attrs[$attr][$i];
          $tmp[] = $str;
        }
        $use_attr = "use_".$attr;
        $this->$use_attr = $tmp;
      }
    }

    /* Get list of array attributes with priority tag ( Test|32 )*/
    foreach(array("gotoMimeApplication","gotoMimeEmbeddedApplication") as $attr){
      $this->$attr = array();
      if(isset($this->attrs[$attr])){
        $tmp = array();
        for($i = 0 ; $i < $this->attrs[$attr]['count'] ; $i ++){
          $str = $this->attrs[$attr][$i];
          $tmp2= explode("|",$str);

          if(count($tmp2) == 2){
            $name = $tmp2[0];
            $prio = $tmp2[1];
            $tmp[$prio] = $name;
          }
        }
        ksort($tmp);
        $use_attr = "use_".$attr;
        $this->$use_attr = $tmp;
      }
    }

    /* Set base */
    if ($this->dn == "new"){
      if(session::is_set('CurrentMainBase')){
        $this->base= session::get('CurrentMainBase');
      }else{
        $ui= get_userinfo();
        $this->base= dn2base($ui->dn);
      }
    } else {
      $this->base= preg_replace ("/^.*,".preg_quote(get_ou("mimetypeRDN"), '/')."/i", "", $this->dn);
    }

    /* Get icon data */
    if(isset($this->attrs['gotoMimeIcon'])){
      $ldap = $this->config->get_ldap_link();
      $this->iconData = $ldap->get_attribute($this->dn,"gotoMimeIcon");
      $this->saved_attributes['gotoMimeIcon'] = $this->iconData;
    }
    if ($this->iconData == ""){
      $this->set_new_picture("");
    }
    session::set('binary',$this->iconData);
    session::set('binarytype',"image/jpeg");
    $this->orig_base = $this->base;
  }


  function execute()
  {
    $smarty = get_smarty();

    if(!$this->view_logged){
      $this->view_logged =TRUE;
      new log("view","mimetypes/".get_class($this),$this->dn);
    }

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name,preg_match("/freeze/i",$this->FAIstate)));
    }
    $smarty->assign("IconReadable", preg_match("/r/",$this->getacl("gotoMimeIcon",preg_match("/freeze/i",$this->FAIstate))));

    /* Check Posts */
    $posts = array(
        "/^Pattern_SortUp_/"  => array("Action" => "Pattern_SortUp",  "Func" => "ArrayUp",  "Attr" => "use_gotoMimeFilePattern"),
        "/^Pattern_SortDown_/"  => array("Action" => "Pattern_SortDown","Func" => "ArrayDown",  "Attr" => "use_gotoMimeFilePattern"),
        "/^Pattern_Remove_/"  => array("Action" => "Pattern_Remove",  "Func" => "ArrayRemove","Attr" => "use_gotoMimeFilePattern"),
        "/^Apps_SortUp_/"     => array("Action" => "Apps_SortUp",   "Func" => "ArrayUp",    "Attr" => "use_gotoMimeApplication"),
        "/^Apps_SortDown_/"   => array("Action" => "Apps_SortDown", "Func" => "ArrayDown",  "Attr" => "use_gotoMimeApplication"),
        "/^Apps_Remove_/"     => array("Action" => "Apps_Remove",   "Func" => "ArrayRemove","Attr" => "use_gotoMimeApplication"),
        "/^EApps_SortUp_/"    => array("Action" => "EApps_SortUp",  "Func" => "ArrayUp",    "Attr" => "use_gotoMimeEmbeddedApplication"),
        "/^EApps_SortDown_/"  => array("Action" => "EApps_SortDown",  "Func" => "ArrayDown",  "Attr" => "use_gotoMimeEmbeddedApplication"),
        "/^EApps_Remove_/"    => array("Action" => "EApps_Remove",  "Func" => "ArrayRemove","Attr" => "use_gotoMimeEmbeddedApplication"));
    $once = true;

    /* Walk through posts and try to find some commands for us. */
    if(!preg_match("/freeze/i",$this->FAIstate)){
      foreach($_POST as $name => $value){

        /* Walk through possible commands */
        foreach($posts as $regex => $action){

          /* Check if there is a command posted */
          if(preg_match($regex,$name) && $once){
            $once = false;

            /* Get action vars */
            $func     = $action['Func'];    // Get function name
            $attr     = $action['Attr'];    // Get attribute name
            $acl_a    = preg_replace("/^use_/","",$action['Attr']);   // Get attribute name

            /* Get entry id */
            $s_entry    = preg_replace($regex,"",$name);
            $s_entry  = preg_replace("/_[xy]$/","",$s_entry);

            /* Execute a command with the given attribute and entry
               e.g. $this->gotoMimeFilePattern = $this->ArrayUp(3,$this->gotoMimeFilePattern) */
            if($this->acl_is_writeable($acl_a)){
              $this->$attr= $this->$func($s_entry,$this->$attr,true);
            }
          }
        }
      }

      /* Set a new icon was requested */
      if(isset($_POST['update_icon']) && (isset($_FILES['picture_file']['name']))){
        $this->set_new_picture($_FILES['picture_file']['tmp_name']);
      }

      /* Add gotoMimeFilePattern */
      if(isset($_POST['AddNewFilePattern']) && isset($_POST['NewFilePattern'])){
        if($this->acl_is_writeable("gotoMimeFilePattern")){
          $str = trim($_POST['NewFilePattern']);
          if($str != ""){
            $this->use_gotoMimeFilePattern[] = $str;
          }
        }
      }

      /* Add gotoMimeFilePattern */
      if(isset($_POST['AddNewApplication']) &&
          (isset($_POST['NewApplication']) || isset($_POST['NewApplicationSelect']))){
        if($this->acl_is_writeable("gotoMimeApplication")){
          $str = "";
          if(isset($_POST['NewApplicationSelect']) && !empty($_POST['NewApplicationSelect'])){
            $str = trim(get_post("NewApplicationSelect"));
          }
          if(isset($_POST['NewApplication']) && !empty($_POST['NewApplication'])){
            $str = trim(get_post("NewApplication"));
          }
          if($str != "" && !in_array($str,$this->use_gotoMimeApplication)){
            $this->use_gotoMimeApplication[] = $str;
          }
        }
      }

      /* Add embedded application
       *  - From input or from select box
       */
      if(isset($_POST['AddNewEmbeddedApplication']) &&
          (isset($_POST['NewEmbeddedApplication']) || isset($_POST['NewEmbeddedApplicationSelect']))){
        if($this->acl_is_writeable("gotoMimeEmbeddedApplication")){
          $str = "";
          if(isset($_POST['NewEmbeddedApplicationSelect']) && !empty($_POST['NewEmbeddedApplicationSelect'])){
            $str = trim(get_post('NewEmbeddedApplicationSelect'));
          }
          if(isset($_POST['NewEmbeddedApplication']) && !empty($_POST['NewEmbeddedApplication'])){
            $str = trim(get_post('NewEmbeddedApplication'));
          }
          if($str != "" && !in_array($str,$this->use_gotoMimeEmbeddedApplication)){
            $this->use_gotoMimeEmbeddedApplication[] = $str;
          }
        }
      }
    }

    /* Create divlists */
    $DivPatterns  = new divSelectBox("gotoMimePatterns");
    $DivApps    = new divSelectBox("gotoMimeApplications");
    $DivEApps     = new divSelectBox("gotoMimeEmbeddedApplications");
    $DivPatterns  -> SetHeight(100);
    $DivApps    -> SetHeight(100);
    $DivEApps   -> SetHeight(100);


    if($this->acl_is_writeable("gotoMimeFilePattern") && !preg_match("/freeze/i",$this->FAIstate)){
      $Pattern_Actions= " <input type='image' src='images/lists/sort-up.png'  class='center'  name='Pattern_SortUp_%s' >&nbsp;
      <input type='image' src='images/lists/sort-down.png'  class='center'  name='Pattern_SortDown_%s'>&nbsp;
      <input type='image' src='images/lists/trash.png'  class='center'  name='Pattern_Remove_%s'>";
    }else{
      $Pattern_Actions= "";
    }

    if($this->acl_is_writeable("gotoMimeApplication") && !preg_match("/freeze/i",$this->FAIstate)){
      $Apps_Actions   = " <input type='image' src='images/lists/sort-up.png'  class='center'  name='Apps_SortUp_%s' >&nbsp;
      <input type='image' src='images/lists/sort-down.png'  class='center'  name='Apps_SortDown_%s'>&nbsp;
      <input type='image' src='images/lists/trash.png'  class='center'  name='Apps_Remove_%s'>";
    }else{
      $Apps_Actions= "";
    }

    if($this->acl_is_writeable("gotoMimeEmbeddedApplication") && !preg_match("/freeze/i",$this->FAIstate)){
      $EApps_Actions  = " <input type='image' src='images/lists/sort-up.png'  class='center'  name='EApps_SortUp_%s' >&nbsp;
      <input type='image' src='images/lists/sort-down.png'  class='center'  name='EApps_SortDown_%s'>&nbsp;
      <input type='image' src='images/lists/trash.png'  class='center'  name='EApps_Remove_%s'>";
    }else{
      $EApps_Actions= "";
    }

    /* Before adding some entries check acls */
    if($this->acl_is_readable("gotoMimeFilePattern")){
      foreach($this->use_gotoMimeFilePattern as $key => $pattern){
        $field1 = array("string" => $pattern);
        $field2 = array("string" => preg_replace("/%s/",$key,$Pattern_Actions),"attach"=>"style='border-right:0px;width:50px;'");
        $fields = array($field1,$field2);
        $DivPatterns -> AddEntry($fields);
      }
    }

    if($this->acl_is_readable("gotoMimeApplication")){
      foreach($this->use_gotoMimeApplication as $key => $pattern){
        $field1 = array("string" => $pattern);
        $field2 = array("string" => preg_replace("/%s/",$key,$Apps_Actions),"attach"=>"style='border-right:0px;width:50px;'");
        $fields = array($field1,$field2);
        $DivApps -> AddEntry($fields);
      }
    }
    if($this->acl_is_readable("gotoMimeEmbeddedApplication")){
      foreach($this->use_gotoMimeEmbeddedApplication as $key => $pattern){
        $field1 = array("string" => $pattern);
        $field2 = array("string" => preg_replace("/%s/",$key,$EApps_Actions),"attach"=>"style='border-right:0px;width:50px;'");
        $fields = array($field1,$field2);
        $DivEApps -> AddEntry($fields);
      }
    }
    $smarty->assign("bases",            $this->get_allowed_bases());
    $smarty->assign("base_select",          $this->base);
    $smarty->assign("isReleaseMimeType",      $this->is_release());
    $smarty->assign("gotoMimeFilePatterns",     $DivPatterns->DrawList());
    $smarty->assign("gotoMimeApplications",     $DivApps->DrawList());
    $smarty->assign("gotoMimeEmbeddedApplications", $DivEApps->DrawList());

    $smarty->assign("ApplicationList",$this->ApplicationList);

    /* Assign class vars to smarty */
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    /* Assign additional vars that are not included in attributes*/
    foreach(array("gotoMimeLeftClickAction_I","gotoMimeLeftClickAction_E","gotoMimeLeftClickAction_Q") as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    /* Assign select box options */
    $smarty->assign("gotoMimeGroups",$this->MimeGroups);
    $smarty->assign("gotoMimeIcon"  ,$this->get_picture());
    return($smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
  }


  function save_object()
  {
    if(isset($_POST['MimeGeneric']) && !preg_match("/freeze/i",$this->FAIstate)){

      /* Create a base backup and reset the
         base directly after calling plugin::save_object();
         Base will be set seperatly a few lines below */
      $base_tmp = $this->base;
      plugin::save_object();
      $this->base = $base_tmp;

      /* Only save base if we are not in release mode */
      if(!$this->is_release()){

        /* Set new base if allowed */
        $tmp = $this->get_allowed_bases();
        if(isset($_POST['base'])){
          if(isset($tmp[$_POST['base']])){
            $this->base= $_POST['base'];
          }
        }

      }

      /* Save radio buttons */
      if($this->acl_is_writeable("gotoMimeLeftClickAction")){
        if(isset($_POST['gotoMimeLeftClickAction_IE'])){
          $chr = $_POST['gotoMimeLeftClickAction_IE'];
          if($chr == "E"){
            $this->gotoMimeLeftClickAction_E = true;
            $this->gotoMimeLeftClickAction_I = false;
          }else{
            $this->gotoMimeLeftClickAction_E = false;
            $this->gotoMimeLeftClickAction_I = true;
          }
        }
        if(isset($_POST['gotoMimeLeftClickAction_Q'])){
          $this->gotoMimeLeftClickAction_Q = true;
        }else{
          $this->gotoMimeLeftClickAction_Q = false;
        }
      }
    }
  }


  /* save current changes */
  function save()
  {
    /* Create gotoMimeLeftClickAction out of checkboxes and radio buttons */
    $arr = array ("E","I","Q");
    $str = "";
    foreach ($arr as $Chr){
      $var = "gotoMimeLeftClickAction_".$Chr;
      if($this->$var){
        $str .= $Chr;
      }
    }
    $this->gotoMimeLeftClickAction = $str;

    /* Create array entries with priority tag ( Test|3 )*/
    foreach(array("gotoMimeEmbeddedApplication","gotoMimeApplication") as $attr){
      $i        = 0;
      $use_attr     = "use_".$attr;
      $tmp      = array();
      $this->$attr  = array();
      foreach($this->$use_attr as $entry){
        $tmp[] = $entry."|".$i ++;
      }
      $this->$attr = $tmp;
    }

    /* Create array entries */
    foreach(array("gotoMimeFilePattern") as $attr){
      $i        = 0;
      $use_attr     = "use_".$attr;
      $tmp      = array();
      $this->$attr  = array();
      foreach($this->$use_attr as $entry){
        $tmp[] = $entry;
      }
      $this->$attr = $tmp;
    }

    /* Remove Icon if requested  */
    if($this->use_gotoMimeIcon != "*removed*"){
      $this->gotoMimeIcon = $this->iconData;
    }else{
      $this->gotoMimeIcon = "";
    }

    plugin::save();

    /* If this is a newly created object, skip storing those
       attributes that contain an empty array */
    if($this->orig_dn == "new"){
      foreach(array("gotoMimeEmbeddedApplication","gotoMimeApplication","gotoMimeFilePattern") as $attr){
        if(!count($this->$attr)){
          unset($this->attrs[$attr]);
        }
      }
    }

    $ldap = $this->config->get_ldap_link();
    $ldap-> cd ( $this->config->current['BASE']);
    $ldap->cat($this->dn);
    if($ldap->count()){
      $ldap->cd($this->dn);
      $this->cleanup();
      $ldap->modify($this->attrs);
      new log("modify","mimetypes/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
      new log("create","mimetypes/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
  }


  /* Remove current mime type */
  function remove_from_parent()
  {
    plugin::remove_from_parent();
    $ldap = $this->config->get_ldap_link();
    $ldap->rmDir($this->dn);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }
    new log("remove","mimetypes/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove");

    /* Delete references to object groups */
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
    while ($ldap->fetch()){
      $og= new ogroup($this->config, $ldap->getDN());
      unset($og->member[$this->dn]);
      $og->save ();
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $og->dn, 0, get_class()));
      }
    }
  }


  /* Check given values */
  function check()
  {
    $message = plugin::check();
    if($this->cn == "" ){
      $message[] = msgPool::required(_("Name"));
    }
    if(!count($this->use_gotoMimeFilePattern)){
      $message[] = msgPool::required(_("File pattern"));
    }

    /* Check if there is already a mime type with this cn */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current["BASE"]);

    if($this->is_release()){
      $base = $this->parent->parent->mime_release;
    }else{
      $base = get_ou("mimetypeRDN").$this->base;
    }

    $ldap->ls("(&(objectClass=gotoMimeType)(cn=".$this->cn."))",$base,array("cn"));
    if($ldap->count()){
      $attrs = $ldap->fetch();
      if($this->dn != $attrs['dn']) {
        $message[]= msgPool::duplicated("cn");
      }
    }

    /* Check if we are allowed to create or move this object
     */
    if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
      $message[] = msgPool::permCreate();
    }elseif($this->orig_dn != "new" && $this->base != $this->orig_base && !$this->acl_is_moveable($this->base)){
      $message[] = msgPool::permMove();
    }

    return($message);
  }


  /** Helper functions **/

  /* Set a new picture */
  function set_new_picture($filename)
  {
    if (empty($filename)){
      $filename= "./plugins/systems/images/default_icon.png";
      $this->use_gotoMimeIcon= "*removed*";
    }else{
      $this->use_gotoMimeIcon= $filename;
    }

    if (file_exists($filename)){
      $fd = fopen ($filename, "rb");
      $this->iconData= fread ($fd, filesize ($filename));
      session::set('binary',$this->iconData);
      session::set('binarytype',"image/jpeg");
      fclose ($fd);
    }
  }

  /* Get picture link */
  function get_picture()
  {
    session::set('binary',$this->iconData);
    session::set('binarytype',"image/jpeg");
    return("getbin.php");
  }

  /* Transports the given Arraykey one position up*/
  function ArrayUp($atr,$attrs)
  {
    $ret = $attrs;
    $pos = $this->getpos($atr,$attrs) ;
    $cn = count($attrs);
    if(!(($pos == -1)||($pos == 1))){
      $before = array_slice($attrs,0,($pos-2));
      $mitte  = array_reverse(array_slice($attrs,($pos-2),2));
      $unten  = array_slice($attrs,$pos);
      $ret = array();
      $ret = $this->combineArrays($before,$mitte,$unten);
    }
    return($ret);
  }


  /* Transports the given Arraykey one position down*/
  function ArrayDown($atr,$attrs)
  {
    $ret = $attrs;
    $pos = $this->getpos($atr,$attrs) ;
    $cn = count($attrs);
    if(!(($pos == -1)||($pos == $cn))){
      $before = array_slice($attrs,0,($pos-1));
      $mitte  = array_reverse(array_slice($attrs,($pos-1),2));
      $unten  = array_slice($attrs,($pos+1));
      $ret = array();
      $ret = $this->combineArrays($before,$mitte,$unten);
    }
    return($ret);
  }


  /* return the position of the element in the array */
  function getpos($atr,$attrs)
  {
    $i = 0;
    foreach($attrs as $attr => $name)    {
      $i++;
      if($attr == $atr){
        return($i);
      }
    }
    return(-1);
  }


  /* Remove this element from array */
  function ArrayRemove($key,$array,$reorder = false)
  {
    if(isset($array[$key])){
      unset($array[$key]);
      if($reorder){
        $tmp = array();
        foreach($array as $entry){
          $tmp[] = $entry;
        }
        $array = $tmp;
      }
    }
    return($array);
  }


  /* Combine new array */
  function combineArrays($ar0,$ar1,$ar2)
  {
    $ret = array();
    if (is_array($ar0)) {
      foreach ($ar0 as $ar => $a) {
        $ret[] = $a;
      }
    }
    if (is_array($ar1)) {
      foreach ($ar1 as $ar => $a) {
        $ret[] = $a;
      }
    }
    if (is_array($ar2)) {
      foreach ($ar2 as $ar => $a) {
        $ret[] = $a;
      }
    }
    return $ret;
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Generic"),
      "plDescription" => _("Mime type generic"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("mimetypes" => array("description"  => _("Mime types"),
                                                    "objectClass"  => "gotoMimeType")),
      "plObjectType"  => array("mimetype" => array("name"  => _("Mime type"),
                                                  "filter"  => "objectClass=gotoMimeType")),

      "plProvidedAcls"  => array(
        "cn"                          => _("Name"),
        "gotoMimeGroup"               => _("Mime group"),
        "description"                 => _("Description"),
        "base"                        => _("Base"),
        "gotoMimeApplication"         => _("Application"),
        "gotoMimeLeftClickAction"     => _("Left click action"),
        "gotoMimeIcon"                => _("Icon"),
        "gotoMimeFilePattern"         => _("File patterns"),
        "gotoMimeEmbeddedApplication" => _("Embedded applications")
      )
    );

  }


  function is_release()
  {
    if (is_object($this->parent->parent)) {
      return $this->parent->parent->IsReleaseManagementActivated();
    } else {
      /* Check if we should enable the release selection */
      return class_available("faiManagement");
    }
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    $source_o = new mimetype($this->config,$source['dn'],$this->parent);

    foreach(array("gotoMimeLeftClickAction_Q","gotoMimeEmbeddedApplication","gotoMimeApplication","gotoMimeFilePattern","gotoMimeIcon","iconData") as $name){
      $use_attr     = "use_".$name;
      if(isset($this->$use_attr)){
        $this->$use_attr= $source_o->$use_attr;
      }
      $this->$name = $source_o->$name;
    }
    foreach($this->attributes as $name){
      $this->$name = $source_o->$name;
    }

    if($this->iconData){
      $this->use_gotoMimeIcon ="Not emtpy, causes icon to be written.";
    }
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
