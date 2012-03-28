<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2010 Antoine Gallavardin
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

class heimdalAccount extends plugin
{

  /* declaration des variables de description */
  var $plHeadline = "heimdal kerberos";
  var $plDescription = "heimdal kerberos plugin";

  /* declaration des variables système, log, initialisation de la classe */

  var $view_logged    = FALSE;
  var $is_account     = FALSE;
  var $initially_was_account = FALSE;
  
  /* declaration et initialisation des variables dedié au plugin */
  
  var $krb5KeyVersionNumber="1";
  var $krb5MaxLife="";
  var $krb5MaxRenew="";
  var $krb5EncryptionType="";
  var $krb5ValidStart="";
  var $krb5ValidEnd="";
  var $krb5PasswordEnd="";
  var $krb5PrincipalName="";
  //var $userPassword="{K5KEY}";
  
  
  var $flag_list = array(
      "0"=>"initial" , 
      "1"=>"forwardable" , 
      "2"=>"proxiable" , 
      "3"=>"renewable" , 
      "4"=>"postdate" , 
      "5"=>"server" , 
      "6"=>"client" , 
      "7"=>"invalid" , 
      "8"=>"require-preauth" , 
      "9"=>"change-pw" , 
      "10"=>"require-hwauth" , 
      "11"=>"ok-as-delegate" , 
      "12"=>"user-to-user" , 
      "13"=>"immutable");
      
  var $krb5KDCFlags     = 123;
  
  //TODO $attributes = merge de attributes_single attributes_multi
  var $attributes= array("krb5KeyVersionNumber","krb5MaxLife","krb5MaxRenew","krb5KDCFlags","krb5EncryptionType","krb5ValidStart","krb5ValidEnd", "krb5PasswordEnd","krb5PrincipalName","krb5key");
  var $objectclasses= array("krb5Principal","krb5KDCEntry");

  function heimdalAccount(&$config, $dn= NULL)
  {
    
    $this->config= $config;
    $this->parent_dn = $dn;
    $this->is_account = FALSE;

    global $class_mapping;

    plugin::plugin($config,$dn); 

    /* get an ldap link */
    $ldap= $this->config->get_ldap_link();
    
    /* if DN is not nulll, the record is already existing */
    if ($dn !== NULL){
      if (isset($this->attrs['objectClass']) &&
          in_array ( array("krb5Principal","krb5KDCEntry"), $this->attrs['objectClass'])){
        $this->is_account= TRUE;
      }
      /* Save initial account state */
      $this->initially_was_account= $this->is_account;
    }

    foreach(array("krb5KeyVersionNumber","krb5MaxLife","krb5MaxRenew","krb5KDCFlags","krb5EncryptionType","krb5ValidStart","krb5ValidEnd", "krb5PasswordEnd","krb5PrincipalName") as $attr){
      if(isset($this->parent->by_object['group']) && isset($this->parent->by_object['group']->$attr)){
        $this->$attr = &$this->parent->by_object['group']->$attr;
      }elseif(isset($this->attrs[$attr])){
        $this->$attr = $this->attrs[$attr][0];
      }
    }
    
    # split krb5PrincipalName
    $tmpkrb5PrincipalName=array();
    $tmpkrb5PrincipalName=preg_split('/@/',$this->krb5PrincipalName);
    print_a($tmpkrb5PrincipalName);
    $this->uidKrb5PrincipalName=$tmpkrb5PrincipalName[0];
    $this->realmKrb5PrincipalName=$tmpkrb5PrincipalName[1];
    
    $date_values = array("krb5ValidStart","krb5ValidEnd","krb5PasswordEnd");

    foreach($date_values as $date_val){
      $clear = $date_val."_clear";
      if(empty($this->$date_val)){
            $this->$clear = TRUE;
      }else{
            $this->$clear = FALSE;
      }
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']); 
    $ldap->search("(objectClass=goKrbServer)",array("goKrbRealm"));

    while($attrs = $ldap->fetch()){
      if (isset($attrs['goKrbRealm'][0])){
      $this->liste_realm[]=$attrs['goKrbRealm'][0];
      }
    }
  }
    
  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    $display= "";
    /* log function */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","users/".get_class($this),$this->dn);
    }

    if(isset($_POST['modify_state'])){
      if($this->is_account && $this->acl_is_removeable()){
        $this->is_account= FALSE;
      }elseif(!$this->is_account && $this->acl_is_createable()){
        $this->is_account= TRUE;
      }
    }

    /* is this account a valid account  */
    if (!$this->is_account && $this->parent == NULL ){
        $display= "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
        msgPool::noValidExtension(_("KERBEROS"))."</b>";
        $display.= back_to_main();
        return ($display);
    }

    /* Show tab dialog headers */
    if ($this->parent !== NULL){
      if ($this->is_account){
        $display= $this->show_disable_header(msgPool::removeFeaturesButton(_("KERBEROS")), msgPool::featuresEnabled(_("KERBEROS")));
      } else { 
        $display= $this->show_enable_header(msgPool::addFeaturesButton(_("KERBEROS")), msgPool::featuresDisabled(_("KERBEROS")));
        return($display);
      }
    }
    
    /* Templates now! */
    $smarty=get_smarty();

    foreach($this->attributes as $attr){
      $smarty->assign($attr ,$this->$attr);
    }
    $years = array();
    $start = date("Y")-1;
    for($i = $start; $i < ($start +20) ; $i++){
      $years[$i] = $i;
    }
    $month= array();
    for($i = 1; $i <= 12  ; $i++){
      $month[str_pad($i,2,"0",STR_PAD_LEFT)] = $i;
    }
    $days= array();
    for($i = 1; $i <= 31  ; $i++){
      $days[str_pad($i,2,"0",STR_PAD_LEFT)] = $i;
    }
    $hours= array();
    for($i = 0; $i <= 23  ; $i++){
      $hours[str_pad($i,2,"0",STR_PAD_LEFT)] = $i;
    }
    $minutes= array();
    for($i = 0; $i <= 59  ; $i++){
      $minutes[str_pad($i,2,"0",STR_PAD_LEFT)] = $i;
    }
  
    $smarty = get_smarty();
    $smarty->assign("years",$years);
    $smarty->assign("month",$month);
    $smarty->assign("days",$days);
    $smarty->assign("hours",$hours);
    $smarty->assign("minutes",$minutes);

    $date_values = array("krb5ValidStart","krb5ValidEnd","krb5PasswordEnd");
    foreach($date_values as $date_val){
      $clear = $date_val."_clear";
      $smarty->assign($date_val."_clear",$this->$clear);
      $smarty->assign($date_val."_y",substr($this->$date_val,0,4));
      $smarty->assign($date_val."_m",substr($this->$date_val,4,2));
      $smarty->assign($date_val."_d",substr($this->$date_val,6,2));
      $smarty->assign($date_val."_h",substr($this->$date_val,8,2));
      $smarty->assign($date_val."_i",substr($this->$date_val,10,2));
    }

    // affect object value to smarty variables
    foreach($this->attributes as $attr){
      $smarty->assign($attr ,$this->$attr);
    }
    // affect object value to smarty variables split kerberosPrincipalName
    $smarty->assign("uidKrb5PrincipalName",$this->uidKrb5PrincipalName);
    $smarty->assign("realmKrb5PrincipalName",$this->realmKrb5PrincipalName);
    
    // affect object value to smarty variables ( check box)
    foreach($this->flag_list as $key => $name){
      $val = pow(2,$key);
      if($this->krb5KDCFlags & $val){
        $smarty->assign("krb5KDCFlags_".$key,TRUE);
      }else{
        $smarty->assign("krb5KDCFlags_".$key,FALSE);
      }
    }
    
    // affect object value to smarty variables (list of available realm)  
    $tmp=array();
    foreach($this->liste_realm as $attr){
        $tmp[$attr] = $attr;        
    }
    
    $smarty->assign("liste_realm",$tmp); 

    //$SkipWrite = (!isset($this->parent) || !$this->parent) && !session::is_set('edit');
    $display= $display.$smarty->fetch (get_template_path('heimdal.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }

  /* Validate user input 
   */
  function check()
  {
  }

  /* Save attribute modifications back to the ldap
   */
  function save()
  {
    /* Write modifications bak to the ldap */
    /* Call the save method of the mother plugin, to generate a new ldap entry */

    if (isset($_POST['heimdalAccountTab'])){

      /* import all differente date */
      $date_values = array("krb5ValidStart","krb5ValidEnd","krb5PasswordEnd");
      foreach($date_values as $date_value){
        $clear = $date_value."_clear";
        if(isset($_POST[$date_value."_clear"])){
          $this->$clear = TRUE;
          unset($this->$date_value);
        }else{
          $this->$clear = FALSE;
          $str = "";
          foreach(array("y","m","d","h","i") as $val){
            if(isset($_POST[$date_value."_".$val])){
              $str .= $_POST[$date_value."_".$val]; 
            }
          }
          $this->$date_value = $str."Z";
        }
      }
      
      /* import all attributes */
      foreach($this->attributes as $attr){
        if(isset($_POST[$attr])){
          $this->$attr = get_post($attr);
        }
      }
      
      /* import SMARTY value for kerberos principal name */
      $this->uidKrb5PrincipalName = get_post('uidKrb5PrincipalName');
      $this->realmKrb5PrincipalName = get_post('realmKrb5PrincipalName');
      $this->krb5PrincipalName = $this->uidKrb5PrincipalName."@".$this->realmKrb5PrincipalName;
     
      /* Heimdal Kerberos flag Management*/
      $int = "";
      foreach($this->flag_list as $key => $name){
        $post = "krb5KDCFlags_".$key;
        if(isset($_POST[$post])){
          $int |= pow(2,$key);
        }
      }
      $this->krb5KDCFlags = $int;

      // userPassword MUST be {K5KEY}
      $this->userPassword="{K5KEY}";
    
      //print_a($this->attrs);
      plugin::save();
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
      if($ldap->success()){
      // "Ok!"
      }else{
        msgPool::ldaperror($ldap->get_error(), $this->dn,LDAP_MOD, get_class());
      }
    }
  }


  /*! \brief  Removes the Heimdal Kerberos extension from ldap 
   */  
  function remove_from_parent()
  {
    /* Cancel if there's nothing to do here */
    if (!$this->initially_was_account){
      return;
    }

    /* Remove attributes */
    plugin::remove_from_parent();

    @DEBUG (DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__,$this->attributes, "Save");
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $ldap->modify ($this->attrs);

    /* Add "view" to logging class */
    new log("remove","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
    }
  }


  /* Adapt from template, using 'dn' */
  function adapt_from_template($dn, $skip= array())
  {
    /* Include global link_info */
    $ldap= $this->config->get_ldap_link();

    plugin::adapt_from_template($dn, $skip);
    // get name of template
    $template= $this->attrs['uid'][0];

    /* Fix primary group settings */
    $ldap->cd($this->config->current['BASE']);
    
    /* Adjust krbValid  checkboxes */
    foreach (array("krb5ValidEnd", "krb5ValidStart", "krb5PasswordEnd") as $val){
      if ($this->$val != 0){
        $oval= $val."_clear";
        $this->$oval= "0";
      }
    }
  }

  static function plInfo()
  {
   return (array(
        "plShortName"   => _("Informations REALM "),
        "plDescription" => _("Plugin des REALM Cemagref"),
        "plSelfModify"  => TRUE,
        "plDepends"     => array("users"),
        "plPriority"    => 5,
        "plSection"     => array("personal" => _("My account")),
        "plCategory"  => array("users"),
        "plOptions" =>array(),

        "plProvidedAcls" => array(
          "krb5KeyVersionNumber" =>  _("krb5KeyVersionNumber"),
          "krb5MaxLife" => _("krb5MaxLife"),
          "krb5MaxRenew" => _("krb5MaxRenew"),
          "krb5KDCFlags" => _("krb5KDCFlags"),
          "krb5EncryptionType" => _("krb5EncryptionType"),
          "krb5ValidStart" => _("krb5ValidStart"),
          "krb5ValidEnd" => _("krb5ValidEnd"),
          "krb5PasswordEnd" => _("krb5PasswordEnd"),
          "krb5Key" =>_("krb5Key"),
          "objectClass" => array("krb5Principal","krb5KDCEntry"),
        ));
  }




}

 
 
 
 
 
  

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
