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

class faiProfile extends plugin
{
  /* attribute list for save action */
  var $ignore_account   = TRUE;

  /* Attributes for this Object */
  var $attributes       = array("cn","description","FAIclass");

  /* ObjectClasses for this Object*/
  var $objectclasses    = array("top","FAIclass","FAIprofile");

  /* Specific attributes */
  var $old_cn           = "";
  var $cn               = "";       // The class name for this object
  var $description      = "";       // The description for this set of partitions
  var $is_dialog        = false;    // specifies which buttons will be shown to save or abort
  var $FAIclass         = "";       // Classnames used for this profile
  var $FAIclasses       = array();  // Contains classname seperated in an array
  var $FAIAllclasses    = array();  // Contains all possible Classnames
  var $ui               ;
  var $FAIstate      = "";
  var $view_logged      = FALSE;

  var $classSelect;

  function faiProfile(&$config, $dn= NULL)
  {
    /* Load Attributes */
    plugin::plugin ($config, $dn);

    $this->ui = get_userinfo();

    /* Parse ldap attribute to get all assigned classes */
    $tmp = explode(" ",$this->FAIclass);
    $tmp2 = array();
    foreach($tmp as $class){
      if(!empty($class)){
        $tmp2[trim($class)] = trim($class);
      }
    }

    /* Sort assigned classes */
    if(is_array($tmp2)){
      foreach($tmp2 as $class){
        $this->FAIclasses[$class]=$class;
      }
    }

    if($dn != "new"){
      $this->dn =$dn;
    }
    $this->old_cn   = $this->cn;
  }


  /* Combine new array, used for up down buttons */
  function combineArrays($ar0,$ar1,$ar2)
  {
    $ret = array();
    if(is_array($ar0))
      foreach($ar0 as $ar => $a){
        $ret[$ar]=$a;
      }
    if(is_array($ar1))
      foreach($ar1 as $ar => $a){
        $ret[$ar]=$a;
      }
    if(is_array($ar2))
      foreach($ar2 as $ar => $a){
        $ret[$ar]=$a;
      }
    return($ret);
  }

  function get_all_fai_classes()
  {
    if (count($this->FAIAllclasses)) {
      return;
    }

    $ldap = $this->config->get_ldap_link();
    $categories = array("FAIscript","FAItemplate","FAIhook","FAIvariable","FAIpartitionTable","FAIpackageList");

    /* Build filter */
    $filter= "";
    foreach ($categories as $cat){
      $filter.= "(objectClass=$cat)";
    }

    /* Get ldap connection */
    $base  = session::get('CurrentMainBase');
    $ldap->cd($base);
    $sort = array();

    /* search all FAI classes */
    $res = FAI::get_all_objects_for_given_base($this->parent->parent->fai_release, "(|$filter)", true);
    foreach($res as $attrs){

    /* Sort by categorie */
    foreach($categories as $cat){
      if(in_array($cat,$attrs['objectClass'])) {

        /* Append entry */
        $this->FAIAllclasses[$attrs['cn'][0]][$cat]=$attrs;

        /* Create sort array, because the array above is a multidimensional array, and can't be sorted by php sorting functions*/
        $sort[strtolower($attrs['cn'][0])] = $attrs['cn'][0];
        }
      }
    }

    /* Sort the sort array */
    //ksort($sort);

    /* Reorder the FAIclasses array */
    foreach($sort as $name){
      $tmp[$name] =$this->FAIAllclasses[$name];
    }

    /* Assign sorted classes */
    $this->FAIAllclasses = array();
    $this->FAIAllclasses = $tmp;
  }

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


  /* returns position in array */
  function getpos($atr,$attrs)
  {
    $i = 0;
    foreach($attrs as $attr => $name) {
      $i++;
      if($attr == $atr){
        return($i);
      }
    }
    return(-1);
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

  /* class one position up */
  function catUp($id)
  {
    /* Get all cats depinding on current dir */
    $cats = $this->FAIclasses;
    $this->FAIclasses =$this->ArrayUp($id,$cats);
  }

  /* Class one position down */
  function catDown($id)
  {
    /* Get all cats depinding on current dir */
    $cats = $this->FAIclasses;
    $this->FAIclasses =$this->ArrayDown($id,$cats);
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

    $s_entry = "";
    $s_action = "";

    /* Remove class name From list */
    $sort_once = false;
    if(!preg_match("/freeze/",$this->FAIstate)){
      foreach($_POST as $name => $post){
        if(preg_match("/DEL_/i",$name) && $this->acl_is_writeable("FAIclass")){
          $s_action = "delete";
          $s_entry  = preg_replace("/DEL_/","",$name);
          $s_entry  = base64_decode(preg_replace("/_.*$/","",$s_entry));
        }elseif(preg_match("/Add_class/i",$name)&& $this->acl_is_writeable("FAIclass")){
          $s_action  = "add";
        }elseif(preg_match("/DelClass/i",$name) && $this->acl_is_writeable("FAIclass")){
          $s_action  = "delete";
          $s_entry = $_POST['FAIclass'];
        }elseif(preg_match("/AddClass/i",$name) && $this->acl_is_writeable("FAIclass")){
          $s_action  = "add";
        }

        /* Check if a list element should be pushed one position up */
        if((preg_match("/sortup_/",$name))&&(!$sort_once) && $this->acl_is_writeable("FAIclass")){
          $sort_once = true;
          $val = preg_replace("/sortup_/","",$name);
          $val = preg_replace("/_.*$/","",$val);
          $val = base64_decode($val);
          $this->catUp($val);
        }

        /* Check if a list element should be pushed one position down */
        if((preg_match("/sortdown_/",$name))&&(!$sort_once) && $this->acl_is_writeable("FAIclass")){
          $sort_once = true;
          $val = preg_replace("/sortdown_/","",$name);
          $val = preg_replace("/_.*$/","",$val);
          $val = base64_decode($val);
          $this->catDown($val);
        }

      }

      if($s_action == "delete" && $this->acl_is_writeable("FAIclass")){
        unset($this->FAIclasses[$s_entry]);
      }

      if($s_action == "add" && $this->acl_is_writeable("FAIclass")){
        $this->classSelect = new classSelect($this->config, get_userinfo());
        $this->dialog  =true;
      }

      /* Save Dialog */
      if(isset($_POST['classSelect_save']) && $this->classSelect instanceOf classSelect){
        $list = $this->classSelect->save();
        foreach($list as $entry){
          $class = $entry['cn'][0];
          $this->FAIclasses[$class] =$class;
        }
        $this->dialog=false;
        $this->classSelect=FALSE;
      }
    }

    /* Cancel Dialog */
    if(isset($_POST['classSelect_cancel']) && $this->classSelect instanceOf classSelect){
      $this->dialog=false;
      $this->classSelect=FALSE;
    }

    if($this->classSelect instanceOf classSelect){
      session::set('filterFAIClass_base',$this->parent->parent->fai_release);
      session::set('filterBlacklist', array('cn' => array_values($this->FAIclasses)));
      return($this->classSelect->execute());
    }

    $divlist  =new divSelectBox("Profile");
    $divlist->SetSummary(_("This list displays all assigned class names for this profile."));

    /* item images */
    $objTypes['FAIhook']            = "<img src='plugins/fai/images/fai_hook.png' title='"._("Hook bundle")."' alt=''>";
    $objTypes['FAItemplate']        = "<img src='plugins/fai/images/fai_template.png' title='"._("Template bundle")."' alt=''>";
    $objTypes['FAIscript']          = "<img src='plugins/fai/images/fai_script.png' title='"._("Script bundle")."' alt=''>";
    $objTypes['FAIvariable']        = "<img src='plugins/fai/images/fai_variable.png' title='"._("Variable bundle")."' alt=''>";
    $objTypes['FAIpackageList']        = "<img src='plugins/fai/images/fai_packages.png' title='"._("Package bundle")."' alt=''>";
    $objTypes['FAIpartitionTable']  = "<img src='plugins/fai/images/fai_partitionTable.png' title='"._("Partition table")."' alt=''>";

    /* Delete button */
    $actions = "<input type='image' src='images/lists/trash.png' title='"._("Remove class from profile")."' name='DEL_%KEY%'>";

    /* Up down buttons */
    $linkupdown = "&nbsp;<input type='image' name='sortup_%s'   alt='up'    title='"._("Up")."'   src='images/lists/sort-up.png' align='top' >";
    $linkupdown.= "<input type='image' name='sortdown_%s' alt='down'  title='"._("Down")."' src='images/lists/sort-down.png' >";

    /* Append fai classes to divlist */
    if($this->acl_is_readable("FAIclass")){
      foreach($this->FAIclasses as $usedClass){
        $str = "&nbsp;";
        $act = "";

        $this->get_all_fai_classes();

        if(isset($this->FAIAllclasses[$usedClass])){
          foreach($this->FAIAllclasses[$usedClass] as $class => $obj){
            $str.= $objTypes[$class];
          }
        }

        $field1 = array("string"=> $usedClass,"attach"=>"");
        $field2 = array("string"=> $str,"attach"=>"");
        if(!preg_match("/freeze/", $this->FAIstate) && $this->acl_is_writeable("FAIclass")){
          $field3 = array("string"=> preg_replace("/%KEY%/",base64_encode($usedClass),$actions).
              preg_replace("/%s/",base64_encode($usedClass),$linkupdown),
              "attach"=>"style='border-right:none;'");
        }else{
          $field3 = array("string"=>"&nbsp;", "attach"=>"style='border-right:none;'");
        }
        $divlist->AddEntry(array($field1,$field2,$field3));
      }
    }

    $smarty->assign("freeze", preg_match("/freeze/i",$this->FAIstate));

    $smarty->assign("FAIclasses"  ,$this->FAIclasses);
    $smarty->assign("divlist"     ,$divlist->DrawList());

    /* Magic quotes GPC, escapes every ' " \, to solve some security risks
     * If we post the escaped strings they will be escaped again
     */
    foreach($this->attributes as $attrs){
      $smarty->assign($attrs,($this->$attrs));
    }


    $dn = $this->acl_base_for_current_object($this->dn);
    $smarty->assign("sub_object_is_addable",
        preg_match("/c/",$this->ui->get_permissions($dn,"fai/faiScriptEntry")) &&
        !preg_match("/freeze/",$this->FAIstate));

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translated){
      $smarty->assign($name."ACL",$this->getacl($name, preg_match("/freeze/",$this->FAIstate)));
    }

    $display.= $smarty->fetch(get_template_path('faiProfile.tpl', TRUE));
    return($display);
  }

  function remove_from_parent()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->dn);
    $release = $this->parent->parent->fai_release;
    $use_dn = preg_replace("/".preg_quote(FAI::get_release_dn($this->dn), '/')."/i", $release, $this->dn);
    new log("remove","fai/".get_class($this),$use_dn,$this->attributes);
    FAI::prepare_to_save_FAI_object($use_dn,array(),true);
    $this->handle_post_events("remove");
  }


  /* Save data to object
   */
  function save_object()
  {
    if(!preg_match("/freeze/",$this->FAIstate)){
      plugin::save_object();
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    if(count($this->FAIclasses) == 0){
      $message[]=_("No class specified for this profile!");
    }

    if($this->cn == ""){
      $message[]= msgPool::required(_("Name"));
    }

    /* Ensure that we do not overwrite an allready existing entry
     */
    if($this->dn == "new" || $this->cn != $this->old_cn){
      $release = $this->parent->parent->fai_release;
      $new_dn= 'cn='.$this->cn.",".get_ou('faiProfileRDN').get_ou('faiBaseRDN').$release;
      $res = faiManagement::check_class_name("FAIprofile", $this->cn, $new_dn);
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

    $this->FAIclass = "";
    foreach($this->FAIclasses as $class){
      $this->FAIclass.=$class." ";
    }

    $this->attrs['FAIclass']=trim($this->FAIclass);

    /* Remove the old FAI profile, if the dn has changed.
     */
    if($this->cn != $this->old_cn && $this->old_cn != ""){

      $old_dn = preg_replace("/^cn=[^,]+,/","cn=".$this->old_cn.",",$this->dn);
      FAI::prepare_to_save_FAI_object($old_dn,array(),TRUE);
    }
    FAI::prepare_to_save_FAI_object($this->dn,$this->attrs);

    if($this->initially_was_account){
      new log("modify","fai/".get_class($this),$this->dn,$this->attributes);
    }else{
      new log("create","fai/".get_class($this),$this->dn,$this->attributes);
    }
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Profile"),
      "plDescription" => _("FAI profile"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("fai"),
      "plObjectType"  => array("faiProfile" => array("name" => _("FAI Profile"),
                                                    "filter" => "objectClass=FAIprofile")),
      "plProvidedAcls" => array(
        "cn"                => _("Name"),
        "description"       => _("Description"),
        "FAIclass"          => _("FAI classes")
      )
    );
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
