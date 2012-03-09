<?php

class novaUser extends plugin
{
  /* Definitions */
  var $plHeadline= "novaUser";
  var $plDescription= "Edit users Nova settings";

  /* plugin specific values */
  var $isNovaAdmin = "false";
  var $accessKey = "";
  var $secretKey = "";

  /* attribute list for save action */
  var $attributes = array("isNovaAdmin","accessKey","secretKey");

  var $is_chk_box = array("isNovaAdmin");

  var $objectclasses= array("novaUser");

  var $uid = "";
  var $view_logged = FALSE;

  function getRandStr($len) {
    $chars = "abcbefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^[]{}&/-=+_";
    $res = "";
    for ($i=0; $i<$len; $i++) {
      $res .= $chars[rand(0,strlen($chars)-1)];
    }
    return $res;
  }

  /* constructor, if 'dn' is set, the node loads the given
     'dn' from LDAP */
  function novaUser (&$config, $dn= NULL)
  {
    /* Configuration is fine, allways */
    $this->config= $config;

    plugin::plugin ($config, $dn);

    /* Setting uid to default */
    if(isset($this->attrs['uid'][0])){
      $this->uid = $this->attrs['uid'][0];
    }
    if($this->accessKey === ""){
      $this->accessKey = $this->getRandStr(36);
    }
    if($this->secretKey === ""){
      $this->secretKey = $this->getRandStr(36);
    }

    /* Save initial account state */  
    $this->initially_was_account= $this->is_account;
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Log view */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","users/".get_class($this),$this->dn);
    }

    /* Load templating engine */
    $smarty= get_smarty();
    $display= "";

    /* Do we need to flip is_account state? */
    if(isset($_POST['modify_state'])){
      if($this->is_account && $this->acl_is_removeable()){
        $this->is_account= FALSE;
      }elseif(!$this->is_account && $this->acl_is_createable()){
        $this->is_account= TRUE;
      }
    }    

    /* Do we represent a valid account? */
    if (!$this->is_account && $this->parent === NULL){
      $display= "<img alt=\"\"src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
        msgPool::noValidextension(_("novaUser"))."</b>";

      $display.= back_to_main();
      return ($display);
    }

    /* Show tab dialog headers */
    if ($this->parent !== NULL && !$this->is_account){
        $display= $this->show_enable_header(msgPool::addFeaturesButton(_("novaUser")), msgPool::featuresDisabled(_("novaUser")));
        return($display);
    }

    /* Assign acls */
    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $acl => $desc){
      $smarty->assign($acl."ACL",$this->getacl($acl));
    }

    /* Assign attributes an ACL to smarty */
    foreach($this->attributes as $val) { 
      $smarty->assign("$val", $this->$val);
      if(in_array($val,$this->is_chk_box)){
        if($this->$val == "TRUE"){
          $smarty->assign($val."CHK", " checked ");
        }else{
          $smarty->assign($val."CHK", "");
        }
      }
    }
    $display.= $smarty->fetch (get_template_path('novauser.tpl', TRUE, dirname(__FILE__)));
    return ($display);
  }

  /* Adapt from template, using 'dn' */
  function adapt_from_template($dn, $skip= array())
  {
    /* Include global link_info */
    $ldap= $this->config->get_ldap_link();

    plugin::adapt_from_template($dn, $skip);
  }

  /* Save to LDAP */
  function save()
  {
    $ldap= $this->config->get_ldap_link();

    /* Call parents save to prepare $this->attrs */
    plugin::save();

    /* Adapt attributes if needed */
    //$method= new $this->method($this->config);
    //$id= $method->uattrib;
    //$method->fixAttributesOnStore($this);

    /* Write back to ldap */
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify($this->attrs); 

    /* Log last action */
    if ($this->initially_was_account) {
      new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    } else {
      new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    /* Optionally execute a command after we're done */
    if ($this->initially_was_account == $this->is_account) {
      if ($this->is_modified) {
        $this->handle_post_events("modify",array("uid" => $this->uid));
      }
    } else {
      $this->handle_post_events("add", array("uid" => $this->uid));
    }
  }

  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    return($message);
  }


  /* Use Save_object for every Post handling */
  function save_object()
  {
    if (isset($_POST['novauserTab'])){
      /* Save ldap attributes */
      plugin::save_object();

      /* Specialhandling for checkboxes */
      foreach($this->is_chk_box as $val){
        if($this->acl_is_writeable($val)){
          if(isset($_POST[$val])){
            $this->$val = "TRUE";
          }else{
            $this->$val = "FALSE";
          }
        }
      }
    }
  }


  /* hook for working synchronization */
  function remove_from_parent()
  {
    return;
  }


  static function plInfo()
  {
    return (array(
          "plShortName"     => _("novaUser"),
          "plDescription"   => _("novaUser account settings"),
          "plSelfModify"    => TRUE,
          "plDepends"       => array("user"),
          "plPriority"      => 3,                                 // Position in tabs
          "plSection"     => array("personal" => _("My account")),
          "plCategory"    => array("users"),
          "plOptions"       => array(),
          "plRequirements"=> array(
              'ldapSchema' => array('novaUser' => ''),
              'onFailureDisablePlugin' => array(get_class())
              ),

          "plProvidedAcls"  => array(
            "isNovaAdmin" => _("Is Nova admin"))
      ));
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:

?>
