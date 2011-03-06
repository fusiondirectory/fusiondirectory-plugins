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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class faiScript extends plugin
{
  /* attribute list for save action */
  var $ignore_account   = TRUE;

  /* Attributes for this Object */
  var $attributes       = array("cn","description");

  /* ObjectClasses for this Object*/
  var $objectclasses    = array("top","FAIclass","FAIscript");

  /* Class name of the Ldap ObjectClass for the Sub Object */
  var $subClass         = "FAIscriptEntry";
  var $subClasses       = array("top","FAIclass","FAIscriptEntry");

  /* Class name of the php class which allows us to edit a Sub Object */
  var $subClassName     = "faiScriptEntry";      

  /* Attributes to initialise for each subObject */
  var $subAttributes    = array("cn","description","FAIpriority"); 
  var $sub_Load_Later   = array("FAIscript");
  var $sub64coded       = array();
  var $subBinary        = array("FAIscript");

  /* Specific attributes */
  var $cn               = "";       // The class name for this object
  var $description      = "";       // The description for this set of partitions
  var $is_dialog        = false;    // specifies which buttons will be shown to save or abort
  var $SubObjects       = array();  // All leafobjects of this object

  var $FAIstate         = "branch";
  var $sort_by          = "name";
  var $sort_order       = "up";

  var $view_logged = FALSE;
  var $ui;

  function faiScript (&$config, $dn= NULL)
  {
    /* Load Attributes */
    plugin::plugin ($config, $dn);

    /* If "dn==new" we try to create a new entry
     * Else we must read all objects from ldap which belong to this entry.
     * First read SubObjects from ldap ... and then the partition definitions for the SubObjects.
     */
    if($dn != "new"){
      $this->dn =$dn;

      /* Read all leaf objects of this object (For FAIscript this would be FAIscriptEntry)
       */
      $res = FAI::get_all_objects_for_given_base($this->dn,"(&(objectClass=FAIclass)(objectClass=".$this->subClass."))");
      foreach($res as $obj){

        /* Skip not relevant objects */
        if(!preg_match("/".preg_quote($this->dn, '/')."$/i",$obj['dn'])) continue;

        $objects = array();
        $objects['status']      = "FreshLoaded";
        $objects['dn']          = $obj['dn'];
        $objects                = $this->get_object_attributes($objects,$this->subAttributes);
        $this->SubObjects[$objects['cn']] = $objects;
      }
    }

    $this->is_new = FALSE;
    if($this->dn == "new"){
      $this->is_new =TRUE;
    }
 
    $this->ui = get_userinfo();
  }


  /* Reload some attributes */
  function get_object_attributes($object,$attributes)
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->cat($object['dn'],$attributes);
    $tmp  = $ldap->fetch();

    foreach($attributes as $attrs){
      if(isset($tmp[$attrs][0])){
        $var = $tmp[$attrs][0];

        /* Check if we must decode some attributes */
        if(in_array_ics($attrs,$this->sub64coded)){
          $var = base64_decode($var);
        }

        /*  check if this is a binary entry */
        if(in_array_ics($attrs,$this->subBinary)){
          $var = $ldap->get_attribute($object['dn'], $attrs,$r_array=0);
        }

        /* Fix slashes */
        $var = addslashes($var);
        $object[$attrs] = $var;
      }
    }
    return($object);
  }

  
  /* Return a valid dn to fetch acls. Because 'new' will not work. */
  function acl_base_for_current_object($dn)
  {
    if($dn == "new" || $dn == ""){
      if($this->dn == "new"){
        $dn= $this->parent->parent->acl_base;
      }else{
        $dn = $this->dn;
      }
    }
    return($dn);
  }


  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","fai/".get_class($this),$this->dn);
    }

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

    /* Add new sub object */
    if(isset($_POST['AddSubObject']) && !preg_match("/freeze/i",$this->FAIstate)){
      $this->dialog= new $this->subClassName($this->config,"new");
      $this->dialog->FAIstate = $this->FAIstate;
      $this->dialog->set_acl_base($this->acl_base);
      $this->dialog->set_acl_category("fai");
      $this->dialog->parent = &$this;
      $this->is_dialog=true;
    }

    if($this->dn != "new"){
      set_object_info($this->dn);
    }

    /* File download requested */
    if(isset($_GET['getFAIscript'])){
      if(isset($this->SubObjects[base64_decode($_GET['getFAIscript'])])){
        $obj = $this->SubObjects[base64_decode($_GET['getFAIscript'])];
        $obj  = $this->get_object_attributes($obj,$this->sub_Load_Later);
        send_binary_content(stripslashes($obj['FAIscript']),$obj['cn'].".FAIscript"); 
      }
    }
    
    /* Handle posts */
    $s_action = $s_entry = "";
    foreach($_POST as $name => $value){

      /* Edit script posted */
      if(preg_match("/^editscript_/",$name)){
        $s_action = "edit";
        $s_entry = preg_replace("/^editscript_/","",$name);
        $s_entry = base64_decode(preg_replace("/_.*/","",$s_entry));
        break;
      }

      /* Delete script requested */
      if(preg_match("/^deletescript_/",$name) && !preg_match("/freeze/i",$this->FAIstate)){
        $s_action = "remove";
        $s_entry = preg_replace("/^deletescript_/","",$name);
        $s_entry = base64_decode(preg_replace("/_.*/","",$s_entry));
        break;
      }
    }

    if(isset($_GET['act']) && $_GET['act'] == "edit" && isset($_GET['id'])){
      $s_entry = base64_decode($_GET['id']);
      if(isset($this->SubObjects[$s_entry])){
        $s_action = "edit";
      }
    }

    if($s_action =="edit" && isset($this->SubObjects[$s_entry])){

      /* Get object, and load missing entry values */
      $obj  = $this->SubObjects[$s_entry];
      if($obj['status'] == "FreshLoaded"){
        $obj  = $this->get_object_attributes($obj,$this->sub_Load_Later);
      }

      /* Create new dialog and set acl attributes  */
      $this->dialog= new $this->subClassName($this->config,$this->dn,$obj);
      $this->dialog->FAIstate = $this->FAIstate;
      $this->dialog->set_acl_base($this->acl_base_for_current_object($obj['dn']));
      $this->dialog->set_acl_category("fai");

      /* Assign some additional dialog informations like headline and parent  */
      set_object_info($obj['dn']);
      $this->dialog->parent = &$this;
      $this->is_dialog=true;
    }

    /* Check acls, are we allowed to delete an entry */
    if($s_action == "remove" && isset($this->SubObjects[$s_entry])){
      $entry = $this->SubObjects[$s_entry];  
      $acl = $this->ui->get_permissions($this->acl_base_for_current_object($entry['dn']),"fai/faiScriptEntry")  ;
      if(preg_match("/d/",$acl)){
        $status = $entry['status'];
        if($status == "edited" || $status == "FreshLoaded"){
          $this->SubObjects[$s_entry]['status']= "delete";
        }else{
          unset($this->SubObjects[$s_entry]);
        }
      }
    }

      /* Save the edited entry */
    if(isset($_POST['SaveSubObject'])){

      /* Check if there are still errors remaining that must be fixed before saving */
      $this->dialog->save_object();
      $msgs = $this->dialog->check();
      if(count($msgs)>0){
        foreach($msgs as $msg){
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
        }
      }else{

        /* Get return object */
        $obj = $this->dialog->save();

        /* If we have renamed the script entry, we must remove the old entry */
        if(isset($obj['remove'])){

          /* Get old entry values */
          $old_stat = $this->SubObjects[$obj['remove']['from']]['status'];

          /* Depending on status, set new status */
          if($old_stat == "edited" || $old_stat == "FreshLoaded"){
            $this->SubObjects[$obj['remove']['from']]['status'] = "delete";
          }elseif($this->SubObjects[$obj['remove']['from']]['status']=="new"){
            unset($this->SubObjects[$obj['remove']['from']]);
          }

          /* Append the new entry */
          $obj['status'] = "new";
          $this->SubObjects[$obj['remove']['to']] = $obj;
          unset($this->SubObjects[$obj['remove']['to']]['remove']);
        }else{
  
          /* Set new status and append the entry */
          if($obj['status'] == "FreshLoaded"){
            $obj['status'] = "edited";
          }
          $this->SubObjects[$obj['cn']]=$obj;
        }
        $this->is_dialog=false;
        unset($this->dialog);
        $this->dialog=FALSE;

      }
    }

    /* Sort entries */
    $tmp = $keys = array();

    if($this->sort_by == "name"){
      foreach($this->SubObjects as $key => $entry){
        $keys[$key]=$entry['cn'];
      }
    }elseif($this->sort_by == "priority"){
      foreach($this->SubObjects as $key => $entry){
        $keys[$key]=$entry['FAIpriority'];
      }
    }

    natcasesort($keys);

    if($this->sort_order == "down"){
      $keys =array_reverse($keys);
    }

    foreach($keys as $key => $order_var){
      $tmp[$key]=$this->SubObjects[$key];
    }
    $this->SubObjects = $tmp;

    /* Cancel Dialog */
    if(isset($_POST['CancelSubObject'])){
      $this->is_dialog=false; 
      unset($this->dialog);
      $this->dialog=FALSE;
    }

    /* Print dialog if $this->dialog is set */
    if(is_object($this->dialog)){
      $this->dialog->save_object();
      $display = $this->dialog->execute();
      return($display);
    }

    /* Divlist            added 23.02.2006 
       Containing FAIscripts 
     */
    $divlist = new divlist("FAIscripts");
    $divlist->SetEntriesPerPage(0);
    $plug = $_GET['plug'];
   
    if($this->sort_order == "up"){
      $dir = "<img src='images/lists/sort-up.png' title='"._("Sort direction")."' alt='\/' border=0>";
    }else{
      $dir = "<img src='images/lists/sort-down.png' title='"._("Sort direction")."' alt='/\' border=0>";
    }
 
    if($this->sort_by == "name"){
      $sort_name = $dir;
      $sort_prio = "";
    }else{
      $sort_name = "";
      $sort_prio = $dir;
    }

    $divlist->SetHeader(array(  array("string"=>"<a href='?plug=".$plug."&amp;sort=name'>"._("Name").$sort_name."</a>"),
                                array("string"=>"<a href='?plug=".$plug."&amp;sort=priority'>".$sort_prio._("Priority")."</a>",
                                      "attach"=>"style='width:100px;'"),
                                array("string"=>_("Download"),
                                      "attach"=>"style='width:100px;'"),
                                array("string"=>_("Action"),
                                      "attach"=>"style='border-right: 0px;width:100px;text-align:right;'")));
    $divlist->SetHeight(300);
    $divlist->SetWidth("100%");
    foreach($this->getList(true) as $key => $name){

      $dn= $this->acl_base_for_current_object($name['dn']);
      $acl = $this->ui->get_permissions($dn,"fai/faiScriptEntry")  ;
      $act = "";
      
      /* Hide delete icon if this object is freezed */
      if(preg_match("/freeze/", $this->FAIstate)){
        $act .= "<input type='image' src='images/lists/edit.png'      name='editscript_%s'    title='"._("edit")."' alt='"._("edit")."'>";
      }else{
        $act .= "<input type='image' src='images/lists/edit.png'      name='editscript_%s'    title='"._("edit")."' alt='"._("edit")."'>";
        if(preg_match("/d/",$acl)){
          $act .="<input type='image' src='images/lists/trash.png' name='deletescript_%s'  title='"._("delete")."' alt='"._("delete")."'>";
        }
      }

      /* Check acls for download icon */
      $s_acl = $this->ui->get_permissions($dn,"fai/faiScriptEntry","FAIscript")  ;
      if(($this->SubObjects[$key]['status'] == "new") || ($this->SubObjects[$key]['dn'] == "new") || !preg_match("/r/",$s_acl)){
        $down = "";
      }else{
        $down = "<a href='?plug=".$_GET['plug']."&getFAIscript=".base64_encode($key)."'>
          <img src='images/save.png' alt='"._("Download")."' title='"._("Download")."' border=0>
          </a>"; 
      } 

      /* Check if we are allowed to view this object */
      $s_acl = $this->ui->get_permissions($dn,"fai/faiScriptEntry","cn")  ;
      if(preg_match("/r/",$s_acl)){
        $divlist->AddEntry(array( array("string"=>"<a href='?plug=".$_GET['plug']."&amp;act=edit&amp;id=".base64_encode($key)."'>".$name['name']."</a>"),
              array("string"=>$name['FAIpriority'] , "attach" => "style='width:100px;'"),
              array("string"=>$down , "attach" => "style='width:100px;'"),
              array("string"=>str_replace("%s",base64_encode($key),$act),
                "attach"=>"style='border-right: 0px;width:100px;text-align:right;'")));
      }
    }
    $smarty->assign("Entry_divlist",$divlist->DrawList());

    /* Magic quotes GPC, escapes every ' " \, to solve some security risks
     * If we post the escaped strings they will be escaped again
     */
    foreach($this->attributes as $attrs){
      if(get_magic_quotes_gpc()){
        $smarty->assign($attrs,stripslashes($this->$attrs));
      }else{
        $smarty->assign($attrs,($this->$attrs));
      }
    }

    $dn = $this->acl_base_for_current_object($this->dn);
    $smarty->assign("sub_object_is_addable",  
        preg_match("/c/",$this->ui->get_permissions($dn,"fai/faiScriptEntry")) && 
        !preg_match("/freeze/",$this->FAIstate));

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translated){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    $display.= $smarty->fetch(get_template_path('faiScript.tpl', TRUE));
    return($display);
  }


  /* Generate listbox friendly SubObject list
   */
  function getList($use_dns=false){
    $a_return=array();
    foreach($this->SubObjects as $obj){
      if($obj['status'] != "delete"){

        $cn   = stripslashes($obj['cn']);
        $desc = "";

        if((isset($obj['description']))&&(!empty($obj['description']))){
          $desc = " [".stripslashes($obj['description'])."]";
        }

        if($use_dns){
          $a_return[$obj['cn']]['name']= $cn.$desc;
          $a_return[$obj['cn']]['dn']= $obj['dn'];
          $a_return[$obj['cn']]['FAIpriority']= $obj['FAIpriority'];
        }else{
          $a_return[$obj['cn']] =  $cn.$desc;
        }
      }
    }
    return($a_return);
  }


  /* Delete me, and all my subtrees
   */
  function remove_from_parent()
  {
    if($this->acl_is_removeable()){
      $ldap = $this->config->get_ldap_link();
      $ldap->cd ($this->dn);
      $release = $this->parent->parent->fai_release;
      $use_dn = preg_replace("/".preg_quote(FAI::get_release_dn($this->dn), '/')."/i", $release, $this->dn);
      new log("remove","fai/".get_class($this),$use_dn,$this->attributes);
      FAI::prepare_to_save_FAI_object($use_dn,array(),true);
 
      foreach($this->SubObjects as $name => $obj){
        $use_dn = preg_replace("/".preg_quote(FAI::get_release_dn($this->dn), '/')."/i", $release, $obj['dn']);
        FAI::prepare_to_save_FAI_object($use_dn,array(),true);
      }
      $this->handle_post_events("remove");
    }
  }


  /* Save data to object 
   */
  function save_object()
  {
    if((isset($_POST['FAIscript_posted'])) && !preg_match("/freeze/", $this->FAIstate)){
      plugin::save_object();
    }
    
    /* Get sort order */
    if(isset($_GET['sort']) && in_array($_GET['sort'],array("name","priority"))){
      if($this->sort_by == $_GET['sort']){
        if($this->sort_order == "up"){
          $this->sort_order = "down";
        }elseif($this->sort_order == "down"){
          $this->sort_order = "up";
        }
      }
      $this->sort_by = $_GET['sort'];
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* Ensure that we do not overwrite an allready existing entry 
     */
    if($this->is_new){
      $release = $this->parent->parent->fai_release;
      $new_dn= 'cn='.$this->cn.",".get_ou('faiScriptRDN').get_ou('faiBaseRDN').$release;
      $res = faiManagement::check_class_name("FAIscript",$this->cn,$new_dn);
      if(isset($res[$this->cn])){
        $message[] = msgPool::duplicated(_("Name"));
      }
    }

    return ($message);
  }


  /* Save to LDAP */
  function save()
  {
    plugin::save();

    $ldap = $this->config->get_ldap_link();

    FAI::prepare_to_save_FAI_object($this->dn,$this->attrs);

    if($this->initially_was_account){
      new log("modify","fai/".get_class($this),$this->dn,$this->attributes);
    }else{
      new log("create","fai/".get_class($this),$this->dn,$this->attributes);
    }

    /* Prepare FAIscriptEntry to write it to ldap
     * First sort array.
     *  Because we must delete old entries first.
     * After deletion, we perform add and modify 
     */
    $Objects = array();

    /* We do not need to save untouched objects */
    foreach($this->SubObjects as $name => $obj){
      if($obj['status'] == "FreshLoaded"){
        unset($this->SubObjects[$name]);
      }
    }

    foreach($this->SubObjects as $name => $obj){
      if($obj['status'] == "delete"){
        $Objects[$name] = $obj; 
      }
    }
    foreach($this->SubObjects as $name => $obj){
      if($obj['status'] != "delete"){
        $Objects[$name] = $obj; 
      }
    }

    foreach($Objects as $name => $obj){

      foreach($this->sub64coded as $codeIt){
        $obj[$codeIt]=base64_encode(stripslashes($obj[$codeIt]));
      }

      $tmp = array();
      $attributes = array_merge($this->sub_Load_Later,$this->subAttributes);
      foreach($attributes as $attrs){
        if(!isset($obj[$attrs])) continue; 
        if($obj[$attrs] == ""){
          $obj[$attrs] = array();
        }
        if(!is_array($obj[$attrs])){
          $tmp[$attrs] = stripslashes($obj[$attrs]);
        }else{
          $tmp[$attrs] = $obj[$attrs];
        }
      }    

      $tmp['objectClass'] = $this->subClasses;

      $sub_dn = "cn=".$obj['cn'].",".$this->dn;

      if($obj['status']=="new"){
        $ldap->cat($sub_dn,array("objectClass"));
        if($ldap->count()){
          $obj['status']="edited";
        }
      }

      if(empty($tmp['FAIpriority'])){
        $tmp['FAIpriority']  ="0";
      }

      /* Tag object */
      $this->tag_attrs($tmp, $sub_dn, $this->gosaUnitTag);

      if($obj['status'] == "delete"){
        FAI::prepare_to_save_FAI_object($sub_dn,array(),true);
        $this->handle_post_events("remove");
      }elseif($obj['status'] == "edited"){
        FAI::prepare_to_save_FAI_object($sub_dn,$tmp);
        $this->handle_post_events("modify");
      }elseif($obj['status']=="new"){
        FAI::prepare_to_save_FAI_object($sub_dn,$tmp);
        $this->handle_post_events("add");
      }
    }
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    /* Read all leaf objects of this object (For FAIscript this would be FAIscriptEntry)
     */
    $res = FAI::get_all_objects_for_given_base($source['dn'],"(&(objectClass=FAIclass)(objectClass=".$this->subClass."))");
    foreach($res as $obj){

      /* Skip not relevant objects */
      if(!preg_match("/".preg_quote($source['dn'], '/')."$/i",$obj['dn'])) continue;

      $objects = array();
      $objects['status']      = "edited";
      $objects['dn']          = $obj['dn'];
      $objects                = $this->get_object_attributes($objects,$this->subAttributes);
      $objects                = $this->get_object_attributes($objects,$this->sub_Load_Later);
      $this->SubObjects[$objects['cn']] = $objects;
    }
  }


  /*! \brief  Used for copy & paste.
              Returns a HTML input mask, which allows to change the cn of this entry.
      @param  Array   Array containing current status && a HTML template.
   */
  function getCopyDialog()
  {
    $vars = array("cn");
    $smarty = get_smarty();
    $smarty->assign("cn", htmlentities($this->cn));
    $str = $smarty->fetch(get_template_path("paste_generic.tpl",TRUE));
    $ret = array();
    $ret['string'] = $str;
    $ret['status'] = "";
    return($ret);
  }


  /*! \brief  Used for copy & paste.
              Some entries must be renamed to avaoid duplicate entries.
   */
  function saveCopyDialog()
  {
    if(isset($_POST['cn'])){
      $this->cn = get_post('cn');
    }
  }
  

  /* Return plugin informations for acl handling */ 
  static function plInfo()
  {
    return (array( 
          "plShortName" => _("Script"),
          "plDescription" => _("FAI script"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 18,
          "plSection"     => array("administration"),
          "plCategory"    => array("fai"),
          "plProvidedAcls" => array(
            "cn"                => _("Name")." ("._("Readonly").")",
            "description"       => _("Description"))
          ));
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
