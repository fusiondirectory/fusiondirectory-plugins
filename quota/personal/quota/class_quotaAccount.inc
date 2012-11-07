<?php

/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011  FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class quotaAccount extends plugin
{
  var $plHeadline     = "quota";
  var $plDescription  = "Plugin quota";

  var $quotaDeviceParameters  = array();
  var $quotaServersList       = array();
  var $quotaServer            = "";

  var $quotaDevice            = "";

  var $memberList             = array();

  var $quota_blocksoft        = 0;
  var $quota_blocksoft_unit   = 'K';
  var $quota_blockhard        = 0;
  var $quota_blockhard_unit   = 'K';
  var $quota_inodesoft        = 0;
  var $quota_inodehard        = 0;

  var $quota_subattributes = array( 'quota_blocksoft','quota_blocksoft_unit',
                                    'quota_blockhard','quota_blockhard_unit',
                                    'quota_inodesoft','quota_inodehard',
                                  );

  var $unit_options;

  var $view_logged            = FALSE;
  var $is_account             = FALSE;
  var $initially_was_account  = FALSE;

  var $quota = array();

  var $attributes = array("quota");
  var $objectclasses = array("systemQuotas");

  /* CLASS CONSTRUCTOR               */
  function quotaAccount(&$config, $dn= NULL)
  {
    global $class_mapping;

    /* loading global configuration of fusiondirectory */

    $this->config= $config;

    plugin::plugin($config,$dn);

    /* create a link for reading in LDAP tree */
    $ldap= $this->config->get_ldap_link();

    /* Check if DN isn't null */
    if ($dn !== NULL) {
      /* if DN exist , we check if quota clas is declared : show or not the quota screen */
      if (isset($this->attrs['objectClass']) && in_array ('quotaAccount', $this->attrs['objectClass'])) {
        $this->is_account = TRUE;
      }

      /* Save initial account state */
      $this->initially_was_account = $this->is_account;

      #fetch all information
      $tmp = array();
      if (isset($this->attrs['quota'])) {
        for ($i= 0; $i<$this->attrs['quota']['count']; $i++){
          $tmp[$this->attrs['quota'][$i]] = $this->attrs['quota'][$i];
        }
      }
      $this->quota = $tmp;
    }

    $this->unit_options = array('K'=>_('KB'),'M'=>_('MB'),'G'=>_('GB'),'T'=>_('TB'));

    /********************** SERVER SELECTION SYSTEM ***********************/
    $ldap = $this->config->get_ldap_link();

    /* Get List of server with Quota service */
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=goServer)(objectClass=quotaService))", array("cn", "description","quotaDeviceParameters"));
    while ($attrs = $ldap->fetch()) {
      $entry['cn'] = $attrs['cn'][0];
      if (isset($attrs['description'][0])) {
        $entry['description'] = $attrs['description'][0];
      } else {
        $entry['description'] = "";
      }
      $quotaDeviceParameters = array();
      for ($i= 0; $i<$attrs['quotaDeviceParameters']['count']; $i++) {
        $quotaDeviceParameters[] = $attrs['quotaDeviceParameters'][$i];
      }
      $this->quotaDeviceParameters[$entry['cn']] = $quotaDeviceParameters;
      $server_display = $entry['cn'];
      if (!empty($entry['description'])) {
        $server_display .= " - ".$entry['description'];
      }
      $this->quotaServersList[$entry['cn']] = $server_display;
    }
    if (!empty($this->quotaServersList)) {
      $this->quotaServer = key($this->quotaServersList);
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


    /*************************************************/
    /* Check if Quota part could modify bor not depend on ACL
    /*************************************************/

    if(isset($_POST['modify_state'])){
      if($this->is_account && $this->acl_is_removeable()){
        $this->is_account= FALSE;
      }elseif(!$this->is_account && $this->acl_is_createable()){
        $this->is_account= TRUE;
      }
    }

    /* Check if Quota extension is available or not*/
    if (!$this->is_account && $this->parent === NULL ) {
      $display = "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
        msgPool::noValidExtension(_("QUOTA"))."</b>";
      $display .= back_to_main();
      return $display;
    }

    /****************
      Account status
     ****************/

    if (empty($this->quotaServersList)) {
      $display = "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
                _("There is no quota server configured")."</b>";
      $display .= back_to_main();
      return $display;
    }

    /* Show tab dialog headers with a toggle button for activate/desactivate */
    if ($this->parent !== NULL) {
      if ($this->is_account) {
        $display= $this->show_disable_header(msgPool::removeFeaturesButton(_("QUOTA")), msgPool::featuresEnabled(_("QUOTA")));
      } else {
        $display= $this->show_enable_header(msgPool::addFeaturesButton(_("QUOTA")), msgPool::featuresDisabled(_("QUOTA")));
        return($display);
      }
    }

    /* Templates now! */
    $smarty = get_smarty();

    $smarty->assign("quotaServersList",$this->quotaServersList);
    $smarty->assign("quotaServer",$this->quotaServer);

    $quotaDeviceList = array();
    for ($i=0; $i<count($this->quotaDeviceParameters[$this->quotaServer]); $i++) {
      $quotas = explode(":",$this->quotaDeviceParameters[$this->quotaServer][$i]);
      $quotaDeviceList[$quotas[0].":".$quotas[1]] = $quotas[0]." - ".$quotas[2];
    }

    if (empty($this->quotaDevice)) {
      $this->quotaDevice = key($quotaDeviceList);
    }

    /* quotaDeviceList is the list of available value that could be computed */
    $smarty->assign("quotaDeviceList",$quotaDeviceList);
    $smarty->assign("quotaDevice",$this->quotaDevice);

    /* PuT acl in place : name of acl is <attribute NAME>ACL : eg quotaACL*/
    $tmp = $this->plInfo();
    foreach ($tmp['plProvidedAcls'] as $var => $rest) {
      $smarty->assign($var."ACL",$this->getacl($var));
    }

    /******************** DISPLAY LIST OF QUOTA WITH EDIT/DELETE ICON *********************/
    /* create list box */
    $QuotaList = new divSelectBox("quota");
    /* set listbox height */
    $QuotaList->SetHeight(56);
    /* store in php value html code for editing entries by POST variable */
    $link_delete = "<a href='?plug=".$_GET['plug']."&act=delete&id=%s'><img style='border: 0px' src='images/lists/trash.png'></a>";

    /* walk all values in quota object */
    foreach ($this->quota as $quota => $values ) {
      /* fetch all values for all record in object */
      $quota_exploded = explode(":",$values);
      /* fetch the blocksize following the server and device used */
      for ($i=0; $i<count($this->quotaDeviceParameters[$quota_exploded[5]]); $i++) {
        $quota_available=explode(":",$this->quotaDeviceParameters[$quota_exploded[5]][$i]);
        if ($quota_available[0] == $quota_exploded[0]) {
          $blocksize = $quota_available[1];
          break;
        }
      }
      /* convert value */
      $warning_size   = $this->ldap_to_human_size($quota_exploded[1],$blocksize);
      $critical_size  = $this->ldap_to_human_size($quota_exploded[2],$blocksize);

      /* create entries for list box */
      $QuotaList->AddEntry(array(
        array("string" => _("Server")." : " . $quota_exploded[5]),
        array("string" => _("Device")." : " . $quota_exploded[0]),
        array("string" => _("Block soft limit")." : ".$warning_size),
        array("string" => _("Block hard limit")." : ".$critical_size),
        array("string" => _("Inode soft limit")." : ".$quota_exploded[3]),
        array("string" => _("Inode hard limit")." : ".$quota_exploded[4]),
        array("string" => str_replace("%s",base64_encode($quota),$link_delete))
      ));
    }

    foreach ($this->quota_subattributes as $attr) {
      $smarty->assign("$attr",$this->$attr);
    }

    $smarty->assign("unit_options",$this->unit_options);

    /* Display template */
    $smarty->assign("QuotaList",$QuotaList->DrawList());
    $display .= $smarty->fetch(get_template_path('quota.tpl', TRUE, dirname(__FILE__)));
    return $display;
  }

  /* Validate user input
   */
  function check()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,"", "check");
    $message = plugin::check();
    return $message;
  }

  /* Save attribute modifications back to the ldap
   */
  function save()
  {
    if (isset($_POST['quotaTab'])) {
      /* Save values to object */
      plugin::save();

      $this->attrs['quota'] = array();
      foreach ($this->quota as $quota_id) {
        $this->attrs['quota'][] = $quota_id;
      }

      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);

      if (!$ldap->success()) {
        msgPool::ldaperror($ldap->get_error(), $this->dn,LDAP_MOD, get_class());
      }
    }
  }

  function save_object()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,"", "save_object");
    /******************** PRECISE DELETE QUOTA ACTION *****************************/
    /* check posts for delete zone */
    /* the id is the value stored but base64coded */
    if (isset($_GET['act']) && ($_GET['act'] == "delete")) {
      $tmp = base64_decode($_GET['id']);
      unset($this->quota[$tmp]);
    }

    if (isset($_POST['quotaTab'])) {
      plugin::save_object();

      foreach ($this->quota_subattributes as $attr) {
        if (isset($_POST["$attr"])) {
          if (in_array($attr,array('quota_blocksoft_unit','quota_blockhard_unit'))) {
            if (isset($this->unit_options[$_POST["$attr"]])) {
              $this->$attr = $_POST["$attr"];
            }
          } else {
            $this->$attr = intval($_POST["$attr"]);
          }
        }
      }

      if (isset($_POST['quotaServer']) && ($this->quotaServer != $_POST['quotaServer'])) {
        $this->quotaServer = $_POST['quotaServer'];
        unset($this->quotaDevice);
      }
      /****************** ADD A NEW QUOTA ***************************/
      elseif ((isset($_POST['addquota'])) &&
              $this->acl_is_writeable("quota")) {
        $quotaUsed    = explode(":",$this->quotaDevice);
        $quota_device = $quotaUsed[0];
        $blocksize    = $quotaUsed[1];

        $units = array(
                        'K' => 1024,
                        'M' => 1024 * 1024,
                        'G' => 1024 * 1024 * 1024,
                        'T' => 1024 * 1024 * 1024 * 1024,
                      );

        # convert human quantity on computer value
        $quota_blocksoft = ceil($this->quota_blocksoft * $units[$this->quota_blocksoft_unit] / $blocksize);

        $quota_blockhard = ceil($this->quota_blockhard * $units[$this->quota_blockhard_unit] / $blocksize);

        if ($quota_blockhard < $quota_blocksoft) {
          $quota_blockhard = $quota_blocksoft;
        }
        if ($this->quota_inodehard < $this->quota_inodesoft) {
          $this->quota_inodehard = $this->quota_inodesoft;
        }

        $quota_added =  $quota_device.":".$quota_blocksoft.":".$quota_blockhard.":".
                        $this->quota_inodesoft.":".$this->quota_inodehard.":".
                        $this->quotaServer;

        /* we add value on object */
        $this->quota[$quota_added] = $quota_added;
      }
    }
  }

  function remove_from_parent()
  {
    /* Cancel if there's nothing to do here */
    if (!$this->initially_was_account){
      return;
    }

    /* Remove GOsa attributes */
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

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove",array("uid" => $this->uid));
  }

  static function plInfo()
  {
    return array(
      "plShortName"   => _("Quota"),
      "plDescription" => _("Plugin for quota support"),
      "plSelfModify"  => TRUE,
      "plDepends"     => array("users"),
      "plPriority"    => 5,
      "plSection"     => array("personal" => _("My account")),
      "plCategory"    => array("users"),
      "plObjectType"  => array("user"),

      "plProvidedAcls" => array(
        "uid"   =>  _("uid"),
        "quota" =>  _("quota"),
      )
    );
  }

  static function ldap_to_human_size($blocknumber, $blocksize) {
    $size = $blocknumber * $blocksize;

    $mod = 1024;

    $units = array(_('B'),_('KB'),_('MB'),_('GB'),_('TB'),_('PB'),_('EB'));
    $i = 0;
    while ($size >= $mod) {
        $size /= $mod;
        $i++;
    }
    return round($size, 2).' '.$units[$i];
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
