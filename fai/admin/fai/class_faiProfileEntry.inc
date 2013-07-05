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

class faiProfileEntry extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes   = array();
  var $objectclasses= array();

  /* Array with available class names */
  var $FAIAllclasses        = array();
  var $ui;

  function faiProfileEntry (&$config, $dn= NULL,$used=array(),$base)
  {
    parent::__construct($config, $dn);

    /* Search only in fai tree */
    $this->ui = get_userinfo();

    $filter = "(|(objectClass=FAIscript)(objectClass=FAItemplate)(objectClass=FAIhook)".
                "(objectClass=FAIvariable)(objectClass=FAIpartitionTable)(objectClass=FAIpackageList))";

    $FAI_objects = FAI::get_all_objects_for_given_base($base,$filter,true);

    /* Create array with categories to be able to sort different types of objects */
    $categories = array("FAIscript"         => "faiScript",
                        "FAItemplate"       => "faiTemplate",
                        "FAIhook"           => "faiHook",
                        "FAIvariable"       => "faiVariable",
                        "FAIpartitionTable" => "faiPartitionTable",
                        "FAIpackageList"    => "faiPackage");

    /* Read out all objects from fai tree */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($base);
    $sort=array();
    foreach($FAI_objects as $obj){

      $ldap->cat($obj['dn']);
      $attrs = $ldap->fetch();

      /* Only use objects which have cn set */
      if((isset($attrs['cn'][0]))&&(!in_array($attrs['cn'][0],$used))){
        foreach($categories as $cat => $acl){
          if(in_array($cat,$attrs['objectClass'])){

            $acl =  $this->ui->get_permissions($attrs['dn'],"fai/".$acl);

            /* Append different types of objects */
            $this->FAIAllclasses[$attrs['cn'][0]]['objects'][$cat]=$cat;

            if(!isset($this->FAIAllclasses[$attrs['cn'][0]]['acl'])){
              $this->FAIAllclasses[$attrs['cn'][0]]['acl']="";
            }
            $this->FAIAllclasses[$attrs['cn'][0]]['acl'].=$acl;

            /* Set selected status to false */
            $this->FAIAllclasses[$attrs['cn'][0]]['status']=false;

            $sort[strtolower($attrs['cn'][0])] = $attrs['cn'][0];
          }
        }
      }
    }


    $tmp = array();
    ksort($sort);
    foreach($sort as $name){
      $tmp[$name] =$this->FAIAllclasses[$name];
    }
    $this->FAIAllclasses = array();
    $this->FAIAllclasses = $tmp;

    /* If no search filter is set, create one */
    if (!session::is_set("SUBfaifilter")){
      $SUBfaifilter= array("base" => get_ou('faiBaseRDN').session::get('CurrentMainBase'), "Sregex" => "*");
      $SUBfaifilter['SShowTemplates'] = true;
      $SUBfaifilter['SShowScripts']   = true;
      $SUBfaifilter['SShowHooks']     = true;
      $SUBfaifilter['SShowVariables'] = true;
      $SUBfaifilter['SShowPackages']  = true;
      $SUBfaifilter['SShowPartitions']= true;
      session::set("SUBfaifilter", $SUBfaifilter);
    }
  }

  function execute()
  {
    /* Call parent execute */
    parent::execute();

    /* Fill templating stuff */
    $smarty     = get_smarty();
    $display = "";

    $SUBfaifilter  = session::get('SUBfaifilter');

    /* Get alphabet selection */
    if(isset($_GET['search'])){
      $SUBfaifilter['Sregex'] = strtolower($_GET['search'])."*";
    }

    /* If someone used the apply button, update searchfilter */
    if(isset($_POST['Sregex'])){
      foreach($SUBfaifilter as $key => $val){
        if(!isset($_POST[$key])){
          $SUBfaifilter[$key] = false;
        }else{
          $SUBfaifilter[$key] =$_POST[$key];
        }
      }
      $SUBfaifilter['Sregex']= $_POST['Sregex'];
      if(!strstr($SUBfaifilter['Sregex'],"*")){
        $SUBfaifilter['Sregex'] = $SUBfaifilter['Sregex']."*";
      }
    }

    /* Save current searchfilter in session*/
    session::set('SUBfaifilter',$SUBfaifilter);

    /* Check if we used a checkboxe from the list, to select dselect an entry */
    foreach($_POST as $name => $value){
      foreach($this->FAIAllclasses as $class => $obj){
        $bclass = base64_encode($class);
        if(isset($_POST["ON_PAGE_".$bclass])){
          if(isset($_POST['USE_'.$bclass])){
            $this->FAIAllclasses[$class]['status']=true;
          }else{
            $this->FAIAllclasses[$class]['status']=false;
          }
        }
      }
    }

    /* Create different picture for each object type */
    $objTypes['FAIhook']            = "<img src='plugins/fai/images/fai_hook.png' title='"._("Hook bundle")."' alt=''>&nbsp;";
    $objTypes['FAItemplate']        = "<img src='plugins/fai/images/fai_template.png' title='"._("Template bundle")."' alt=''>&nbsp;";
    $objTypes['FAIscript']          = "<img src='plugins/fai/images/fai_script.png' title='"._("Script bundle")."' alt=''>&nbsp;";
    $objTypes['FAIvariable']        = "<img src='plugins/fai/images/fai_variable.png' title='"._("Variable bundle")."' alt=''>&nbsp;";
    $objTypes['FAIpackageList']     = "<img src='plugins/fai/images/fai_packages.png' title='"._("Package bundle")."' alt=''>&nbsp;";
    $objTypes['FAIpartitionTable']  = "<img src='plugins/fai/images/fai_partitionTable.png' title='"._("Partition table")."' alt=''>&nbsp;";

    foreach($this->attributes as $attrs){
        $smarty->assign($attrs,($this->$attrs));
    }

    /* Assign all filter attributes to smarty */
    foreach($SUBfaifilter as $key => $val){
      if(($key != "base")&&($key != "Sregex")){
        if($val){
          $smarty->assign($key."CHK", " checked ");
        }else{
          $smarty->assign($key."CHK", "");
        }
      }else{
        $smarty->assign($key,$val);
      }
    }

    /* Create a divselectbox to display all available class names with the depending object types */
    $divselectbox = new divSelectBox();
    $divselectbox->SetHeader(array(_("Class name"),_("Objects")));

    /* Action button allows us to delete an entry */
    $action = "<input type='hidden' name='ON_PAGE_%KEY%' value='1'>
              <input type='checkbox' name='USE_%KEY%' value='%KEY%' onClick='document.mainform.submit();' %CHECK%>";

    /* Check every single class name to match the current search filter */
    foreach($this->FAIAllclasses as $usedClass => $classes){

      /* Skip those entries that we are not allowed to read. */
      if(!preg_match("/r/",$classes['acl'])){
        continue;
      }

      $fi = str_replace("*",".*",$SUBfaifilter['Sregex']);

      $abort = true;
      if(!preg_match("/^".$fi."/i",$usedClass)){
        $abort = false;
        continue;
      }
      if((isset($classes['objects']['FAIscript']))&&($SUBfaifilter['SShowScripts'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAItemplate']))&&($SUBfaifilter['SShowTemplates'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAIhook']))&&($SUBfaifilter['SShowHooks'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAIvariable']))&&($SUBfaifilter['SShowVariables'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAIpartitionTable']))&&($SUBfaifilter['SShowPartitions'])){
        $abort = false;
      }
      if((isset($classes['objects']['FAIpackageList']))&&($SUBfaifilter['SShowPackages'])){
        $abort = false;
      }

      if(!$abort){
        $str = "";
        foreach($classes['objects'] as $class => $obj){
          $str.= $objTypes[$obj];
        }

        /* Is this entry selected */
        if($classes['status']==true){
          $action_check = preg_replace("/%CHECK%/"," checked ",$action);
        }else{
          $action_check = preg_replace("/%CHECK%/","",$action);
        }

        /* Append to list */
        $field1 = array("html" => str_replace("%KEY%",base64_encode($usedClass),$action_check).$usedClass, "attach"=>"");
        $field2 = array("html" => $str, "attach"=>"style='border-right:0px;'");
        $divselectbox->AddEntry(array($field1,$field2));
      }
    }

    $smarty->assign("faihead"       , "");
    $smarty->assign("failist"       , $divselectbox->DrawList());
    $smarty->assign("infoimage"     , get_template_path('images/info_small.png'));
    $smarty->assign("launchimage"   , get_template_path('images/lists/action.png'));
    $smarty->assign("alphabet"      , generate_alphabet());
    $smarty->assign("apply"         , apply_filter(TRUE));
    $smarty->assign("search_image"  , get_template_path('images/lists/search.png'));

    $display .= $smarty->fetch(get_template_path('faiProfileEntry.tpl', TRUE));
    return $display;
  }

  /* Save data to object */
  function save_object()
  {
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    return ($message);
  }

  function save()
  {
    $tmp = array();
    foreach($this->FAIAllclasses as $class => $obj){
      if($obj['status']==true){
        $tmp[$class]=$class;
      }
    }
    return($tmp);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
