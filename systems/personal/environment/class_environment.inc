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
class LogonScriptAttribute extends FileTextAreaAttribute
{
  function inputValue ($value)
  {
    return base64_decode($value);
  }

  function computeLdapValue ()
  {
    return base64_encode($this->getValue());
  }

  function displayValue($value)
  {
    return "md5:".md5($value);
  }
}

class environment2 extends simplePlugin
{
  var $objectclasses = array('gotoEnvironment');

  var $displayHeader = TRUE;

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Environment'),
      'plDescription' => _('Environment settings'),         // Description
      'plIcon'        => 'plugins/systems/images/plugin.png',
      'plSelfModify'  => TRUE,
      'plDepends'     => array('posixAccount'),
      'plCategory'    => array('users', 'groups'),
      'plObjectType'  => array('user', 'group'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'scripts' => array(
        'template'  => get_template_path('environment_script.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Scripts'),
        'icon'      => 'plugins/systems/images/logon_script.png',
        'attrs'     => array(
          new OrderedArrayAttribute (
            new PipeSeparatedCompositeAttribute (
              _('Logon script'),
              'gotoLogonScripts',
              array(
                new StringAttribute(
                  _('Name'), _('Filename for this script'),
                  'gotoLogonScriptName', TRUE
                ),
                new LogonScriptAttribute(
                  _('Script'), _('The script content'),
                  'gotoLogonScriptContent', TRUE
                ),
                new StringAttribute(
                  _('Description'), _('Short description for this script'),
                  'gotoLogonScriptDescription', FALSE
                ),
              ),
              '', '', '',
              _('Logon script')
            ),
            TRUE,
            array(),
            TRUE
          ),
          new OrderedArrayAttribute (
            new PipeSeparatedCompositeAttribute (
              _('Logoff script'),
              'gotoLogoffScripts',
              array(
                new StringAttribute(
                  _('Name'), _('Filename for this script'),
                  'gotoLogoffScriptName', TRUE
                ),
                new LogonScriptAttribute(
                  _('Script'), _('The script content'),
                  'gotoLogoffScriptContent', TRUE
                ),
                new StringAttribute(
                  _('Description'), _('Short description for this script'),
                  'gotoLogoffScriptDescription', FALSE
                ),
              ),
              '', '', '',
              _('Logoff script')
            ),
            TRUE,
            array(),
            TRUE
          ),
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->attributesAccess['gotoLogonScripts']->setLinearRendering(FALSE);
    $this->attributesAccess['gotoLogoffScripts']->setLinearRendering(FALSE);
  }
}

class environment extends plugin
{
  /* attribute list for save action */
  var $ignore_account       = FALSE;
  var $plHeadline           = "Environment";
  var $plDescription        = "This does something";
  var $dialog               = false;    // Indicates that we are currently editing in an seperate dialog

  var $in_dialog            = false;
  var $uid                  = "";

  var $is_group             = false;
  var $view_logged = FALSE;

  /* Attribute definition
   */

  /* Logon script section*/
  var $gotoLogonScripts   = array();// Contains all available Logon Scripts
  var $gotoLogonScript    = "";     // The selected Logon Script

  /* Logo script section*/
  var $gotoLogoffScripts   = array();// Contains all available Logoff Scripts
  var $gotoLogoffScript    = "";     // The selected Logoff Script

  /* Printer */
  var $gotoPrinter        = array();// All available Printer, with their configurations
  var $gotoPrinterSel     = "";     //  The selected Printer
  var $gosaDefaultPrinter = "";     // Default printer

  /* Hotplug Devices */
  var $gotoHotplugDevice  = array();     // Selected hotplug
  var $gotoHotplugDevices = array();// Already configured hotplug devices
  var $gotoHotplugDeviceDN= array();

  var $NewAddedPrinters   = array();
  var $NewDeletedPrinters = array();

  /* general settings */
  // Sets the attributes which will kept on page reload, which will be saved, ...

  var $CopyPasteVars = array(
                            "gotoHotplugDeviceDN",
                            "gotoLogonScripts",
                            "gotoLogonScript",
                            "gotoLogoffScripts",
                            "gotoLogoffScript",
                            "gotoPrinter",
                            "gotoPrinterSel",
                            "gosaDefaultPrinter",
                            "gotoHotplugDevices",
                            "is_group",
                            "in_dialog",
                            );

  var $attributes         = array("uid","gotoHotplugDeviceDN",
      "gotoLogonScript","gotoLogonScripts",
      "gotoLogoffScript","gotoLogoffScripts","gotoHotplugDevices",
      "gotoPrinter", "gosaDefaultPrinter");

  var $objectclasses      = array("gotoEnvironment"); // Specifies the objectClass which contains the attributes edited here
  var $cn;
  var $OrigCn;
  var $OrigUID; // Store orig UID if in template mode
  var $suffix;  // Store information weither we are editting an user or a group
  var $add_del_printer_member_was_called = false;

  var $multiple_support =TRUE;

  var $use_gotoPrinter;

  function environment (&$config, $dn= NULL)
  {
    plugin::plugin ($config, $dn);

    /* Setting uid to default */
    if(isset($this->attrs['uid'][0])){
      $this->uid = $this->attrs['uid'][0];
    }

    /* Check : Are we currently editing a group or user dialog */
    if((isset($this->attrs['cn'][0]))&&(!isset($this->attrs['uid'][0]))){
      $this->suffix="Group";
      $this->uid          = $this->attrs['cn'][0];
      $this->attrs['uid'] = $this->attrs['cn'][0];
      $this->OrigCn = $this->attrs['cn'][0];
    }else{
      $this->suffix="User";
    }

    /* Get all Printer assignments */
    $this->gotoPrinter = $this->get_printers();

    /* Prepare hotplugs */
    $this->gotoHotplugDevices = $this->get_HotplugDevices();

    /* prepare LogonScripts */
    $this->gotoLogonScripts = $this->get_LogonScripts();

    /* prepare LogoffScripts */
    $this->gotoLogoffScripts = $this->get_LogoffScripts();
  }

  function get_LogonScripts($data=NULL) {
    return $this->get_multi_attrs('gotoLogonScript', 'prep_LogonScript', $data);
  }

  function get_LogoffScripts($data=NULL) {
    return $this->get_multi_attrs('gotoLogoffScript', 'prep_LogoffScript', $data);
  }

  function get_multi_attrs($attr, $prep_method, $data=NULL) {
    if (!$data) {
      $data = $this->attrs;
    }

    $result = array();
    if((isset($data[$attr]))&&(is_array($data[$attr]))){
      unset($data[$attr]['count']);
      foreach($data[$attr] as $obj){
        list($name, $obj) = $this->$prep_method($obj);
        $result[$name]=$obj;
      }
    }

    return $result;
  }


  function get_printers($uid=NULL,$suffix=NULL) {
    if (!$uid) {
      $uid = $this->uid;
    }
    if (!$suffix) {
      $suffix = $this->suffix;
    }

    $printers = array();

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gotoPrinter)(goto".$suffix."Printer=".$this->uid."))",array("*"),TRUE);
    while($printer = $ldap->fetch()){
      $printers[$printer['cn'][0]]=$printer;
      $printers[$printer['cn'][0]]['mode']="user";
    }
    $ldap->search("(&(objectClass=gotoPrinter)(goto".$suffix."AdminPrinter=".$this->uid."))",array("*"),TRUE);
    while($printer = $ldap->fetch()){
      $printers[$printer['cn'][0]]=$printer;
      $printers[$printer['cn'][0]]['mode']="admin";
    }

    return $printers;
  }

  function get_HotplugDevices($obj_attrs=NULL) {
    if (!$obj_attrs) {
      $obj_attrs = $this->attrs;
    }

    $devs = array();
    if(isset($obj_attrs['gotoHotplugDeviceDN']) && is_array($obj_attrs['gotoHotplugDeviceDN'])){
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      for($i = 0 ; $i < $obj_attrs['gotoHotplugDeviceDN']['count'] ; $i ++){
        $ldap->cat($obj_attrs['gotoHotplugDeviceDN'][$i]);
        if($ldap->count()){
          $attrs = $ldap->fetch();
          if(isset($attrs['gotoHotplugDevice'][0])){
            list($name, $dev) = $this->prep_hotplug($attrs);
            $devs[$name] = $dev;
          }
        }else{
          msg_dialog::display(_("Warning"), sprintf(_("Device '%s' is not available anymore. It will be removed!"), $attrs['gotoHotplugDeviceDN'][$i]), WARNING_DIALOG);
        }
      }
    }

    return $devs;
  }

  function prep_hotplug($attrs) {
    $tmp      = preg_split("/\|/",$attrs['gotoHotplugDevice'][0]);
    $dev     = array();
    $dev['name']         = $name = $attrs['cn'][0];
    $dev['description']  = $tmp[0];
    $dev['id']           = $tmp[1];
    $dev['produkt']      = $tmp[2];
    $dev['vendor']       = $tmp[3];
    $dev['dn']           = $attrs['dn'];

    return array($name, $dev);
  }

  function prep_LogonScript($input) {
    return $this->prep_script($input, "Logon");
  }

  function prep_LogoffScript($input) {
    return $this->prep_script($input, "Logoff");
  }

  function prep_script($input, $type) {
    $tmp = $script = array();
    $tmp = explode("|",$input);
    $script[$type.'Name']        = $name = $tmp[0];
    $script[$type.'Priority']    = $tmp[2];
    if(preg_match("/O/i",$tmp[1])){
      $script[$type.'Overload'] = "O";
    }else{
      $script[$type.'Overload'] = "";
    }
    if(preg_match("/L/i",$tmp[1])){
      $script[$type.'Last'] = "L";
    }else{
      $script[$type.'Last'] = "";
    }
    $script[$type.'Data']        = base64_decode($tmp[3]);
    $script[$type.'Description'] = $tmp[4];

    return array($name, $script);
  }

  /* Detect type of edited object (user|group)*/
  function detect_grouptype()
  {
    if((!isset($this->parent))&&(!$this->is_account)){
      $this->is_group     = false;
    }elseif((isset($this->parent))&&(!isset($this->parent->by_object['posixAccount']))){
      $this->is_group     = true;
    }else{
      $this->is_group     = false;
    }
  }


  function execute()
  {

    /* Call parent execute */
    plugin::execute();
    /* Log view */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      if(isset($this->parent->by_object['user']) || (isset($this->attrs['objectClass']) &&in_array("gosaAccount",$this->attrs['objectClass']))){
        new log("view","users/".get_class($this),$this->dn);
      }else{
        new log("view","groups/".get_class($this),$this->dn);
      }
    }

    /* Are we editing from MyAccount and not editing a user */
    $WriteOnly = (!isset($this->parent)|| !$this->parent) && !session::is_set('edit');

    $this->detect_grouptype();

    /* Fill templating stuff */
    $smarty   = get_smarty();
    $display  = "";

    $smarty->assign("is_group",$this->is_group);

    /* Prepare all variables for smarty */
    foreach($this->attributes as $s_attr){
      /* Set value*/
      $smarty->assign($s_attr,$this->$s_attr);

      /* Set checkbox state*/
      if(empty($this->$s_attr)){
        $smarty->assign($s_attr."CHK","");
      }else{
        $smarty->assign($s_attr."CHK"," checked ");
      }

      /* Prepare ACL settings*/
      $smarty->assign($s_attr."ACL",$this->getacl($s_attr,$WriteOnly));
    }

    /* Is accout enabled | are we editing from usermenu or admin menu
       All these tab management is done here
     */


    /* Working from Usermenu an the Account is currently disbled
     * this->parent :  is only set if we are working in a list of tabs
     * is_account   :  is only true if the needed objectClass is given
     */
    if((!isset($this->parent))&&(!$this->is_account)){
      /* We are currently editing this tab from usermenu, but this account is not enabled */
      $smarty->assign("is_account",$this->is_account);
      /* Load template */
      $display .= $smarty->fetch(get_template_path('environment.tpl', TRUE));
      /* Avoid the "You are currently editing ...." message when you leave this tab */
      $display .= back_to_main();
      /* Display our message to the user */
      return $display;


      /* We are currently editing from group tabs, because
       * $this->parent is set
       * posixAccount is not set, so we are not in usertabs.
       */
    }elseif((isset($this->parent))&&(!isset($this->parent->by_object['posixAccount']))){
      $smarty->assign("is_account","true");
      $this->uid          = $this->cn;
      $this->attrs['uid'] = $this->cn;

      /* Change state if needed */
      if (isset($_POST['modify_state'])){
        if(($this->acl_is_createable() && !$this->is_account) ||
            ($this->acl_is_removeable() &&  $this->is_account)){
          $this->is_account= !$this->is_account;
        }
      }
      /* Group Dialog with enabled environment options */
      if ($this->is_account){
        $display= $this->show_enable_header(msgPool::removeFeaturesButton(_("Environment")),
            msgPool::featuresEnabled(_("Environment")));
      } else {

        /* Environment is disabled
           If theres is no posixAccount enabled, you won't be able to enable
           environment extensions
         */
        if((isset($this->parent->by_object['group']))||(isset($this->attrs['objectClass']))&&((in_array("posixAccount",$this->attrs['objectClass'])))){
          $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Environment")),
              msgPool::featuresDisabled(_("Environment")));
          return $display;
        }elseif((isset($this->parent->by_object['ogroup']))){
          $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Environment")),
              msgPool::featuresDisabled(_("Environment")));
          return $display;
        }else{
          $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Environment")),
              msgPool::featuresDisabled(_("Environment"), _("POSIX")), TRUE);
          return $display;
        }
      }
    }else{
      /* Editing from Usermenu
       *  Tell smarty that this accoutn is enabled
       */
      $smarty->assign("is_account","true");

      /* Change state if needed */
      if (isset($_POST['modify_state'])){
        if(($this->acl_is_createable() && !$this->is_account) ||
            ($this->acl_is_removeable() &&  $this->is_account)){
          $this->is_account= !$this->is_account;
        }
      }

      if(isset($this->parent)){

        // 3. Account enabled . Editing from adminmenu
        if ($this->is_account){
          $display= $this->show_enable_header(msgPool::removeFeaturesButton(_("Environment")),
              msgPool::featuresEnabled(_("Environment")));
        } else {

          if($this->parent->by_object['posixAccount']->is_account==true){
            $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Environment")),
                msgPool::featuresDisabled(_("Environment")));
            return $display;
          }else{
            $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Environment")),
                msgPool::featuresDisabled(_("Environment"), _("POSIX")), TRUE);
            return $display;
          }
        }
      }
    }

    /* Account is Account : is_accounbt=true.
     * Else we won't reach this.
     */

    /* Prepare all variables for smarty */
    foreach ($this->attributes as $s_attr) {
      /* Set value*/
      $smarty->assign($s_attr,$this->$s_attr);

      /* Set checkbox state*/
      if(empty($this->$s_attr)){
        $smarty->assign($s_attr."CHK","");
      }else{
        $smarty->assign($s_attr."CHK"," checked ");
      }

      /* Prepare ACL settings*/
      $smarty->assign($s_attr."ACL",$this->getacl($s_attr,$WriteOnly));
    }

    foreach(array("gotoHotplugDevice") as $s_attr){
      $smarty->assign($s_attr."ACL",$this->getacl($s_attr,$WriteOnly));
    }

    if($WriteOnly) {
      $smarty->assign("gotoPrinterACL","r");
    }else{
      $smarty->assign("gotoPrinterACL","rw");
    }

    /* HANDLE Profile Settings here
     * Assign available Quota settings
     * Get all available profile server
     * Get cache checkbox
     * Assign this all to Smarty
     */

    /* Logonscript Management
     * Get available LogonScripts (possibly grey out (or mark) these script that are defined for the group)
     * Perform add Delete edit Posts
     */

    /* Dialog Save */
    if(isset($_POST['LogonSave'])){

      if(!$this->acl_is_writeable("gotoLogonScript")){
        msg_dialog::display(_("Permission error"), msgPool::permModify(_("Logon scripts")), ERROR_DIALOG);
        unset($this->dialog);
        $this->dialog=FALSE;
        $this->is_dialog=false;
      }else{
        $this->dialog->save_object();
        if(count($this->dialog->check())!=0){
          foreach($this->dialog->check() as $msg){
            msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
          }
        }else{
          $tmp = $this->dialog->save();
          unset($this->dialog);
          $this->dialog=FALSE;
          $this->is_dialog=false;

          $this->gotoLogonScripts[$tmp['LogonName']]=$tmp;
        }
      }
    }


    /* Dialog Quit without saving */
    if(isset($_POST['LogonCancel'])){
      $this->is_dialog= false;
      unset($this->dialog);
      $this->dialog= FALSE;
    }

    /* Check Edit Del New Posts for a selected LogonScript */
    if($this->acl_is_writeable("gotoLogonScript") &&
        (isset($_POST['gotoLogonScriptNew'])) || isset($_POST['gotoLogonScriptEdit']) ||isset($_POST['gotoLogonScriptDel'])){

      /* New Logon Script: Open an edit dialog, we don't need a $_POST['gotoLogonScript'] here.
       * In this case we create a new Logon Script.
       */
      if(isset($_POST['gotoLogonScriptNew'])){
        $this->is_dialog = true;
        $this->dialog = new logonManagementDialog($this->config,$this->dn);
      }

      /* If we receive a Delete request and there is a Script selected in the selectbox, delete this one.
       * We only can delete if there is an entry selected.
       */
      if((isset($_POST['gotoLogonScriptDel']))&&(isset($_POST['gotoLogonScript']))){
        unset($this->gotoLogonScripts[$_POST['gotoLogonScript']]);
      }

      /* In this case we want to edit an existing entry, we open a new Dialog to allow editing.
       * There must be an entry selected to perform edit request.
       */
      if((isset($_POST['gotoLogonScriptEdit']))&&(isset($_POST['gotoLogonScript']))){
        $is_entry = $this->gotoLogonScripts[$_POST['gotoLogonScript']];
        $this->is_dialog = true;
        $this->dialog = new logonManagementDialog($this->config,$this->dn,$is_entry);
      }
    }

    /* Logoffscript Management
     * Get available LogoffScripts (possibly grey out (or mark) these script that are defined for the group)
     * Perform add Delete edit Posts
     */

    /* Dialog Save */
    if(isset($_POST['LogoffSave'])){

      if(!$this->acl_is_writeable("gotoLogoffScript")){
        msg_dialog::display(_("Permission error"), msgPool::permModify(_("Log off scripts")), ERROR_DIALOG);
        unset($this->dialog);
        $this->dialog=FALSE;
        $this->is_dialog=false;
      }else{
        $this->dialog->save_object();
        if(count($this->dialog->check())!=0){
          foreach($this->dialog->check() as $msg){
            msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
          }
        }else{
          $tmp = $this->dialog->save();
          unset($this->dialog);
          $this->dialog=FALSE;
          $this->is_dialog=false;

          $this->gotoLogoffScripts[$tmp['LogoffName']]=$tmp;
        }
      }
    }


    /* Dialog Quit without saving */
    if(isset($_POST['LogoffCancel'])){
      $this->is_dialog= false;
      unset($this->dialog);
      $this->dialog= FALSE;
    }

    /* Check Edit Del New Posts for a selected LogoffScript */
    if($this->acl_is_writeable("gotoLogoffScript") &&
        (isset($_POST['gotoLogoffScriptNew'])) || isset($_POST['gotoLogoffScriptEdit']) ||isset($_POST['gotoLogoffScriptDel'])){

      /* New Logoff Script: Open an edit dialog, we don't need a $_POST['gotoLogoffScript'] here.
       * In this case we create a new Logoff Script.
       */
      if(isset($_POST['gotoLogoffScriptNew'])){
        $this->is_dialog = true;
        $this->dialog = new LogoffManagementDialog($this->config,$this->dn);
      }

      /* If we receive a Delete request and there is a Script selected in the selectbox, delete this one.
       * We only can delete if there is an entry selected.
       */
      if((isset($_POST['gotoLogoffScriptDel']))&&(isset($_POST['gotoLogoffScript']))){
        unset($this->gotoLogoffScripts[$_POST['gotoLogoffScript']]);
      }

      /* In this case we want to edit an existing entry, we open a new Dialog to allow editing.
       * There must be an entry selected to perform edit request.
       */
      if((isset($_POST['gotoLogoffScriptEdit']))&&(isset($_POST['gotoLogoffScript']))){
        $is_entry = $this->gotoLogoffScripts[get_post('gotoLogoffScript')];
        $this->is_dialog = true;
        $this->dialog = new LogoffManagementDialog($this->config,$this->dn,$is_entry);
      }
    }

    /* Append List to smarty*/
    $ls = $this->printOutLogonScripts();
    $smarty->assign("gotoLogonScripts",  $ls);
    $smarty->assign("gotoLogonScriptKeys",array_flip($ls));
    $smarty->assign("gotoLogonScriptKeysCnt",count($ls));

    $ls = $this->printOutLogoffScripts();
    $smarty->assign("gotoLogoffScripts",  htmlencode($ls));
    $smarty->assign("gotoLogoffScriptKeys",htmlencode(array_flip($ls)));
    $smarty->assign("gotoLogoffScriptKeysCnt",count($ls));

    /* Hotplug devices will be handled here
     * There are 3 possible methods for this feature
     * Create a new Hotplug, A Dialog will open where you can specify some hotplug information
     * Delete will erase an entry, the entry must be selcted in the ListBox first
     * Editing an entry will open a dialog where the informations about the selcted entry can be changed
     */

    /* We have to delete the selected hotplug from the list*/
    if((isset($_POST['gotoHotplugDeviceDel']))&&(isset($_POST['gotoHotplugDevice_post'])) && $this->acl_is_writeable("gotoHotplugDevice")){
      if($this->acl_is_writeable("gotoHotplugDevice")){
        foreach($_POST['gotoHotplugDevice_post'] as $name){
          unset($this->gotoHotplugDevices[$name]);
        }
      }
    }

    /* There are already defined hotplugs from other users we could use */
    if(isset($_POST['gotoHotplugDeviceUse']) && $this->acl_is_writeable("gotoHotplugDevice")){
      $tmp  =array();
      foreach($this->gotoHotplugDevices as $plugs){
        $tmp[] = $plugs['name'];
      }
      $this->dialog = new hotplugSelect($this->config, get_userinfo());
      $this->is_dialog = true;
    }

    /* Dialog Aborted */
    if(isset($_POST['hotplugSelect_cancel'])){
      $this->dialog= FALSE;
      $this->is_dialog = false;
    }

    /* Dialod saved */
    if(isset($_POST['hotplugSelect_save'])){

      $res = $this->dialog->save();
      foreach($res as $hotplug){
        $name = $hotplug['cn'][0];
        $entry['dn'] = $hotplug['dn'];

        /* Set class values */
        $tmp = preg_split("/\|/",$hotplug['gotoHotplugDevice'][0]);
        $entry['name']          = $hotplug['cn'][0];
        $entry['description'] = $tmp[0];
        $entry['id']       = $tmp[1];
        $entry['produkt']      = $tmp[2];
        $entry['vendor']     = $tmp[3];
        $this->gotoHotplugDevices[$name]= $entry;
      }
      $this->dialog= FALSE;
      $this->is_dialog = false;
    }

    if($this->dialog instanceOf hotplugSelect){

      // Build up blocklist
      session::set('filterBlacklist', array('cn' => array_keys($this->gotoHotplugDevices)));
      return($this->dialog->execute());
    }

    $smarty->assign("gotoHotplugDevices",$this->printOutHotPlugDevices());
    $smarty->assign("gotoHotplugDeviceKeys",array_flip($this->printOutHotPlugDevices()));

    /* Printer Assignment will managed below
     * A printer can be assigned in two different ways and two different types
     * There are 2 types of users assigned to a printer : user and admin
     * They only differ in the member attribute they will be assigned to. user: gotoUserPrinter admin: gotoadminPrinter
     * The different types of assigning a user are : 1 assigning a user to a printer 2. assigning a group to a printer
     */

    /* First handle Add Post. Open a dialog that allows us to select a printer or two */
    if(isset($_POST['gotoPrinterAdd'])){
      $this->dialog = new printerSelect($this->config,get_userinfo());
      $this->is_dialog=true;
    }

    if(isset($_POST['printerSelect_cancel']) && $this->dialog instanceOf printerSelect){
      $this->is_dialog=false;
      $this->dialog=FALSE;
    }

    if(isset($_POST['printerSelect_save'])  && $this->dialog instanceOf printerSelect){

      $res = $this->dialog->save();
        foreach($res as $printer){

          $pname = $printer['cn'][0];
          $printerObj = new printtabs($this->config,$this->config->data['TABS']['PRINTERTABS'], $printer['dn'],"printer");
          $printerObj->set_acl_base($printer['dn']);

          $type = false;


          if($this->is_group){
            if($this->dn == "new"){
              $type = "AddGroup";
            }elseif(isset($this->NewDeletedPrinters[$pname])){
              $type = "AddGroup";
            }elseif($printerObj->by_object['printgeneric']->AddMember("AddGroup",$this->dn)){
              $type = "AddGroup";
            }
          }else{
            if(isset($this->NewDeletedPrinters[$pname])){
              $type = "AddUser";
            }elseif($this->dn == "new"){
              $type = "AddUser";
            }elseif($printerObj->by_object['printgeneric']->AddMember("AddUser",$this->dn)){
              $type = "AddUser";
            }
          }

          if($type){
            $this->gotoPrinter[$pname]=$printer;
            $this->gotoPrinter[$pname]['mode']="user";
            $this->add_del_printer_member_was_called = true;

            $this->NewAddedPrinters[$pname] = $pname;
            if(isset($this->NewDeletedPrinters[$pname])){
              unset($this->NewDeletedPrinters[$pname]);
            }
          }
        }

        $this->is_dialog=false;
        unset($this->dialog);
        $this->dialog   =FALSE;
    }

    // Display printer selection dialog
    if($this->dialog instanceOf printerSelect){

      // Build up blocklist
      session::set('filterBlacklist',array('cn' => array_keys($this->gotoPrinter)));
      return($this->dialog->execute());
    }


    if((isset($_POST['gotoPrinterDel']))&&(isset($_POST['gotoPrinterSel']))&&(!empty($_POST['gotoPrinterSel']))){
      $printer = $_POST['gotoPrinterSel'];
      foreach($printer as $pname){

        $printerObj = new printtabs($this->config,$this->config->data['TABS']['PRINTERTABS'],$this->gotoPrinter[$pname]['dn'],"printer");
        $printerObj->set_acl_base($this->gotoPrinter[$pname]['dn']);

        $type = false;
        if($this->is_group){
          if(isset($this->NewAddedPrinters[$pname])){
            $type = "Group";
          }elseif($printerObj->by_object['printgeneric']->DelMember("AddGroup",$this->cn)){
            $type = "Group";
          }
        }else{
          if(isset($this->NewAddedPrinters[$pname])){
            $type = "User";
          }elseif($printerObj->by_object['printgeneric']->DelMember("AddUser",$this->uid)){
            $type = "User";
          }
        }
        if($type){
          $this->add_del_printer_member_was_called = true;
          unset($this->gotoPrinter[$pname]);

          $this->NewDeletedPrinters[$pname] = $pname;
          if(isset($this->NewAddedPrinters[$pname])){
            UNSET($this->NewAddedPrinters[$pname]);
          }
        }
      }
    }

    if((isset($_POST['gotoPrinterEdit']))&&(isset($_POST['gotoPrinterSel']))&&(!empty($_POST['gotoPrinterSel']))){
      $printers = $_POST['gotoPrinterSel'];
      $this->add_del_printer_member_was_called = true;
      foreach($printers as $printer){
        if($this->gotoPrinter[$printer]['mode']=="user"){
          $this->gotoPrinter[$printer]['mode']="admin";
        }else{
          $this->gotoPrinter[$printer]['mode']="user";
        }
      }
    }

    if((isset($_POST['gotoPrinterDefault']))&&(isset($_POST['gotoPrinterSel']))&&(!empty($_POST['gotoPrinterSel']))){
      if($this->is_group){
        msg_dialog::display(_("Error"), _("Cannot set default printer flag for groups!"), ERROR_DIALOG);
      }else{
        if ($this->gosaDefaultPrinter == $_POST['gotoPrinterSel'][0]){
          $this->gosaDefaultPrinter= "";
        } else {
          $this->gosaDefaultPrinter= $_POST['gotoPrinterSel'][0];
        }
      }
    }

    $smarty->assign("gotoPrinter",$this->printOutPrinterDevices());

    /* General behavior */
    if(is_object($this->dialog)){
      $this->dialog->save_object();
      $disp =$this->dialog->execute();
      return($disp);
    }

    /* Als smarty vars are set. Get smarty template and generate output */
    $display .= $smarty->fetch(get_template_path('environment.tpl', TRUE,dirname(__FILE__)));
    return $display;
  }

  function remove_from_parent()
  {
    /* only if it was an account*/
    if (!$this->initially_was_account){
      return;
    }

    /* include global link_info */
    $ldap= $this->config->get_ldap_link();

    /* Remove and write to LDAP */
    plugin::remove_from_parent();

    /* Don't save our template variables */
    $skip = array("uid","gotoLogonScripts","gotoPrinter","gotoHotplugDevices" );

    /* Skip all these attributes */
    foreach($skip as $del){
      unset($this->attrs[$del]);
    }

    @DEBUG (DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__,$this->attributes, "Save");

    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs);

    if($this->is_group){
      new log("remove","groups/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("remove","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove",array("uid" => $this->uid));
  }


  /* Save data to object */
  function save_object()
  {
    /* Get all Posted vars
     * Setup checkboxes
     */
    $WriteOnly = (!isset($this->parent)|| !$this->parent) && !session::is_set('edit');
    if(isset($_POST['iamposted'])){
      plugin::save_object();
      foreach($this->attributes as $s_attr){
        if((!isset($_POST[$s_attr])) ||
            in_array($s_attr,array("gosaDefaultPrinter","gotoHotplugDevices","gotoPrinter","gotoLogonScripts","gotoLogoffScripts","uid"))) continue;
        if(!$this->acl_is_writeable($s_attr)){
          continue;
        }else{
          if(isset($_POST[$s_attr])){
            $this->$s_attr = $_POST[$s_attr];
          }else{
            $this->$s_attr = false;
          }
        }
      }
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    $this->detect_grouptype();

    if(!isset($this->attrs['objectClass'])){
      $this->attrs['objectClass']=array();
    }
    if(!$this->is_group){
      if((!((in_array("posixAccount",$this->attrs['objectClass']))||($this->parent->by_object['posixAccount']->is_account==true)))&&(!$this->is_group)){
        $message[]= msgPool::featuresDisabled(_("environment"),_("POSIX"));
      }
    }
    return ($message);
  }


  /* Save to LDAP */
  function save()
  {
    /* If group was renamed, all printer settings get lost
     */
    /* only save changed variables ....*/

    plugin::save();
    $ldap= $this->config->get_ldap_link();

    $realyUsedAttrs= array();

    /* Save already used objectClasses */
    $ocs        = $this->attrs['objectClass'];
    unset($ocs['count']);
    $this->attrs = array();
    $this->attrs['objectClass']= fusiondirectory_array_merge($ocs,$this->objectclasses);

    /* 1. Search all printers that have our uid/cn as member
     * 2. Delete this uid/cn from every single entry and save it again.
     * 2.1 There are different types of members: Users / Groups, this will be defined in $suffix
     * 2.2 And each type has two modes, Admin (e.g. 'gotoUserAdminPrinter') and Normal
     */

    $this->detect_grouptype();

    if($this->add_del_printer_member_was_called || $this->OrigCn != $this->cn){

      $types = array( "gotoUserPrinter"       => "AddUser",
          "gotoGroupPrinter"      => "AddGroup",
          "gotoUserAdminPrinter"  => "AddAdminUser",
          "gotoGroupAdminPrinter" => "AddAdminGroup");

      if($this->is_group){
        $s_suffix = "Group";
        $useVar   = "cn";
      } elseif (isset($this->OrigUID)) {
        $useVar   = "OrigUID";
        $s_suffix = "User";
      }else {
        $useVar   = "uid";
        $s_suffix = "User";
      }

      /* Remove old entries */
      $ldap->cd($this->config->current['BASE']);;
      $ldap->search("(&(objectClass=gotoPrinter)(goto".$s_suffix."Printer=".$this->$useVar."))",array("*"));
      while($attr = $ldap->fetch()){
        $printerObj = NULL;
        $printerObj = new printtabs($this->config,$this->config->data['TABS']['PRINTERTABS'],$attr['dn'],"printer");
        $printerObj->set_acl_base($attr['dn']);
        $printerObj->by_object['printgeneric']->DelMember($types["goto".$s_suffix."Printer"],$this->$useVar);
        $printerObj->by_object['printgeneric']->save();
      }

      $ldap->cd($this->config->current['BASE']);;
      $ldap->search("(&(objectClass=gotoPrinter)(goto".$s_suffix."AdminPrinter=".$this->$useVar."))",array("*"));
      while($attr = $ldap->fetch()){
        $printerObj = NULL;
        $printerObj = new printtabs($this->config,$this->config->data['TABS']['PRINTERTABS'],$attr['dn'],"printer");
        $printerObj->set_acl_base($attr['dn']);
        $printerObj->by_object['printgeneric']->DelMember($types["goto".$s_suffix."AdminPrinter"],$this->$useVar);
        $printerObj->by_object['printgeneric']->save();
      }

      foreach($this->gotoPrinter as $printer){
        $printerObj = NULL;
        $printerObj = new printtabs($this->config,$this->config->data['TABS']['PRINTERTABS'],$printer['dn'],"printer");
        $printerObj->set_acl_base($printer['dn']);


        if($printer['mode'] == "admin") {
          $attribute = "goto".$s_suffix."AdminPrinter";
        }else{
          $attribute = "goto".$s_suffix."Printer";
        }

        $printerObj->by_object['printgeneric']->AddMember($types[$attribute],$this->dn);
        $printerObj->by_object['printgeneric']->save();
      }
    }

    /* Prepare HotPlug devices */
    $this->attrs['gotoHotplugDeviceDN'] = array();
    foreach($this->gotoHotplugDevices as $name => $device){
      $this->attrs['gotoHotplugDeviceDN'][]= LDAP::fix($device['dn']);
    }

    /* Prepare LogonScripts */
    $this->attrs['gotoLogonScript'] = array();
    foreach($this->gotoLogonScripts as $name => $script){
      $this->attrs['gotoLogonScript'][] =   $script['LogonName']."|".
        $script['LogonOverload'].$script['LogonLast']."|".
        $script['LogonPriority']."|".
        base64_encode($script['LogonData'])."|".
        $script['LogonDescription'];
    }

    /* Prepare LogoffScripts */
    $this->attrs['gotoLogoffScript'] = array();
    foreach($this->gotoLogoffScripts as $name => $script){
      $this->attrs['gotoLogoffScript'][] =   $script['LogoffName']."|".
        $script['LogoffOverload'].$script['LogoffLast']."|".
        $script['LogoffPriority']."|".
        base64_encode($script['LogoffData'])."|".
        $script['LogoffDescription'];
    }

    $saveThis = array("gosaDefaultPrinter");

    foreach($saveThis as $tosave){
      if(!empty($this->$tosave)){
        $this->attrs[$tosave]=$this->$tosave;
      }else{
        $this->attrs[$tosave]=array();
      }
    }

    $ldap->cat ($this->dn, array('dn'));
    if ($ldap->fetch()){
      $mode= "modify";
    } else {
      $mode= "add";
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
    }

    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->$mode($this->attrs);

    $cat = "users";
    if($this->is_group){
      $cat = "groups";
    }

    /* Log last action */
    if($this->initially_was_account){
      new log("modify",$cat."/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("create",$cat."/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
    $this->handle_post_events($mode,array("uid"=>$this->uid));
  }

  /* Generate ListBox frindly output for the definedhotplugs
   * Possibly Add or remove an attribute here,
   */
  function printOutHotPlugDevices()
  {
    $a_return= array();
    if(is_array($this->gotoHotplugDevices)){
      foreach($this->gotoHotplugDevices as $key=>$device){
        $a_return[$key] = $device['name']." [".$device['description']."] - ".$device['id'];
      }
    }
    return($a_return);
  }

  /* Generates ListBox frienly output of used printer devices
   * Append ' - admin' if printer is used in admin mode
   */
  function printOutPrinterDevices()
  {
    $a_return = array();

    if(is_array($this->gotoPrinter)){
      foreach($this->gotoPrinter as $printer){
        if($printer['mode'] == 'admin'){
          $a_return[$printer['cn'][0]]= $printer['cn'][0]." - "._("Administrator");
        }else{
          $a_return[$printer['cn'][0]]= $printer['cn'][0];
        }
        if ($printer['cn'][0] == $this->gosaDefaultPrinter){
          $a_return[$printer['cn'][0]].=" - "._("Default printer");
        }
      }
    }
    asort($a_return);
    return($a_return);
  }

  function postCopyHook()
  {
    /* Force adding the new object to eventually
     * existing printer objects */
    $this->add_del_printer_member_was_called = TRUE;
  }

  /* Generates ListBox frienly output of used logonscripts
   */
  function printOutLogonScripts()
  {
    $a_return = array();
    if(is_array($this->gotoLogonScripts)){
      foreach($this->gotoLogonScripts as $script){
        $a_return[$script['LogonName']]= $script['LogonPriority']." - ".$script['LogonName'];
      }
    }
    return($a_return);
  }

  /* Generates ListBox frienly output of used logoffscripts
   */
  function printOutLogoffScripts()
  {
    $a_return = array();
    if(is_array($this->gotoLogoffScripts)){
      foreach($this->gotoLogoffScripts as $script){
        $a_return[$script['LogoffName']]= $script['LogoffPriority']." - ".$script['LogoffName'];
      }
    }
    return($a_return);
  }

  function adapt_from_template($tmpl_dn, $skip=array()) {

    if (isset($this->uid)) {
      $this->OrigUID = $this->uid;
    }

    $skip_types = array("gotoLogonScripts", "gotoLogoffScripts",
      "gotoPrinter","gotoHotplugDevices" );
    plugin::adapt_from_template($tmpl_dn, $skip_types);

    // Handle gotoLogonScripts
    $tmpl_logon_scripts = $this->get_LogonScripts();
    $this->gotoLogonScripts = array_merge($this->gotoLogonScripts,
      $tmpl_logon_scripts);

    // Handle logoff scripts
    $tmpl_logoff_scripts = $this->get_LogoffScripts();
    $this->gotoLogoffScripts = array_merge($this->gotoLogoffScripts,
      $tmpl_logoff_scripts);

    // Handle hotplug devices
    $tmpl_hotplug_dev = $this->get_HotplugDevices();
    $this->gotoHotplugDevices = array_merge($this->gotoHotplugDevices,
      $tmpl_hotplug_dev);

    // Handle printers
    $tmpl_printers = $this->get_printers();
    $this->gotoPrinter = array_merge($this->gotoPrinter, $tmpl_printers);
    $this->add_del_printer_member_was_called = TRUE;
  }

  /* Return plugin informations for acl handling
#FIXME these ACLs should work for groups too */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("Environment"),
      "plDescription"   => _("Environment settings"),         // Description
      "plIcon"          => "plugins/systems/images/plugin.png",
      "plSelfModify"    => TRUE,
      "plDepends"       => array("posixAccount"),
      "plCategory"      => array("users", "groups"),
      "plObjectType"    => array("user", "group"),

      "plProvidedAcls"  => array(
        "gotoHotplugDevice"   => _("Hotplug devices"),
        "gotoLogonScript"     => _("Logon script"),
        "gotoLogoffScript"    => _("Log off script")
      )
    );
  }
}
?>
