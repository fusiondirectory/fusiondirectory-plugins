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

class servapache extends goService
{
  /* attribute list for save action */
  var $ignore_account   = FALSE;
  var $attributes       = array(); 
  var $objectclasses    = array("whatever");

  var $RecordTypes      = array();
  var $Vhosts           = array();
  var $dialog           = NULL;
  var $orig_dn          = "";

  var $conflicts        = array("servapache");

  var $initially_was_account;
  var $krb_service_prefix = "HTTP/";

  function servapache ($config, $dn= NULL, $parent= NULL)
  {
    plugin::plugin ($config, $dn, $parent);

    $this->orig_dn = $dn;

    /* Get all vhost Informations
     */
    $this->Vhosts = apacheUtils::getVhostEntries($config,$dn);

    /* If there is at least one entry in this -> types, we have apache vhosts enabled 
     */
    if(count($this->Vhosts) == 0){
      $this->is_account = false;
    }else{
      $this->is_account = true;
    }
    $this->initially_was_account = $this->is_account;

    /* Set service name */
    $this->DisplayName = _("Apache service");
  }


  function execute()
  {
    /* Call parent execute 
     */
    plugin::execute();

    /* Fill templating stuff 
     */
    $smarty= get_smarty();
    $display= "";

    /* Edited or Added vhost 
     */
    if((isset($_POST['SaveVhostChanges'])) && is_object($this->dialog)){
      $this->dialog->save_object();

      /* Check for errors  
       */
      if(count($this->dialog->check())){
        msg_dialog::displayChecks($this->dialog->check());
      }else{
        /* add new/edited vhost 
         */
        $ret = $this->dialog->save();
        if(!$this->dialog->isNew){
          unset($this->Vhosts[$this->dialog->OldApacheServerName]);
        }
        $this->Vhosts[$ret['apacheServerName']] = $ret;
        $this->dialog = NULL;
      }
    }

    /* Cancel vhost edit / new 
     */
    if(isset($_POST['CancelVhostChanges'])){
      $this->dialog = NULL;
    }

    /* Add empty new vhost 
     */
    if(isset($_POST['AddVhost']) && $this->acl_is_writeable("VirtualHosts")){
      $this->dialog = new servapacheVhostEdit($this->config,$this->dn);
    }

    /* Check for edit vhost request 
     */
    $once = false;
    foreach( $_POST as $name => $value){

      /* check all post for edit request 
       */
      if(preg_match("/^editVhost_/",$name)&& !$once && $this->acl_is_writeable("VirtualHosts")){
        $once =true;
        $tmp = preg_replace("/^editVhost_/","",$name);
        $tmp = base64_decode(preg_replace("/_.*$/","",$tmp));
        $this->dialog= new servapacheVhostEdit($this->config,$this->dn,$this->Vhosts[$tmp]);
      }

      /* check posts for delete vhost 
       */
      if(preg_match("/^delVhost_/",$name)&&!$once && $this->acl_is_writeable("VirtualHosts")){

        $once =true;
        $tmp = preg_replace("/^delVhost_/","",$name);
        $tmp = base64_decode(preg_replace("/_.*$/","",$tmp));
     
        /* Initiate deletion
         */ 
        $this->RemoveVhost($tmp);
      }
    }

    /* Show dialog 
     */
    if($this->dialog!= NULL){
      $this->dialog->save_object();
      $this->dialog->parent = $this;
      return($this->dialog->execute());
    }

    /* Create Listbox with existing Vhosts 
     */
    $VhostList = new divSelectBox("apacheConfigs");
    $VhostList -> SetHeight(254);

    /* Add entries to divlist
     */
    $editImg = "<input type='image' src='images/lists/edit.png' name='editVhost_%s'>
      <input type='image' src='images/lists/trash.png' name='delVhost_%s'>";
    foreach($this->Vhosts as $vhost => $values ){
      $VhostList->AddEntry(array(
            array("string" => $vhost),
            array("string" => str_replace("%s",base64_encode($vhost),$editImg))
            ));
    }    

    /* Assign ACL to smarty 
     */    
    $info = $this->plInfo();
    foreach($info['plProvidedAcls'] as $name => $desc){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

  
    /* Display template 
     */
    $smarty->assign("VhostList",$VhostList->DrawList());
    $display.= $smarty->fetch(get_template_path('servApacheVhost.tpl', TRUE , dirname(__FILE__)));
    return($display);
  }


  /* Delete specified vhost
   */  
  function RemoveVhost($id)
  {
      unset($this->Vhosts[$id]);
      return(true);
  }


  /* This function returns all used Vhostnames 
   */
  function getUsedServerNames()
  {
    $ret = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=apacheConfig)(apacheServerName=*))",array("apacheServerName"));
    while($attr = $ldap->fetch()){
      $ret[$attr['apacheServerName'][0]][] = $attr['dn'];
    }
    return($ret);
  }


  /* Remove apache service
   */
  function remove_from_parent()
  {
    foreach($this->Vhosts as $key => $vhost){
      $this->RemoveVhost($key);
    }
    $this->save();
  }


  /* Save to LDAP */
  function save()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);  
  
    /* Get differences 
     */

    $old_dn =  $this->orig_dn;
    if($old_dn == "new"){
      $old_dn = $this->dn;
    }

    $tmp = apacheUtils::getVhostEntriesDiff($this->config,$this->Vhosts,$old_dn);

    /* Updated vhost entries if reverser or forward name has changed  
     * Must be done before moving entries, else the given dn is invalid
     */
    if(isset($tmp['vhostUpdates'])){
      foreach($tmp['vhostUpdates'] as $dn => $attrs){
        $ldap->cd($dn);
        $ldap->modify($attrs);

        if (!$ldap->success()){
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
        }
      }
    }

    /* Delete apache vhost
     */
    foreach($tmp['del'] as $dn => $del){
      $ldap->cd($dn);
      $ldap->rmdir_recursive($dn);
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
      }
    }

    /* move follwoing entries
     */
    foreach($tmp['move'] as $src => $dst){
      $this->recursive_move($src,$dst);
    }

    /* Add || Update new apache entries
     */
    foreach($tmp['add'] as $dn => $attrs){
      $ldap->cd($dn);
      $ldap->cat($dn, array('dn'));
      if(count($ldap->fetch())){
        $ldap->cd($dn);
        $ldap->modify ($attrs);
      }else{
        $ldap->cd($dn);
        $ldap->add($attrs);
      }
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_ADD, get_class()));
      }
    }
  }


  /*! \brief Returns the required information about this plugin for the 
             service list 
      @return Array  With information for the service plugin.
    */
  function getListEntry()
  {
    $fields = goService::getListEntry();
    $fields['Message']    = _("Apache service");
    #$fields['AllowEdit']  = TRUE;
    #$fields['AllowRemove']= TRUE;
    return($fields);
  }


  /*  \brief Return plugin information used by the ACL handling.
      @return Array ACL infos.
   */  
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Apache service"),
          "plDescription" => _("Apache virtual host service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 88,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
              "VirtualHost"           => _("Virtual hosts"))
            ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
