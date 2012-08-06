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

class application extends plugin
{
  /* application attributes */
  var $cn= "";
  var $description= "";
  var $base= "";
  var $gosaApplicationExecute= "";
  var $gosaApplicationName= "";
  var $gosaApplicationFlags= "";
  var $gosaApplicationIcon= "";
  var $gotoLogonScript  ="";
  var $iconData;
  var $view_logged = FALSE;

  /* Headpage attributes */
  var $last_sorting= "invalid";
  var $applications= array();

  var $orig_base = "";
  var $orig_cn = "";
  var $orig_dn = "";

  /* attribute list for save action */
  var $attributes= array("cn", "description", "gosaApplicationExecute", "gosaApplicationName","gosaApplicationIcon",
      "gosaApplicationFlags","gotoLogonScript");

  var $objectclasses= array("top", "gosaApplication");
  var $FAIstate ="";

  function application (&$config, $dn= NULL, $parent= NULL)
  {
    plugin::plugin ($config, $dn, $parent);

    /* Load icon */
    $ldap= $config->get_ldap_link();
    $this->orig_dn = $dn;
    if ($dn != 'new'){
      $this->iconData= $ldap->get_attribute($dn, "gosaApplicationIcon");
      $this->saved_attributes['gosaApplicationIcon'] = $this->iconData;
    }
    if ($this->iconData == ""){
      $this->set_picture("");
    }
    session::set('binary',$this->iconData);
    session::set('binarytype',"image/jpeg");
    $this->gosaApplicationIcon= $this->iconData;

    /* This is always an account */
    $this->is_account= TRUE;

    if ($this->dn == "new"){
      if(session::is_set('CurrentMainBase')){
        $this->base = session::get('CurrentMainBase');
      }else{
        $ui= get_userinfo();
        $this->base= dn2base($ui->dn);
      }
    } else {
      $this->base= preg_replace ("/^[^,]+,".preg_quote(get_ou('applicationRDN'), '/')."/i", "", $this->dn);
    }

    $this->orig_cn = $this->cn;
    $this->orig_base = $this->base;
    $this->gosaApplicationFlags = preg_replace("/[^GDLMO]/","",$this->gosaApplicationFlags);
  }


  function generateTemplate()
  {
    $str= "# This code is part of FusionDirectory http://www.fusiondirectory.org/\n#\n";

    $values = array();
    $names  = array();
    if($this->parent->by_object['applicationParameters']->is_account){
      $names = $this->parent->by_object['applicationParameters']->option_name;
      $values = $this->parent->by_object['applicationParameters']->option_value;
    }

    if (count($names)){
      $str .="# This plugin handles these environment variables:\n";
    } else {
      $str .="# This plugin handles no environment variables.\n";
    }

    foreach($names as $index => $name){

      // Fix length
      for($i = strlen($name) ; $i < 30 ; $i++){
        $name= $name." ";
      }
      if((isset($values[$index]))&&(!empty($values[$index]))){
        $str.= "# ".$name."\t(e.g. '".$values[$index]."')\n";
      }else{
        $str.= "# ".$name."\t("._("no example").")\n";
      }
    }
    $str .= "#\n".
      "# Don't remove the following tag, it is used for header update.\n".
      "### END HEADER ###";

    return($str);
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Log view */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","application/".get_class($this),$this->dn);
    }

    $smarty= get_smarty();

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name, preg_match("/freeze/i",$this->FAIstate)));
    }
    $smarty->assign("IconReadable", $this->acl_is_readable("gosaApplicationIcon"));

    /* Do we represent a valid group? */
    if (!$this->is_account && $this->parent === NULL){
      $display= "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
        msgPool::noValidExtension(_("application"))."</b>";
      return ($display);
    }

    /* Download requested */
    foreach($_POST as $name => $value){
      if(preg_match("/^downloadScript/",$name)){
        session::set('binary',$this->gotoLogonScript);
        session::set('binarytype',"octet-stream");
        session::set('binaryfile',$this->cn.".gosaApplication");
        header("location: getbin.php ");
        exit();
      }
    }

    /* Reassign picture data, sometimes its corrupt cause we started a download of application scripts */
    session::set('binary',$this->iconData);
    session::set('binarytype',"image/jpeg");

    $smarty->assign("rand", rand(0, 10000));
    $head = $this->generateTemplate();
    $this->gotoLogonScript= $this->generateTemplate().preg_replace('/.*### END HEADER ###/s', '', $this->gotoLogonScript);

    if((isset($_POST['upLoad']))&&(isset($_FILES['ScriptFile'])) && !preg_match("/freeze/i",$this->FAIstate)){
      $str = file_get_contents($_FILES['ScriptFile']['tmp_name']);
      $this->gotoLogonScript = $str;
    }

    /* Fill templating stuff */
    $smarty->assign("cn", $this->cn);
    if(!$this->is_release()){
      $smarty->assign("bases", $this->get_allowed_bases());
    }else{
      $smarty->assign("bases", array());
    }
    if ($this->dn == "new"){
      $smarty->assign("selectmode", "");
      $smarty->assign("namemode", "");
    } else {
      $smarty->assign("namemode", "readonly");
      $smarty->assign("selectmode", "disabled");
    }

    /* Get random number for pictures */
    srand((double)microtime()*1000000);
    $smarty->assign("rand", rand(0, 10000));

    /* Variables */
    foreach(array("description", "gosaApplicationExecute", "gosaApplicationName","cn") as $val){
      $smarty->assign($val, $this->$val);
    }

    /* Checkboxes */
    foreach (array("G" => "execForGroupmembers", "O" => "overwriteConfig",
          "L" => "placeOnKicker",
          "D" => "placeOnDesktop", "M" => "placeInStartmenu") as $key => $val){
      if (preg_match("/$key/", $this->gosaApplicationFlags) && $this->acl_is_readable("$val")){
        $smarty->assign("$val", "checked");
      } else {
        $smarty->assign("$val", "");
      }
    }

    $smarty->assign("isReleaseApplikation" ,$this->is_release());
    $smarty->assign("gotoLogonScript",htmlentities($this->gotoLogonScript, ENT_COMPAT, 'UTF-8'));
    $smarty->assign("base_select", $this->base);
    /* Show main page */
    return($smarty->fetch (get_template_path('generic.tpl', TRUE)));
  }


  function remove_from_parent()
  {
    restore_error_handler();

    /* Parse release out of object dn */
    $release = preg_replace("/".preg_quote(get_ou("applicationRDN").$this->base, '/')."$/i","",$this->dn);
    $release = preg_replace("/^cn=[^,]+,/","",$release);

    /* Get a list of all groups
     */
    $groups = array();
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=posixGroup)",array("dn"));
    while($attrs = $ldap->fetch()){
      $groups[$attrs['dn']] = array();
    }

    /* Check if there are groups, useing this application
     */
    $found = array();
    foreach($groups as $group => $data){
      $ldap->cd($release.$group);
      $ldap->search("(objectClass=gotoMenuEntry)",array("dn"));
      while($attrs = $ldap->fetch()){
        $info = preg_replace("/".preg_quote($release.$group, '/')."$/","",$attrs['dn']);
        if(preg_match("/^cn=".$this->cn."/",$info) && !preg_match("/ou=[^,]+,/",$info)){
          $found[] = $attrs['dn'];
        }
      }
    }

    /* Create an error message an skip remove, if
        this application is still in use.
     */
    if(count($found)){
      msg_dialog::display(_("Error"), sprintf(_("Cannot remove application - it is still in use by these objects: %s"), "<br>".msgPool::buildList($found)), ERROR_DIALOG);
      return(FALSE);
    }

    $ldap->rmDir($this->dn);
    new log("remove","application/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()), ERROR_DIALOG);
    }

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
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()), ERROR_DIALOG);
      }
    }
  }


  /* Save data to object */
  function save_object()
  {
    if(preg_match("/freeze/i",$this->FAIstate)){
      return;
    }

    if (isset($_POST['cn'])){

      /* Create a base backup and reset the
         base directly after calling plugin::save_object();
         Base will be set seperatly a few lines below */
      $base_tmp = $this->base;
      plugin::save_object();
      $this->base = $base_tmp;

      /* Save attributes */
      parent::save_object();

      $checks = array("execForGroupmembers" => "G",
                      "placeOnDesktop"      => "D",
                      "placeOnKicker"       => "L",
                      "placeInStartmenu"    => "M",
                      "overwriteConfig"     => "O");


      /* Save application flags */
      foreach($checks as $name => $flag){
        if($this->acl_is_writeable($name)){
          $set = isset($_POST[$name]) && $_POST[$name] == 1;
          if(!$set && strstr($this->gosaApplicationFlags,$flag)){
            $this->gosaApplicationFlags = str_replace($flag,"",$this->gosaApplicationFlags);
          }elseif($set && !preg_match("/".$flag."/",$this->gosaApplicationFlags)){
            $this->gosaApplicationFlags .= $flag;
          }
        }
      }

      /* Remove current picture */
      if(isset($_POST['remove_picture'])){
        $this->set_picture("");
      }

      /* Check for picture upload */
      if (isset($_FILES['picture_file']['name']) && $_FILES['picture_file']['name'] != ""){

        if (!is_uploaded_file($_FILES['picture_file']['tmp_name'])) {
          msg_dialog::display(_("Error"), msgPool::incorrectUpload(), ERROR_DIALOG);
        }

        /* Load the new uploaded Photo */
        if(!$handle  =  imagick_ReadImage($_FILES['picture_file']['tmp_name'])){
          msg_dialog::display(_("Error"), msgPool::incorrectUpload(_("no read permission")), ERROR_DIALOG);
        }

        /* Resizing image to 147x200 and blur */
        if(!imagick_resize($handle,48,48,IMAGICK_FILTER_GAUSSIAN,0)){
          msg_dialog::display(_("Error"), msgPool::incorrectUpload(_("cannot resize image")), ERROR_DIALOG);
        }

        /* Converting image to JPEG */
        if(!imagick_convert($handle,"PNG")) {
          msg_dialog::display(_("Error"), msgPool::incorrectUpload(_("cannot convert image")), ERROR_DIALOG);
        }

        if(!imagick_writeimage($handle,$_FILES['picture_file']['tmp_name'])){
          msg_dialog::display(_("Error"), msgPool::incorrectUpload(sprintf(_("cannot save image to '%s'"), $_FILES['picture_file']['tmp_name'])), ERROR_DIALOG);
        }

        imagick_free($handle);

        /* Activate new picture */
        $this->set_picture($_FILES['picture_file']['tmp_name']);
      }

      if(!$this->is_release()){
        $tmp = $this->get_allowed_bases();
        if(isset($_POST['base'])){
          if(isset($tmp[$_POST['base']])){
            $this->base= $_POST['base'];
          }
        }
      }
    }
  }


  /* Check values */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* Permissions for that base? */
    if ($this->base != ""){
      $new_dn= "cn=".$this->cn.",".get_ou('applicationRDN').$this->base;
    } else {
      $new_dn= $this->dn;
    }


    if($this->dn == "new"){
      $this->set_acl_base($this->base);
    }

    /* All required fields are set? */
    if ($this->cn == ""){
      $message[]= msgPool::required(_("Name"));
    }

    if(preg_match("/[^a-z0-9_-]/",$this->cn)) {
      $message[]=msgPool::invalid(_("Name"),$this->cn,"/^[a-z0-9_-]*$/");
    }

    if ($this->gosaApplicationExecute == ""){
      $message[]= msgPool::required(_("Execute"));
    }

    /* Check for existing application */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current["BASE"]);

    $tmp = $this->config->search("faiManagement", "CLASS",array('menu','tabs'));

    if($this->is_release()){
      $base = $this->parent->parent->app_release;
    }else{
      $base = get_ou('applicationRDN').$this->base;
    }

    $ldap->ls("(&(objectClass=gosaApplication)(cn=".$this->cn."))",$base,array("cn"));
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

    return $message;
  }


  /* Save to LDAP */
  function save()
  {
    /* Get application script without header part, to check if we must save the script itself */
    $script = preg_replace('/.*### END HEADER ###/s', '', $this->gotoLogonScript);

    $this->gosaApplicationFlags ="[{$this->gosaApplicationFlags}]";
    plugin::save();
    $this->attrs["gosaApplicationIcon"]= $this->gosaApplicationIcon;

    /* Write back to ldap */
    $ldap= $this->config->get_ldap_link();
    $ldap->cat($this->dn, array('dn'));

    $a= $ldap->fetch();
    if (count($a)){

      /* Remove gotoLogonScript if it is empty */
      if(empty($script))    {
        $this->attrs['gotoLogonScript'] = array();
      }

      $ldap->cd($this->dn);
      $this->cleanup();
      $ldap->modify ($this->attrs);
      $this->handle_post_events("modify");
      new log("modify","application/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    } else {

      /* Remove gotoLogonScript if it is empty */
      if(empty($script))    {
        unset($this->attrs['gotoLogonScript']);
      }

      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
      new log("create","application/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      $this->handle_post_events("add");
    }
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()), ERROR_DIALOG);
    }
  }


  function is_release()
  {
    if(isset($this->parent->parent)){
      return($this->parent->parent->IsReleaseManagementActivated());
    }else{
      /* Check if we should enable the release selection */
      $tmp = $this->config->search("faiManagement", "CLASS",array('menu','tabs'));
      if(!empty($tmp)){
        return(true);
      }
    }
    return(FALSE);
  }


  function set_picture($filename)
  {
    if (!is_file($filename)){
      $filename= "./plugins/systems/images/default_icon.png";
      $this->gosaApplicationIcon= "*removed*";
    }

    if (file_exists($filename)){
      $fd = fopen ($filename, "rb");
      $this->iconData= fread ($fd, filesize ($filename));
      session::set('binary',$this->iconData);
      session::set('binarytype',"image/jpeg");
      $this->gosaApplicationIcon= $this->iconData;

      fclose ($fd);
    }
  }

  function getCopyDialog()
  {
    $vars = array("cn");

    $str ="<h2>"._("Application settings")."</h2>
      <table>
      <tr>
      <td>".
      _("Application name").
      "</td>
      <td>
      <input id='gosaApplicationName' name='cn' size='35' maxlength='60'
      value='".$this->cn."'
      title='"._("Application name to be displayed (i.e. below icons)")."'>
      </td>
      </tr>
      </table>";
    $ret = array();
    $ret['status'] = "";
    $ret['string'] = $str;
    return($ret);
  }

  function saveCopyDialog()
  {
    if(isset($_POST['cn'])){
      $this->cn = $_POST['cn'];
    }
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);
    $source_o = new application($this->config,$source['dn']);
    $this->gosaApplicationIcon = $source_o->gosaApplicationIcon;
  }


  /* Return plugin informations for acl handling
      #FIXME FAIscript seams to ununsed within this class... */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Generic"),
          "plDescription" => _("Application generic"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("administration"),
          "plCategory"    => array("application" => array("description"  => _("Application"),
                                                          "objectClass"  => "gosaApplication")),
          "plProvidedAcls"=> array(
            "cn"                => _("Name"),
            "base"              => _("Base"),
            "description"       => _("Description"),
            "gosaApplicationExecute"  => _("Execute"),
            "gosaApplicationName"     => _("Name"),
            "gosaApplicationIcon"     => _("Icon"),
            "gotoLogonScript"         => _("Script content"),

            "execForGroupmembers" => _("Only executable for members"),              // G
            "placeOnDesktop"      => _("Place icon on members desktop"),            // D
            "placeOnKicker"       => _("Place entry in members launch bar"),        // L
            "placeInStartmenu"    => _("Place entry in members startmenu"),         // M
            "overwriteConfig"      => _("Replace user configuration on startup"))  // O
            ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
