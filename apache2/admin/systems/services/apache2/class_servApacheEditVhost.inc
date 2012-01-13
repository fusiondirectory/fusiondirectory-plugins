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

class servapacheVhostEdit extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  
  var $attributes	= array("apacheServerName","apacheDocumentRoot","apacheServerAlias",
	  	"apacheServerAdmin","apacheScriptAlias","apacheSuexecUid","apacheSuexecGid");
      
  var $objectclasses  = array("whatever");

  var $apacheServerName		= "";
  var $apacheDocumentRoot	= "";
  var $apacheServerAdmin	= "";
  var $apacheSuexecUid		= "";
  var $apacheSuexecGid		= "";
    
  var $apacheServerAlias                = array();
  var $apacheScriptAlias                = array();

  var $OldApacheServerName              = ""; // To detect changes made with this edit
  var $InitialApacheServerName          = "";

  var $Records                  = array();
  
  var $dialog                   = false;

  var $isNew                    = true;
  var $cn;
  var $VhostObject               = array();


  function servapacheVhostEdit ($config, $dn= NULL,$attrs = array())
  {
    plugin::plugin ($config, $dn);
    if(!count($attrs)){
      $this->OldApacheServerName        = "";
      $this->isNew              = true;
      $this->InitialApacheServerName    = "";//$attrs['InitialApacheServerName'];
    }else{
      $this->VhostObject         = $attrs;
      $this->OldApacheServerName        = $attrs['apacheServerName'];
      $this->InitialApacheServerName    = $attrs['InitialApacheServerName'];
      $this->isNew                  = false;
      foreach($this->attributes as $value){
        if(isset($attrs[$value])){
          $this->$value = $attrs[$value];
        }
      }

      if(!isset($attrs['apacheServerAlias'])) $this->apacheServerAlias  = array();
      if(!isset($attrs['apacheScriptAlias'])) $this->apacheScriptAlias  = array();
    }
  }

  /* Transports the given Arraykey one position up*/
  function ArrayUp($atr,$attrs)
  {
    $ret = $attrs;
    $pos = $atr ;
    $cn = count($attrs);
    if(!(($pos == -1)||($pos == 1)||($pos >$cn))){
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
    $pos = $atr ;
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

  /* Combine new array */
  function combineArrays($ar0,$ar1,$ar2)
  {
    $ret = array();
    if(is_array($ar0))
    foreach($ar0 as $ar => $a){
        $ret[]=$a;
    }
    if(is_array($ar1))
    foreach($ar1 as $ar => $a){
        $ret[]=$a;
    }
    if(is_array($ar2))
    foreach($ar2 as $ar => $a){
        $ret[]=$a;
    }
    return($ret);
  }
  
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


  function execute()
  {
    /* Call parent execute */
    plugin::execute();


    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

// Outdated?

#   /* Open Vhost Entry Edit Dialog
#    */
#   if(!count($this->VhostObject)){
#     $smarty->assign("AllowVhostEdit" , false);
#   }else{
#     $smarty->assign("AllowVhostEdit" , true);
#     if(isset($_POST['EditVhostEntries'])){
#       $this->dialog= new apacheVhostEditEntries($this->config,$this->dn,$this->VhostObject);
#     }
#   }
#
#   /* Save Vhost Entry Edit Dialog
#    */
#   if(isset($_POST['SaveVhostEntryChanges'])){
#     $this->dialog->save_object();
#     if(count($this->dialog->check())){
#        msg_dialog::displayChecks($this->dialog->check());
#     }else{
#       $this->dialog->save();
#       $this->dialog = false;
#     }
#   }
#
#   /* Cancel Vhost Entrie Edit Dialog
#   */
#   if(isset($_POST['CancelVhostEntryChanges'])){
#     $this->dialog = false;
#   }
#
#   /* Display any type of open dialogs 
#    */
#   if($this->dialog){
#     $this->dialog->save_object();
#     return($this->dialog->execute());
#   }
#

    /* Check posts 
     */
    $once =true;
    foreach($_POST as $name => $value){

      /* Move Server Aliases one position up 
       */
      if((preg_match("/^SAup_/",$name)) && ($once)){
        $once = false;
        $id = preg_replace("/^SAup_/","",$name);
        $id = preg_replace("/_.*$/","",$id);
        $id = base64_decode($id);
        $this->apacheServerAlias = $this->ArrayUp(($id+1),$this->apacheServerAlias);
      }

      /* Move Server Aliases one position down 
       */
      if((preg_match("/^SAdown_/",$name)) && ($once)){
        $once = false;
        
        $id = preg_replace("/^SAdown_/","",$name);
        $id = preg_replace("/_.*$/","",$id);
        $id = base64_decode($id);
  
        $this->apacheServerAlias = $this->ArrayDown(($id+1),$this->apacheServerAlias);
      }

      /* Delete a "Server Alias"  
       */
      if((preg_match("/^SAdel_/",$name)) && ($once)){
        $once = false;
        $id = preg_replace("/^SAdel_/","",$name);
        $id = preg_replace("/_.*$/","",$id);
        $id = base64_decode($id);
        unset($this->apacheServerAlias[$id]);
        $tmp  =array();
        foreach($this->apacheServerAlias as $entry){
          $tmp[] = $entry;
        }
        $this->apacheServerAlias = $tmp; 
      }

      /* Move "Script Alias" one position up 
       */
      if((preg_match("/^SCup_/",$name)) && ($once)){
        $once = false;
        $id = preg_replace("/^SCup_/","",$name);
        $id = preg_replace("/_.*$/","",$id);
        $id = base64_decode($id);
        $this->apacheScriptAlias = $this->ArrayUp(($id+1),$this->apacheScriptAlias);
      }

      /* Move "Script Alias" one position down
       */
      if((preg_match("/^SCdown_/",$name)) && ($once)){
        $once = false;
        $id = preg_replace("/^SCdown_/","",$name);
        $id = preg_replace("/_.*$/","",$id);
        $id = base64_decode($id);
        $this->apacheScriptAlias = $this->ArrayDown(($id+1),$this->apacheScriptAlias);
      }

      /* Remove "Script Alias"
       */
      if((preg_match("/^SCdel_/",$name)) && ($once)){
        $once = false;
        $id = preg_replace("/^SCdel_/","",$name);
        $id = preg_replace("/_.*$/","",$id);
        $id = base64_decode($id);
        unset($this->apacheScriptAlias[$id]);
        $tmp  =array();
        foreach($this->apacheScriptAlias as $entry){
          $tmp[] = $entry;
        }
        $this->apacheScriptAlias = $tmp; 
      }
    }

    /* Add a new "Server Alias" 
     */
    if((isset($_POST['AddSARecord'])) && (!empty($_POST['StrSAAlias'])) && (!empty($_POST['StrSADir']))){
      $this->apacheServerAlias[] = trim($_POST['StrSAAlias']." ".$_POST['StrSADir']);      
    }
    
    /* Add a new "Script Alias" 
     */
  	if((isset($_POST['AddSCRecord'])) && (!empty($_POST['StrSCAlias'])) && (!empty($_POST['StrSCDir']))){
      $this->apacheScriptAlias[] = trim($_POST['StrSCAlias']." ".$_POST['StrSCDir']);      
    }

    /* Fill in values */
    foreach($this->attributes as $name){
      $smarty->assign($name,$this->$name);
    }

    /* Set apacheServerNames without server suffix */
    $smarty->assign("apacheServerName",$this->apacheServerName);
    $smarty->assign("NotNew", false);


    /* Create Server Aliases divlist 
     */
    $div = new divSelectBox("apacheServerAlias");
    $div->setHeight(120);
    $recs = $this->apacheServerAlias;

    $oneup    = "<input name='SAup_%s'    type='image' src='images/lists/sort-up.png'    title='"._("Up")."'      class='center'>&nbsp;"; 
    $onedown  = "<input name='SAdown_%s'  type='image' src='images/lists/sort-down.png'  title='"._("Down")."'    class='center'>&nbsp;"; 
    $onedel   = "<input name='SAdel_%s'   type='image' src='images/lists/trash.png'      title='"._("Delete")."'  class='center'>";
    foreach($recs as $key => $rec){
      $div ->AddEntry(array(
            array("string"=>$rec),
            array("string"=>str_replace("%s",base64_encode($key),$oneup.$onedown.$onedel),
                  "attach"=>"style='width:70px;border-right:0px;'")
            ));
    }
    $smarty->assign("apacheServerAlias",  $div->DrawList());


    /* Create Server Aliases divlist 
     */
    $div = new DivSelectBox("apacheScriptAlias");
    $div->setHeight(120);
    $recs = $this->apacheScriptAlias;

    $oneup    = "<input name='SCup_%s'    type='image' src='images/lists/sort-up.png'    title='"._("Up")."'      class='center'>&nbsp;"; 
    $onedown  = "<input name='SCdown_%s'  type='image' src='images/lists/sort-down.png'  title='"._("Down")."'    class='center'>&nbsp;"; 
    $onedel   = "<input name='SCdel_%s'   type='image' src='images/lists/trash.png'      title='"._("Delete")."'  class='center'>"; 
    
    foreach($recs as $key => $rec){
      $div ->AddEntry(array(
            array("string"=>$rec),
            array("string"=>str_replace("%s",base64_encode($key),$oneup.$onedown.$onedel),
                  "attach"=>"style='width:70px;border-right:0px;'")
            ));
    }
    $smarty->assign("apacheScriptAlias",  $div->DrawList());    
    
    /* Display template */
    $display.= $smarty->fetch(get_template_path('servApacheVhostEdit.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }


  function remove_from_parent()
  {
  }


  /* Save data to object */
  function save_object()
  {
    plugin::save_object();
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();
        
    /* Check if apacheServerName is already in use */
    $usedVhosts = $this->getUsedServerNames();
    if(($this->isNew == true)||($this->apacheServerName  != $this->InitialApacheServerName)){
      if((isset($usedVhosts[$this->apacheServerName]))&&($this->apacheServerName  != $this->InitialApacheServerName)){
        $message[] = msgPool::duplicated(_("Name"));
      }
    }

    /* Check given virtual server name 
     */
    if(!tests::is_domain($this->apacheServerName) || empty($this->apacheServerName)){
      $message[] = msgPool::invalid(_("Virtual host name"),$this->apacheServerName,"",
          _("Only lowercase strings are allowed as Virtual Host Name.")."&nbsp;"._("e.g. example.com"));
    }

    /* check if there is a valid document root given 
     */
    if(!tests::is_path($this->apacheDocumentRoot) || empty($this->apacheDocumentRoot)){
      $message[] = msgPool::invalid(_("Document root"),$this->apacheDocumentRoot);
    }
   
    /* Check given mail address 
     */ 
  	if(!tests::is_email($this->apacheServerAdmin) || empty($this->apacheServerAdmin)){
      $message[] = msgPool::invalid(_("Admin mail address"),$this->apacheServerAdmin);
    }

    /* Check server aliases 
     */    
    foreach($this->apacheServerAlias as $key => $line){
      $apacheServerAlias_ar=explode(" ",$line);
      $url=$apacheServerAlias_ar[1];
      if(!tests::is_path($url)){
        $message[] = msgPool::invalid(_("Server alias"));
      }
    }
   
    /* Check script aliases 
     */    
    foreach($this->apacheScriptAlias as $key => $line){
      $apacheScriptAlias_ar=explode(" ",$line);
      $url=$apacheScriptAlias_ar[1];
      if(!tests::is_path($url)){
        $message[] = msgPool::invalid(_("Script alias"));
      }
    }

    return ($message);
  }


  /* This funtion returns all used apacheServerNames */
  function getUsedServerNames()
  {
    $ret = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=apacheConfig)(apacheServerName=*))",array("apacheServerName"));
    while($attr = $ldap->fetch()){
        $ret[$attr['apacheServerName'][0]]="";
    }
    return($ret);
  }


  /* Save to LDAP */
  function save()
  {
    $ret =array();
    foreach($this->attributes as $name){
      $ret[$name] = $this->$name;
    }

    /* Create SA records 
     */
    foreach($this->apacheServerAlias as $key => $rec){
      $rec['value']= $key." ".$rec['value'];
      $this->Records [] = $rec;
    }

    /* Create SC records 
     */
    foreach($this->apacheScriptAlias as $key => $rec){
      $rec['value']= $key." ".$rec['value'];
      $this->Records [] = $rec;
    }

    $ret['RECORDS'] = $this->Records; 
    $ret['InitialApacheServerName']   =  $this->InitialApacheServerName;
    return($ret);
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
