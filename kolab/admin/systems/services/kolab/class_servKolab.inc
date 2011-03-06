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

class servkolab extends goService {
  var $postfix_mydomain                 = "";
  var $cyrus_admins                     = "";
  var $postfix_mydestination            = array();
  var $postfix_mynetworks               = "127.0.0.1/8";
  var $postfix_enable_virus_scan        = "TRUE";
  var $postfix_relayhost                = "";
  var $postfix_mxrelayenabled           =  true;
  var $postfix_allow_unauthenticated     = "FALSE";
  var $cyrus_quotawarn                  = "80";
  var $kolabFreeBusyFuture              = "1";
  var $k                                = "kolab";
  var $cyrus_imap                       = "TRUE";
  var $cyrus_pop3                       = "FALSE";
  var $cyrus_imaps                      = "TRUE";
  var $cyrus_pop3s                      = "TRUE";
  var $cyrus_sieve                      = "TRUE";
  var $apache_allow_unauthenticated_fb  = "TRUE";
  var $proftpd_ftp                      = "FALSE";
  var $apache_http                      = "TRUE";
  var $kolabHost                        = array();
  var $orig_cn                          = "";
  var $view_logged                      = FALSE;

  var $attributes =  array("postfix_mydomain", "postfix_mydestination", "proftpd_ftp", "k",
      "postfix_mynetworks", "postfix_enable_virus_scan", "postfix_relayhost", "apache_http",
      "postfix_allow_unauthenticated", "cyrus_admins", "cyrus_imap","kolabFreeBusyFuture",
      "cyrus_pop3", "cyrus_imaps", "cyrus_pop3s", "cyrus_sieve", "apache_allow_unauthenticated_fb",
      "cyrus_quotawarn");
  var $objectclasses = array("top", "kolab");

  /* Serverservice vars */
  var $conflicts    = array("goMailServer");
  var $DisplayName  = "Kolab mail service";
  var $StatusFlag   = "";

  function servkolab(&$config, $dn = NULL, $parent= NULL) 
  {
    /* Setting the hostname and tell this Plugin that we are the kolab extension*/
    $this->hostname = preg_replace('/^cn=([^,]+),.*$/', '\1', $dn);
    $this->dn       = "k=kolab,".$config->current['BASE'];

    /* Load variables, if given*/
    plugin::plugin($config, $this->dn);

    /* Copy needed attributes */
    foreach($this->attributes as $val) {
      $name = preg_replace('/_/', '-', $val);
      if (isset($this->attrs["$name"][0])) {
        $this->$val = $this->attrs["$name"][0];
      }
    }

    /* Toggle relayhost */
    $this->postfix_mxrelayenabled = preg_match('/^\[/', $this->postfix_relayhost);
    $this->postfix_relayhost      = preg_replace("/[\[\]]/","",$this->postfix_relayhost);

    /* Is this Server a member of the Kolab extension or not ?*/ 
    if(isset($this->attrs['kolabHost'])) {
      $this->kolabHost= $this->attrs['kolabHost'];
      unset($this->kolabHost['count']);
    }
    $this->is_account            = false;
    $this->initially_was_account = false;
    foreach($this->kolabHost as $host){
      if($this->hostname == $host){
        $this->is_account            = true;
        $this->initially_was_account = true;
      }
    }

    /* Get list of configured domains 
     */ 
    $this->postfix_mydestination = array();
    if(isset($this->attrs['postfix-mydestination'])){
      for($i=0; $i < $this->attrs['postfix-mydestination']['count']; $i++){
        $this->postfix_mydestination[] = $this->attrs['postfix-mydestination'][$i];
      }
    }
  }


  function execute() 
  {
    /* Call parent execute */
    plugin::execute();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    /***************  
      Variable initialisation
     ***************/  
    
    $smarty     = get_smarty();
    $display    = "";
    $ldap       = $this->config->get_ldap_link();


    /* Assemble free/busy string */
    $edit       = sprintf('<input name="kolabFreeBusyFuture" value="%s" type="text" maxlength="3" size="4">',
                    $this->kolabFreeBusyFuture);
    $fbfuture   = sprintf(_("Include data from %s days in the past when creating free/busy lists"), $edit);

    /* Assemble quota string */
    $edit       = sprintf('<input name="cyrus_quotawarn" value="%s" type="text" maxlength="3" size="4">',
                    $this->cyrus_quotawarn);
    $quotastr   = sprintf(_("Warn users when using more than %s%% of their mail quota"), $edit);

    /***************
      Assign informations to smarty 
     ***************/

    /* Set relayhost and if we have MX lookup enabled*/ 
    if($this->postfix_mxrelayenabled) {
      $smarty->assign("RelayMxSupportCheck"," checked ");
    } else {
      $smarty->assign("RelayMxSupportCheck","");
    }

    /* Set acls */
    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    /* Initialize all attributes, that were submitted */
    foreach($this->attributes as $val) 
    {
      /* Tell smarty which variables we are useing */
      $smarty->assign($val, $this->$val);
      if (($this->$val != "FALSE") && !empty($this->$val)){
        $smarty->assign($val."Check", "checked");
      } else {
        $smarty->assign($val."Check", "");
      }
    }

    $smarty->assign("fbfuture", $fbfuture);
    $smarty->assign("quotastr", $quotastr);

    /* Create mail domain divlist  (postfix_mydestination)
     */
    $divlist = new divSelectBox("fuse");
    $divlist->setHeight(100);
    foreach($this->postfix_mydestination as $key => $domain){
      $f1 = array("string" => $domain);
      $f2 = array("string" => "<input type='image' src='images/lists/trash.png' 
                                  name='remove_domain_".$key."'
                                  title='"._("Remove")."' alt='"._("Remove")."'>", 
                  "attach" => "style='border-right:0px; width: 20px;'");
      $divlist->AddEntry(array($f1,$f2));
    }
    $smarty->assign("mdDiv",$divlist->DrawList());

    /* Load Template */
    $display.=$smarty->fetch(get_template_path('servkolab.tpl', TRUE,dirname(__FILE__)));
    return ($display);
  }

  function remove_from_parent() 
  {

     /* Remove status flag, it is not a memeber of
        this->attributes, so ensure that it is deleted too */
    if(!empty($this->StatusFlag)){
      $this->attrs[$this->StatusFlag] = array();
    }


    /* Only walk through following code, if this host 
       was a member of the kolab hosts, else skip this */
    if(!$this->initially_was_account){
      return;
    }

    /* !!! Don't use "cn" in this function 
       hostname -> the initial name of the host
       cn       -> is the new name of the host, in case that it was renamed.
     */

    $ldap     = $this->config->get_ldap_link();
    $this->dn = "k=kolab,".$this->config->current['BASE'];

    /* We can't simply remove the whole entry, it is possible that there are 
       some other hosts assigned to this object. 
       So, first of all check if we are the last host entry within the host 
       : Remove k=kolab entry 
       if we aren't alone, only remove host name from hosts entry and save */
    /* Are we alone? Remove complete entry... */

    /* Check if we are definitly in kolabHosts */
    if(!in_array_ics($this->hostname,$this->kolabHost)) {
      return;
    }

    /* Integration check, not translated because they can't pop up at all, only for debug */ 
    if(count($this->kolabHost) == 0){
      msg_dialog::display(_("Internal error"), _("Kolab account does not exist!"), ERROR_DIALOG);
      return;
    }
    if(!isset($this->hostname) || (empty($this->hostname))){
      msg_dialog::display(_("Error"), msgPool::required(_("hostname")), ERROR_DIALOG);
    }

    /* Our hostname is in kolabHosts and there is a only one entry 
       = we are the last host entry, delete k=kolab entry  */
    if (count($this->kolabHost) == 1){

      /* Remove complete entry */
      $ldap->rmdir ($this->dn);
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
      }
    } else {

      /* Only modify kolabHost */
      $hosts= array();
      foreach ($this->kolabHost as $host){
        if($host != $this->hostname){
          $hosts[]= $host;
        }
      }
      $attrs= array('kolabHost' => $hosts);
      $ldap->cd($this->dn);
      $this->cleanup();
      $ldap->modify ($attrs); 
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
      }
    }

    new log("remove","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove");
  }


  function save_object()    
  {
    if (isset($_POST['kolabtab'])){
      plugin::save_object();

      /* Domain name added/removed 
       */
      foreach($_POST as $name => $value){
        if(preg_match("/^remove_domain_/",$name)){
          $id = preg_replace("/^remove_domain_([0-9]*)_.*$/","\\1",$name);
          if(isset($this->postfix_mydestination[$id])){
            unset($this->postfix_mydestination[$id]);
          }
          break;
        }
      }
      if(isset($_POST['add_domain_name']) && isset($_POST['new_domain_name'])){
        $new_domain = trim(get_post('new_domain_name'));
        if(!empty($new_domain) && tests::is_domain($new_domain)){
          $this->postfix_mydestination[] = $new_domain;
        }
      }

      /* Map attrinutes 
       */
      foreach($this->attributes as $attr){
        if(($this->acl_is_writeable(preg_replace("/_/","",$attr))) && (isset($_POST[$attr]))){
          $this->$attr = $_POST[$attr];
        }
      }

      /* Save checkboxes */
      foreach (array( "postfix_enable_virus_scan", "postfix_allow_unauthenticated",
            "cyrus_imap", "cyrus_pop3", "cyrus_imaps",
            "cyrus_pop3s", "cyrus_sieve", "apache_allow_unauthenticated_fb",
            "proftpd_ftp", "apache_http") as $cb){

        if($this->acl_is_writeable(preg_replace("/_/","",$cb))){
          if (isset($_POST[$cb])){
            $this->$cb= "TRUE";
          } else {
            $this->$cb= "FALSE";
          }
        }
      }

      /* Toggell relay check */
      if($this->acl_is_writeable("postfixmxrelayenabled")){
        $this->postfix_mxrelayenabled= isset($_POST['RelayMxSupport']);
      }
    }

  }


  function check() 
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    if(mailMethod::get_current_method($this->config) != "mailMethodKolab22" && count($this->postfix_mydestination) != 1){
#      $message[] = _("Only kolab version >= 2.2 is able to handle multiple mail domains. Please provide only one mail domain.");
    }

    if(($this->kolabFreeBusyFuture==""))    {
      $message[] = msgPool::required(_("Future days"));
    }elseif(!tests::is_uid($this->kolabFreeBusyFuture) || $this->kolabFreeBusyFuture < 0){
      $message[] = msgPool::invalid(_("Future days"),$this->kolabFreeBusyFuture,"/[0-9]/");
    }

    if(!is_int((int)($this->cyrus_quotawarn))) {
      $message[] = msgPool::required(_("Quota"));
    }elseif($this->cyrus_quotawarn>100){
      $message[] = _("Please choose a value between 1 and 100 for Quota settings.");
    }elseif($this->cyrus_quotawarn < 1){
      $message[] = msgPool::toosmall(_("Quota"),1);
    }

    if(empty($this->postfix_mynetworks)) { 
      $message[] = msgPool::required(_("Privileged networks"));
    }

    if(!count($this->postfix_mydestination)){
      $message[] = msgPool::required(_("Mail domains")." (postfix-mydestination)" );
    }

    return ($message);
  }


  /* Save to LDAP */
  function save() 
  {
    /* Set ldap connection */
    $ldap       = $this->config->get_ldap_link();

    /* Open current dn*/
    $this->dn   = "k=kolab,".$this->config->current['BASE'];

    /* Adapt relayhost */
    $this->postfix_relayhost= preg_replace('/[\[\]]/', '', $this->postfix_relayhost);
    if ($this->postfix_mxrelayenabled && $this->postfix_relayhost != ""){
      $this->postfix_relayhost= "[".$this->postfix_relayhost."]";
    }

    /* Check if this server was renamed, in this case we have to remove old cn first*/
    if($this->hostname != $this->cn){
      $tmp = array();
      if(in_array_ics($this->hostname,$this->kolabHost)){
        foreach($this->kolabHost as $host){
          if($host != $this->hostname){
            $tmp[] = $host;
          }
        }
        $this->kolabHost = $tmp;
      }
    }

    /* Add ourselves to the list of kolabHost's if needed */
    if (!in_array_ics($this->cn,$this->kolabHost)){
      $this->kolabHost[]= $this->cn;
    }

    /* Call parents save to prepare $this->attrs */
    plugin::save();

    /* Save or modify? */
    $ldap->cat($this->dn, array('dn'));
    if (!$ldap->fetch()){
      $mode= "add"; 
    } else {
      $mode= "modify";
    }

    /* Do attribute conversion */
    foreach ($this->attrs as $key => $value){
      if (preg_match('/_/', $key)){
        $old_key= $key;
        $key= preg_replace('/_/', '-', $key);
        $this->attrs[$key]= $value;
        unset($this->attrs[$old_key]);
      }
    }
    /* Add kolab hosts */
    $this->attrs['kolabHost']= $this->kolabHost;

    /* Perform LDAP action */
    $ldap->cd($this->dn);
    $this->cleanup();;
    $ldap->$mode($this->attrs);

    if($mode == "add"){
      new log("create","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("modify","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    /* Optionally execute a command after we're done */
    if ($this->initially_was_account == $this->is_account) {
      if ($this->is_modified) {
        $this->handle_post_events("modify");
      }
    } else {
      $this->handle_post_events("add");
    }
  }


  function getListEntry()
  {
    $fields                 = goService::getListEntry();
    $fields['Message']      = _("Kolab mail service");
    #$fields['AllowEdit']    = true;
    return($fields);
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Kolab"),
          "plDescription" => _("Kolab mail service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 85,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
            "postfixmydomain"                => _("Postfix mydomain") ,
            "postfixmydestination"           => _("My destination") ,
            "cyrusadmins"                    => _("Cyrus admins") ,

            "cyruspop3"                      => _("POP3 service") ,
            "cyruspop3s"                     => _("POP3/SSL service") ,

            "cyrusimap"                      => _("IMAP service") ,
            "cyrusimaps"                     => _("IMAP/SSL service") ,

            "cyrussieve"                     => _("Sieve service") ,
            "proftpdftp"                     => _("FTP FreeBusy service") ,
            "apachehttp"                     => _("HTTP FreeBusy service") ,
            "postfixenablevirusscan"         => _("Enable virus scan") ,

            "cyrusquotawarn"                 => _("Quota settings"), 

            "apacheallowunauthenticatedfb"   => _("Allow unauthenticated free busy"),
            "kolabFreeBusyFuture"            => _("kolabFreeBusyFuture") ,

            "postfixmynetworks"              => _("SMTP privileged networks") ,
            "postfixmxrelayenabled"          => _("Enable MX lookup for relayhost"),
            "postfixrelayhost"               => _("Relayhost") ,
            "postfixallowunauthenticated"    => _("Accept Internet Mail"))
              ));
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
