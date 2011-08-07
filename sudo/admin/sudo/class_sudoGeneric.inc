<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*! \brief Sudo generic class. Allow setting User/Host/Command/Runas
           for a sudo role object.
 */
class sudo extends plugin
{

  protected $cn= "";
  protected $description= "";

  protected $sudoUser   = array("ALL");
  protected $sudoCommand= array();
  protected $sudoHost   = array("ALL");
  protected $sudoRunAs  = array("ALL");
  protected $accessTo         = array();
  protected $trustModel       = "";

  private $is_default = FALSE;
  private $was_trust_account= FALSE;

  public $objectclasses = array("top","sudoRole");
  public $attributes    = array("cn","description","sudoUser","sudoCommand","sudoHost","sudoRunAs","accessTo","trustModel");

  public $ignore_account = TRUE;

  public $orig_dn;

  protected $trustSelect;

  /*! \brief  Returns to the base department for sudo roles.
              This department is then used to store new roles.
      @param  Object  FusionDirectory configuration object.
      @return String sudo store department
   */
  public static function get_sudoers_ou($config)
  {
    return(get_ou("sudoRDN").$config->current['BASE']);
  }

  /*! \brief  Initializes this sudo class, with all required attributes.
      @param  Object $config  FusionDirectory configuration object.
      @param  String $db      "new" or the sudo role dn.
      @return .
   */
  function sudo(&$config, $dn= NULL)
  {
    plugin::plugin ($config, $dn);

    if($this->initially_was_account){
      foreach(array("sudoUser","sudoCommand","sudoHost","sudoRunAs") as $attr){
        $this->$attr = array();
        if(isset($this->attrs[$attr])){
          $tmp = array();
          for($i = 0 ; $i < $this->attrs[$attr]['count']; $i++){
            $tmp[] = $this->attrs[$attr][$i];
          }
          $this->$attr = $tmp;
        }
      }

      /* Is this account a trustAccount? */
      if (isset($this->attrs['trustModel'])){
        $this->trustModel= $this->attrs['trustModel'][0];
        $this->was_trust_account= TRUE;
      } else {
        $this->was_trust_account= FALSE;
        $this->trustModel= "";
      }

      $this->accessTo = array();
      if (isset($this->attrs['accessTo'])){
        for ($i= 0; $i<$this->attrs['accessTo']['count']; $i++){
          $tmp= $this->attrs['accessTo'][$i];
          $this->accessTo[$tmp]= $tmp;
        }
      }

    }

    if(preg_match("/^defaults$/i",$this->cn)){
      $this->is_default = TRUE;
    }

    /* Get global filter config */
    if (!session::is_set("sysfilter")){
      $ui= get_userinfo();
      $base= get_base_from_people($ui->dn);
      $sysfilter= array( "depselect"       => $base,
          "regex"           => "*");
      session::set("sysfilter", $sysfilter);
    }

    $this->orig_dn = $this->dn;
  }


  /*! \brief  Creates the sudo generic ui. 
      @return String  The generated HTML content for this plugin. 
   */
  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /*********************
       Access control list / trust mode 
     *********************/ 

    /* Add user workstation? */
    if (isset($_POST["add_ws"])){
      $this->trustSelect= new trustSelect($this->config,get_userinfo());
      $this->dialog= TRUE;
    }

    // Add selected machines to trusted ones.
    if (isset($_POST["add_ws_finish"]) &&  $this->trustSelect){
      $trusts = $this->trustSelect->detectPostActions();
      if(isset($trusts['targets'])){

        $headpage = $this->trustSelect->getHeadpage();
        foreach($trusts['targets'] as $id){
          $attrs = $headpage->getEntry($id);
          $this->accessTo[$attrs['cn'][0]]= $attrs['cn'][0];
        }
        ksort($this->accessTo);
        $this->is_modified= TRUE;
      }
      $this->trustSelect= NULL;
      $this->dialog= FALSE;
    }


    /* Remove user workstations? */
    if (isset($_POST["delete_ws"]) && isset($_POST['workstation_list'])){
      foreach($_POST['workstation_list'] as $name){
        unset ($this->accessTo[$name]);
      }
      $this->is_modified= TRUE;
    }

    /* Add user workstation finished? */
    if (isset($_POST["add_ws_finish"]) || isset($_POST["add_ws_cancel"])){
      $this->trustSelect= NULL;
      $this->dialog= FALSE;
    }

    /* Show ws dialog */
    if ($this->trustSelect){

      // Build up blocklist
      session::set('filterBlacklist', array('cn' => array_values($this->accessTo)));
      return($this->trustSelect->execute());
    }


    /*********************
       Add users 
     *********************/ 
 
    if(isset($_POST['list_sudoUser']) && !is_object($this->dialog) && $this->acl_is_writeable("sudoUser")){
      $this->dialog =new userGroupSelect($this->config,get_userinfo());
    }
   
    /* Add selected hosts  to the sudoUser list */ 
    if(isset($_POST['userGroupSelect_save']) && $this->dialog instanceof userGroupSelect){
      if($this->acl_is_writeable("sudoUser")){
        foreach($this->dialog->save() as $entry){
          if(in_array("posixGroup",$entry['objectClass'])){
            $name = trim("%".$entry['cn'][0]);
          }elseif(isset($entry['uid'][0])){
            $name = trim($entry['uid'][0]);
          }
          if(!in_array($name,$this->sudoUser) && !in_array("!".$name,$this->sudoUser)){
            $this->sudoUser[] = $name;
          }
        }   
      }
      unset($this->dialog);
      $this->dialog = NULL;
    }    

    if(isset($_POST['userGroupSelect_cancel']) && $this->dialog instanceOf userGroupSelect){
      unset($this->dialog);
      $this->dialog = NULL;
    }    

    if($this->dialog instanceOf userGroupSelect){
      $used = array();
      foreach($this->sudoUser as $name){
        $str = preg_replace("/^!/","",$name);
        if(preg_match("/^%/", $str)){
          $used['cn'][] = preg_replace("/^%/","",$str);
        }else{
          $used['uid'][] = $str;
        }
      }

      // Build up blocklist
      session::set('filterBlacklist', $used);
      return($this->dialog->execute());
    }
 


    /*********************
       Add systems 
     *********************/ 
  
    if(isset($_POST['list_sudoHost']) && !is_object($this->dialog) && $this->acl_is_writeable("sudoHost")){
      $this->dialog =new systemSelect($this->config,get_userinfo());
    }
   
    /* Add selected hosts  to the sudoHost list */ 
    if(isset($_POST['systemSelect_save']) && $this->dialog instanceof systemSelect){
      if($this->acl_is_writeable("sudoHost")){
        foreach($this->dialog->save() as $entry){
          $cn = trim($entry['cn'][0]);
          if(!in_array($cn,$this->sudoHost) && !in_array("!".$cn,$this->sudoHost)){
            $this->sudoHost[] = $cn;
          }
        }   
      }   
      unset($this->dialog);
      $this->dialog = NULL;
    }    

    if(isset($_POST['systemSelect_cancel']) && $this->dialog instanceOf systemSelect){
      unset($this->dialog);
      $this->dialog = NULL;
    }    

    if($this->dialog instanceOf systemSelect){
      $used = array();
      foreach($this->sudoHost as $name){
        $used['cn'][] = preg_replace("/^!/","",$name);
      }

      // Build up blocklist
      session::set('filterBlacklist', $used);
      return($this->dialog->execute());
    }
  
    /*********************
       Dialog handling / display / close  
     *********************/ 
  
    if(is_object($this->dialog)){
      return($this->dialog->execute());
    }

 
    /*********************
       NEGATE values 
     *********************/ 
    foreach($_POST as $name => $value){
      if(preg_match("/^neg_/",$name)){
        $attr = preg_replace("/^neg_([^_]*)_.*$/","\\1",$name);
        $value= preg_replace("/^neg_[^_]*_([0-9]*)_.*$/","\\1",$name);
 
        if($this->acl_is_writeable($attr)){
          $attrs = $this->$attr;
          if(isset( $attrs[$value])){
            $v =  $attrs[$value];
            if(preg_match("/^!/",$v)){
              $attrs[$value] = preg_replace("/^!/","",$v);
            }else{
              $attrs[$value] = "!".$v;
            }
            $this->$attr = $attrs;  
          }
        }
        break; // Do it once, image inputs will be posted twice
      }
    }
  
    /*********************
       Delete values 
     *********************/ 
    foreach($_POST as $name => $value){
      if(preg_match("/^del_/",$name)){
        $attr = preg_replace("/^del_([^_]*)_.*$/","\\1",$name);
        $value= preg_replace("/^del_[^_]*_([0-9]*)_.*$/","\\1",$name);
        if($this->acl_is_writeable($attr)){
          $attrs = $this->$attr;
          if(isset( $attrs[$value])){
            unset($attrs[$value]);
            $this->$attr = $attrs;  
          }
        }
        break; // Do it once, image inputs will be posted twice
      }
    }


    /*********************
       ADD values 
     *********************/

    /* User / Host / Runas */ 
    foreach(array("sudoUser","sudoHost","sudoRunAs") as $attr){
        if($this->acl_is_writeable($attr) && 
                isset($_POST["add_".$attr]) && 
                isset($_POST['new_'.$attr]) && 
                !empty($_POST['new_'.$attr])){

            $c = preg_quote(' *+-?_|!\'"()','/');
            if(preg_match("/^[a-z0-9{$c}]*$/i",$_POST['new_'.$attr])){
                $attrs = $this->$attr;
                $attrs[] =  trim($_POST['new_'.$attr]);
                $this->$attr = $attrs;
            }else{
                msg_dialog::display(_("Error"),msgPool::invalid($attr,$_POST['new_'.$attr],"/[a-z0-9{$c}]/i"));
            }
        }
    }

    /* Command */
    foreach(array("sudoCommand") as $attr){
      if($this->acl_is_writeable($attr) && isset($_POST["add_".$attr]) && isset($_POST['new_'.$attr])){
        $attrs = $this->$attr;
        $attrs[] =  trim($_POST['new_'.$attr]); 
        $this->$attr = $attrs;
      }
    }

    
    /*********************
       SMARTY assignments 
     *********************/

    $smarty = get_smarty();
    $smarty->assign("is_default",$this->is_default);
    foreach($this->attributes as $attr){
      if(is_string($this->$attr)){
        $smarty->assign($attr,htmlentities($this->$attr));
      }else{
        $smarty->assign($attr,$this->$attr);
      }
      $smarty->assign($attr."ACL",$this->getacl($attr));
    }

    /* Work on trust modes */
    $smarty->assign("trusthide", " disabled ");
    if ($this->trustModel == "fullaccess"){
      $trustmode= 1;
    } elseif ($this->trustModel == "byhost"){
      $trustmode= 2;
      $smarty->assign("trusthide", "");
    } else {
      $trustmode= 0;
    }
    $smarty->assign("trustmode", $trustmode);
    $smarty->assign("trustmodes", array( 
          0 => _("disabled"), 
          1 => _("full access"),
          2 => _("allow access to these hosts")));

    if((count($this->accessTo))==0){
      $smarty->assign("emptyArrAccess",true);
    }else{
      $smarty->assign("emptyArrAccess",false);
    }
    $smarty->assign("workstations", $this->accessTo);
    
    /* Create lists 
     */
    $divlist_sudoUser = new divSelectBox("divlist_sudoUser");
    $divlist_sudoUser->SetHeight("90");
    $divlist_sudoHost = new divSelectBox("divlist_sudoHost");
    $divlist_sudoHost->Setheight("90");
    $divlist_sudoRunAs = new divSelectBox("divlist_sudoRunAs");
    $divlist_sudoRunAs->Setheight("90");
    $divlist_sudoCommand = new divSelectBox("divlist_sudoCommand");
    $divlist_sudoCommand->Setheight("90");

    /* Fill divlists
     */
    $neg_img= "<img src='plugins/sudo/images/negate.png' alt='!' class='center'>"; 
    $option = "<input type='image' src='plugins/sudo/images/negate.png'     name='neg_%ATTR%_%KEY%' class='center'>"; 
    $option.= "<input type='image' src='images/lists/trash.png'  name='del_%ATTR%_%KEY%' class='center'>"; 
    foreach(array("sudoCommand","sudoHost","sudoRunAs") as $attr){
      if($this->acl_is_readable($attr)){
        foreach($this->$attr as $key => $entry){
          $neg = "";
          if(preg_match("/^!/",$entry)){
            $neg = $neg_img;
          }
          $entry = preg_replace("/^!/","",$entry);
          $list_name = "divlist_".$attr;
          $$list_name->AddEntry(
              array(
                array("string" => $neg,"attach" => "style='width:18px;'"),
                array("string" => $entry),
                array("string" => preg_replace(array("/%KEY%/","/%ATTR%/"),array($key,$attr),$option),
                  "attach" => "style='width:40px; border-right: 0px;'")));
        }
      }
    }

    foreach(array("sudoUser") as $attr){
      $img1 = "<img src='plugins/users/images/select_user.png'   alt='"._("User")."' class='center'>";
      $img2 = "<img src='plugins/groups/images/groups.png' alt='"._("Group")."' class='center'>";
      if($this->acl_is_readable($attr)){
        foreach($this->$attr as $key => $entry){
          $neg = "";
          if(preg_match("/^!/",$entry)){
            $neg = $neg_img;
          }
          $entry = preg_replace("/^!/","",$entry);

          $img = $img1;
          if(preg_match("/^%/",$entry)){
            $img = $img2;
          }
          $entry = preg_replace("/^%/","",$entry);
  
          $list_name = "divlist_".$attr;
          $$list_name->AddEntry(
              array(
                array("string" => $neg,"attach" => "style='width:18px;'"),
                array("string" => $img,"attach" => "style='width:18px;'"),
                array("string" => $entry),
                array("string" => preg_replace(array("/%KEY%/","/%ATTR%/"),array($key,$attr),$option),
                  "attach" => "style='width:40px; border-right: 0px;'")));
        }
      }
    }




    /* Tell smarty about our divlists 
     */
    $smarty->assign("divlist_sudoUser",   $divlist_sudoUser->DrawList());
    $smarty->assign("divlist_sudoHost",   $divlist_sudoHost->DrawList());
    $smarty->assign("divlist_sudoRunAs",  $divlist_sudoRunAs->DrawList());
    $smarty->assign("divlist_sudoCommand",$divlist_sudoCommand->DrawList());
    return($smarty->fetch(get_template_path('generic.tpl', TRUE)));
  }


  /*! \brief  Remove this sudo role from the ldap server 
   */
  function remove_from_parent()
  {
    plugin::remove_from_parent();

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $ldap->rmdir($this->dn);

    /* Send signal to the world that we've done */
    $this->handle_post_events("remove");
  }


  /*! \brief  Save all relevant HTML posts. 
   */
  function save_object()
  {
    plugin::save_object();
    
    if($this->is_default){
      $this->cn = "defaults";
    }  

    /* Trust mode - special handling */
    if($this->acl_is_writeable("trustModel")){
      if (isset($_POST['trustmode'])){
        $saved= $this->trustModel;
        if ($_POST['trustmode'] == "1"){
          $this->trustModel= "fullaccess";
        } elseif ($_POST['trustmode'] == "2"){
          $this->trustModel= "byhost";
        } else {
          $this->trustModel= "";
        }
        if ($this->trustModel != $saved){
          $this->is_modified= TRUE;
        }
      }
    }
  }


  /*! \brief  Save changes into the ldap database.
   */
  function save()
  {
    plugin::save();
   /* Trust accounts */
    $objectclasses= array();
    foreach ($this->attrs['objectClass'] as $key => $class){
      if (preg_match('/trustAccount/i', $class)){
        continue;
      }
      $objectclasses[]= $this->attrs['objectClass'][$key];
    }

    $this->attrs['objectClass']= $objectclasses;
    if ($this->trustModel != ""){
      $this->attrs['objectClass'][]= "trustAccount";
      $this->attrs['trustModel']= $this->trustModel;
      $this->attrs['accessTo']= array();
      if ($this->trustModel == "byhost"){
        foreach ($this->accessTo as $host){
          $this->attrs['accessTo'][]= $host;
        }
      }
    } else {
      if ($this->was_trust_account){
        $this->attrs['accessTo']= array();
        $this->attrs['trustModel']= array();
      }
    }


    /* Ensure a correct array index 
     */ 
    $this->attrs['sudoHost']    = array_values($this->attrs['sudoHost']);
    $this->attrs['sudoRunAs']   = array_values($this->attrs['sudoRunAs']);
    $this->attrs['sudoUser']    = array_values($this->attrs['sudoUser']);
    $this->attrs['sudoCommand'] = array_values($this->attrs['sudoCommand']);

    $this->cleanup();

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    if($this->is_new){
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);

      /* Send signal to the world that we've done */
      $this->handle_post_events("create");
    }else{
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);;

      /* Send signal to the world that we've done */
      $this->handle_post_events("modify");
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }
  }


  /*! \brief  Check the given input.
      @return Array   All error messages in an array();
   */
  function check()
  {
    $message = plugin::check();

    /* Is a name given? */
    if($this->cn == ""){
      $message[] = msgPool::required(_("Name"));
    }

    /* Check if name is reserved */
    if(!$this->is_default && preg_match("/^defaults$/i",$this->cn)){
      $message[] = msgPool::reserved(_("Name"));
    }

    /* Check name */
    if(!preg_match("/^[0-9a-z\@]*$/i",$this->cn)){
      $message[] = msgPool::invalid(_("Name"),$this->cn,"/[0-9a-z\@]/i");
    }

    /* Check if this entry will cause duplicated ldap entries */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->get_sudoers_ou($this->config));
    $ldap->search("(&(objectClass=sudoRole)(cn=".$this->cn."))");
    while($attrs = $ldap->fetch()){
      if($attrs['dn'] != $this->dn){
        $message[] = msgPool::duplicated(_("Name"));
      }
    }

    /* Check if we are allowed to create or move this object
     */
    if($this->orig_dn == "new" && !$this->acl_is_createable($this->get_sudoers_ou($this->config))){
      $message[] = msgPool::permCreate();
    }

    return ($message);
  }


  /*! \brief Force this entry to be handled and saved as 'default'
      @param  BOOL  TRUE -force defaults   FALSE -normal
   */
  public function set_default($state)
  {
    $this->is_default = TRUE;
    $this->cn = "defaults";
  }


  /*! \brief  Add ACL object
      @return Returns the ACL object.
   */
  static function plInfo()
  {
    return (array(  
          "plShortName" => _("Sudo"),
          "plDescription" => _("Sudo role"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("administration"),
          "plCategory"    => array("sudo" => array("objectClass" => "sudoRole", "description" => _("Sudo role"))),

          "plProvidedAcls"    => array(
            "cn"                => _("Name"),
            "description"       => _("Description"),
            "sudoUser"          => _("Users"),
            "sudoHost"          => _("Host"),
            "sudoCommand"       => _("Command"),
            "sudoRunAs"         => _("Run as user"),
            "trustModel"        => _("Access control list"))
        ));
  }


  /*! \brief  This function will be called if an object gets copied.
              This function adapts attributes from the source object.
      @param  Array The source object.
   */
  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);
    foreach(array("sudoUser","sudoCommand","sudoHost","sudoRunAs") as $attr){
      $this->$attr = array();
      if(isset($source[$attr])){
        $tmp = array();
        for($i = 0 ; $i < $source[$attr]['count']; $i++){
          $tmp[] = $source[$attr][$i];
        }
        $this->$attr = $tmp;
      }
    }

    /* Is this account a trustAccount? */
    if (isset($source['trustModel'])){
      $this->trustModel= $source['trustModel'][0];
      $this->was_trust_account= TRUE;
    } else {
      $this->was_trust_account= FALSE;
      $this->trustModel= "";
    }

    $this->accessTo = array();
    if (isset($source['accessTo'])){
      for ($i= 0; $i<$source['accessTo']['count']; $i++){
        $tmp= $source['accessTo'][$i];
        $this->accessTo[$tmp]= $tmp;
      }
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


  public function get_cn()
  {
    return($this->cn);
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
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
