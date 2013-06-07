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

class phoneAccount extends plugin
{
  var $delimiter = "|";

  /* Attributes */
  var $telephoneNumber        = array();
  var $goFonHardware          = "automatic";
  var $goFonFormat            = "wav";
  var $goFonPIN               = "";
  var $goFonVoicemailPIN      = "";
  var $goFonDeliveryMode      = "";
  var $phoneNumbers           = array();
  var $mail                   = "";
  var $hardware_list          = array();
  var $used_hardware          = array();
  var $goFonMacro             = "";
  var $macro                  = 0;              // Selected Macro
  var $lastmacro              = "";
  var $macros                 = array();        // List of macros for smarty select box
  var $macroarray             = array();        // All needed macro informations
  var $macrostillavailable    = false;
  var $generate_error         = "";
  var $a_old_telenums         = array();
  var $goFonPINVoice          = "";
  var $goFonHomeServer        = "0";            // Contains the dn of the server that manage this account
  var $init_HomeServer        = "0";            // Contains the dn of the server that manage this account
  var $goFonHomeServers       = array();        // Contains all available server configurations

  var $context                = "default";
  var $sip_contexts           = array();
  var $voice_context          = "default";
  var $voicemail_contexts     = array();

  /* attribute list for save action */
  var $CopyPasteVars          = array("phoneNumbers","macroarray","macrostillavailable"/*"phoneNumbers" -Reset- */,
                                      "hardware_list","used_hardware");

  var $attributes             = array("goFonDeliveryMode", "goFonFormat","goFonHomeServer",
      "goFonHardware","goFonPIN","goFonVoicemailPIN","telephoneNumber", "goFonMacro","macro");
  var $objectclasses= array("goFonAccount");

  var $uid;
  var $cn;

  var $view_logged      = FALSE;
  var $mailAddress      = "";
  var $pager            = "";

  function phoneAccount (&$config, $dn= NULL, $parent= NULL)
  {
    plugin::plugin ($config, $dn, $parent);

    $this->delimiter = $config->get_cfg_value("asteriskParameterDelimiter","|");

    /* Assemble phone numbers */
    if (isset($this->attrs['telephoneNumber'])) {
      for ($i= 0; $i<$this->attrs['telephoneNumber']['count']; $i++) {
        $number= $this->attrs['telephoneNumber'][$i];
        $this->phoneNumbers[$number] = $number;
      }
    }

    $this->sip_contexts = $this->get_asterisk_sip_contexts();
    $this->voicemail_contexts = $this->get_asterisk_voicemail_contexts();

    /* Set parent object to tab object */
    if (is_object($parent)) {
      $this->parent = $parent->parent;
    }

    /* Get current uid and cn
     */
    if(isset($this->attrs['uid'][0])){
      $this->uid = $this->attrs['uid'][0];
    }
    if(isset($this->attrs['cn'][0])){
      $this->cn = $this->attrs['cn'][0];
    }
    if(isset($this->attrs['pager'][0])){
      $this->pager = $this->attrs['pager'][0];
    }

    /* If there is a parent object present, use references
     */
    if(isset($this->parent->by_object['user']->uid)){
      $this->uid = &$this->parent->by_object['user']->uid;
    }
    if(isset($this->parent->by_object['user']->cn)){
      $this->cn = &$this->parent->by_object['user']->cn;
    }
    if(isset($this->parent->by_object['user']->pager)){
      $this->pager = &$this->parent->by_object['user']->pager;
    }

    /* Check server configurations
     * Load all server configuration in $this->goFonHomeServers if available
     *  and first server as default if necessary.
     * Check if connection is successfull for the selected server $this->goFonHomeServer
     */


    /* Set available server */
    $config = session::get('config');
    if(isset($config->data['SERVERS']['FON'])){
      $this->goFonHomeServers = $config->data['SERVERS']['FON'];
    }

    $a_SETUP= array();
    if($this->is_account && isset($config->data['SERVERS']['FON']) &&
       array_key_exists('FON',$config->data['SERVERS'])
       ) {

      /* Servers defined? Watch here... */
      if (count($this->goFonHomeServers)){

        /* Set default server */
        if(empty($this->goFonHomeServer) || $this->goFonHomeServer == "0"){
          $this->goFonHomeServer= $this->goFonHomeServers[0]['DN'];
        }

        /* Remember inital home server, to be able to remove old entries */
        $this->init_HomeServer = $this->goFonHomeServer;

        /* Get config */
        if(!isset($this->goFonHomeServers[$this->goFonHomeServer])){
          msg_dialog::display(_("Obsolete entry"), sprintf(_("The current home server is not available anymore. It will be moved to '%s' if you save this entry!"), preg_replace("/,/",", ",$this->goFonHomeServers[0]['DN'])), ERROR_DIALOG);

          $this->goFonHomeServer = $this->goFonHomeServers[0]['DN'];
          $this->init_HomeServer = $this->goFonHomeServers[0]['DN'];
        }
        $cur_cfg = $this->goFonHomeServers[$this->goFonHomeServer];
        $first = false;
        foreach($this->phoneNumbers as $key => $val){
          if(!$first){
            $first = $key;
          }
        }
      }
    }

    /* Get available phone hardware
     * Search for all available phone hardware
     */
    $tmp = get_sub_list("(objectClass=goFonHardware)","phone",array(get_ou("phoneRDN")),
                  $this->config->current['BASE'],array("cn","description"), GL_SUBSEARCH);
    foreach($tmp as $attrs){
      $cn= $attrs['cn'][0];
      $description= "";
      if (isset($attrs['description'])){
        $description= " - ".$attrs['description'][0];
      }
      $this->hardware_list[$cn]= "$cn$description";
    }
    $this->hardware_list["automatic"]= _("automatic");
    ksort($this->hardware_list);


     /* Collect all usd phones
        goFonHardware set.
     */
    $deps_a = array(
        get_people_ou(),
        get_ou("ogroupRDN"),
        get_ou("serverRDN"),
        get_ou("terminalRDN"),
        get_ou("workstationRDN"),
        get_ou("printerRDN"),
        get_ou("componentRDN"),
        get_ou("phoneRDN"));

    $tmp = get_sub_list("(goFonHardware=*)","phone",$deps_a,$this->config->current['BASE'],
        array('cn','dn','goFonHardware'),GL_SUBSEARCH);
    foreach($tmp as $attrs){
      $cn = $attrs['goFonHardware'][0];
      if(isset($this->hardware_list[$cn])){
        $this->used_hardware[$cn]= $cn;
      }
    }


    /* Get available Macros
     * Search for all Macros that are visible and create
     *  an array with name and parameters
     */
    $tmp = get_sub_list("(&(objectClass=goFonMacro)(goFonMacroVisible=1))","gofonmacro",array(get_ou("phoneMacroRDN")),
                  $this->config->current['BASE'],array("displayName","goFonMacroParameter","dn","cn"), GL_NO_ACL_CHECK | GL_SUBSEARCH );


    /* Add none for no macro*/
    $this->macros['none']=_("no macro");
    $this->macro ="none";

    /* Fetch all Macros*/
    foreach($tmp as $attrs){

      $ui = get_userinfo();
      $acl = $ui->get_permissions($attrs['dn'],"gofonmacro/macro","");

      /* Skip all macros we are not able to read
          execpt, the currently selected macro.
       */
      if(!preg_match("/r/",$acl) && !preg_match("/^".preg_quote($attrs['dn'], '/')."/",$this->goFonMacro)){
        continue;
      }

      /* unset Count, we don't need that here */
      unset($attrs['displayName']['count']);

      /* Parse macro data, unset count for parameterarrays  */
      if (isset($attrs['goFonMacroParameter']['count'])){
        unset($attrs['goFonMacroParameter']['count']);
      }

      /* fill Selectfield variable with Macros */
      if(isset($attrs['displayName'][0])){
        $this->macros[$attrs['dn']] = $attrs['displayName'][0]." (".$attrs['cn'][0].")";
      }else{
        $this->macros[$attrs['dn']] = _("undefined");
      }

      /* Go through available parameters and parse all attributes, like parametername, type, default ...*/
      if((isset($attrs['goFonMacroParameter']))&&(is_array($attrs['goFonMacroParameter']))){
          foreach($attrs['goFonMacroParameter'] as $pkey=>$pval){
              /* Split Data in readable values, by delimiter !  */
              $data = explode("!",$attrs['goFonMacroParameter'][$pkey]);

              $string = $data[3];
              $string=preg_replace("/%uid/i",$this->uid,$string);
              $string=preg_replace("/%pager/i",$this->pager,$string);
              $string=preg_replace("/%context/i",$this->context,$string);
              $string=preg_replace("/%voicemailcontext/i",$this->voice_context,$string);

              if(isset($this->cn)){
                  $string=preg_replace("/%cn/i",$this->cn,$string);
              }


              $tmp = array();
              foreach($this->phoneNumbers as $phonenum){
                  $tmp[] = $phonenum;
              }

              for($i = 0 ; $i < 10; $i++){
                  if(isset($tmp[$i])){
                      $string = preg_replace("/%telephoneNumber_".($i+1)."/i",$tmp[$i],$string);
                  }
              }
              if(isset($tmp[0])){
                  $string = preg_replace("/%telephoneNumber/i",$tmp[0],$string);
              }
              $data[3] = $string;


              /* Set all attrs */
              $id = $data[0];
              $this->macroarray[$attrs['dn']][$id]['var']    ="var".$id;
              $this->macroarray[$attrs['dn']][$id]['choosen']=$data[3];
              $this->macroarray[$attrs['dn']][$id]['id']     =$id;
              $this->macroarray[$attrs['dn']][$id]['name']   =$data[1];
              $this->macroarray[$attrs['dn']][$id]['type']   =$data[2];
              $this->macroarray[$attrs['dn']][$id]['default']=$data[3];
              if($data[2] == "bool"){
                  $this->macroarray[$attrs['dn']][$id]['choosen']=$data[3];
              }
          }//foreach
      }//is_array
    }//while


    /* Parse used Macro
     * If we have a macro selected, parse it and set values
     *  in $this->macroarray[$this->macro].
     */
    $tmp = explode("!",$this->goFonMacro);
    if(is_array($tmp)){

      /* First value is the macroname */
      $this->macro = $tmp[0];

      /* Macroname saved, delete that index */
      unset($tmp[0]);

      /* Check if makro has been removed */
      if(!isset($this->macros[$this->macro])){
        $this->macrostillavailable = false;
      }else{
        $this->macrostillavailable = true;
      }

      /* for each parametervalues ( parameterID#value like 25#twentyfive) */
      foreach($tmp as $var){

        /* Split this, so we have $varar[0] = parameterID $varar[1] = SelectedValue */
        $varar = explode("#",$var);

        /* Only insert if the parameter still exists */
        if(isset($this->macroarray[$this->macro][$varar[0]])){
          /* Assign value */
          $this->macroarray[$this->macro][$varar[0]]['choosen']=$varar[1];
        }
      }
    }


    $this->a_old_telenums = $this->phoneNumbers;


    /* Get voicemail PIN from DB
     * Because every user can change his PIN directly from the phone
     *  without any update to the ldap
     * This means, the PIN in the DB is up to date
     */
    // Connect to DB server


    $num = key($this->phoneNumbers);

    if( ((isset($cur_cfg))&&
        (isset($cur_cfg['SERVER']))&&
        (isset($cur_cfg['LOGIN']))&&
        (isset($cur_cfg['PASSWORD'])))){
      $r_con = databaseManagement::connectDatabase($cur_cfg);
      if (PEAR::isError($r_con)) {
        msg_dialog::display(_("Warning"), msgPool::dbselect($cur_cfg['DB'],$r_con->getMessage()), WARNING_DIALOG);
      } else {
        $query_tmp = "SELECT ".$cur_cfg['VOICE_TABLE'].".context as 'v_context',
                             ".$cur_cfg['SIP_TABLE'].".context,
                             ".$cur_cfg['VOICE_TABLE'].".password
                       FROM  ".$cur_cfg['VOICE_TABLE'].",
                             ".$cur_cfg['SIP_TABLE']."
                       WHERE ".$cur_cfg['VOICE_TABLE'].".mailbox = ".$num."
                          AND ".$cur_cfg['SIP_TABLE'].".name='".$this->uid."'";
        $res =& $r_con->query($query_tmp);
        $vp  = $res->fetchRow(MDB2_FETCHMODE_ASSOC);
        if(!isset($vp['context'])){
          $this->is_modified= TRUE;
          msg_dialog::display(_("Warning"), sprintf(_("Cannot identify telephone extension in database, please try to save again.")), WARNING_DIALOG);
        }

        @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,$query_tmp, "Database query");
        if((isset($vp['password']))&&(!empty($vp['password']))){
          $this->goFonPINVoice = $vp['password'];
        }
        if((isset($vp['context']))&&(!empty($vp['context']))){
          $this->context = $vp['context'];
        }
        if((isset($vp['v_context']))&&(!empty($vp['v_context']))){
          $this->voice_context = $vp['v_context'];
        }
        $r_con->disconnect();
      }
    }
    $this->lastmacro=$this->macro;
  }

  /* Transaction will only work with InnoDB tables
   */
  public static function checkRealtimeTables($config)
  {
    $ret =TRUE;

    // Connect to DB server
    if( ((isset($config))&&
        (isset($config['SERVER']))&&
        (isset($config['LOGIN']))&&
        (isset($config['PASSWORD'])))){

      $r_con = databaseManagement::connectDatabase($config);
      if (PEAR::isError($r_con)) {
        new log("debug","GOfon/".get_class($this),"",array(),$r_con->getMessage());
        return FALSE;
      }

      /* Validate Table Type - it must be InnoDB to be able to use transactions
       */
      $inno_tables = array("SIP_TABLE","EXT_TABLE","VOICE_TABLE","QUEUE_TABLE","QUEUE_MEMBER_TABLE");
      foreach($inno_tables as $inno_table) {
        $sql = "show table status like '".$config[$inno_table]."';";
        $res =& $r_con->query($sql);
        $vp  = $res->fetchRow(MDB2_FETCHMODE_ASSOC);
        if (!preg_match("/^InnoDB$/i",$vp['Engine'])) {

          /* Not an InnoDB Table type, try to modify type.
           */
          $sql = "ALTER TABLE `".$config[$inno_table]."` ENGINE = INNODB; ";
          $res = $r_con->exec($sql);
          if (PEAR::isError($res)) {
            @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"<b>".$sql."</b>",
                "<b>FAILED!</b>. Transactions will not work!");
            $ret = FALSE;
          } else {
            @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"<b>".$sql."</b>",
                "<i>Table '".$config[$inno_table]."' is now of type InnoDB, this enables transactions.</i>");
          }
        } else {
          @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"",
              "<i>Table type of '".$config[$inno_table]."' OK, using transactions!</i>");
        }
      }
    }
    return($ret);
  }

  function stripInvalidChars($tele){
    /* Strip invalid chars, but maintain a leading + for international numbers */
    $t_tele= preg_replace("/[^0-9]/","",$tele);
    if (preg_match('/^\+/', $tele)) {
      $t_tele= "+".$t_tele;
    }
    return($t_tele);
  }




  /* This function generates the Database entries.
   * The Parameter 'save' could be true or false.
   *  false - means only testing no database transactions.
   *  true  - write database entries.
   *
   * 'sip_users','voice_mail' and 'extensions' table entries will be created.
   *
   * If the phone hardware is 'automatic' the table entries will only be removed
   *  and not added.
   */
  function generateExtensionEntries($save = false)
  {
    /* Check if there is at least one server available
     * If not, return and tell the user that saving failed
     */
    if(!count($this->goFonHomeServers)){
      if($save){
        msg_dialog::display(_("Configuration error"), msgPool::noserver(_("GOfon")), WARNING_DIALOG);
      }
      return(true);
    }

    /**********************
     * Attribute Initialisation
     **********************/

    $old_connection = false;

    // Get Configuration for database Server
    $s_parameter    = "";                                           // Contains paramter for selected Macro
    $r_con          = false;                                        // DB connection
    $r_db           = false;                                        // Selected DB
    $r_res          = false;                                        // Result resource
    $a_ldap_attrs   = array();                                      //

    $s_ip           = NULL;                   // Contains ip for Sip entry
    $s_host         = NULL;                   // Contains host for Sip entry
    $s_qualify      = "yes";                  // Qualify entry
    $s_pin          = NULL;                   // Entry for secret
    $s_type         = "friend";               // Entry for phone type (friend , peer ..)
                                              // Set the default to the default of the db

    $sip_data_array = array();                // Contains complete sip entry, to generate SQL syntax
    $i_old_key      = false;                  // Contains index for first old phonenumber, to delete old entries corectly
    $i_new_key      = false;                  // Contains index for first new phonenumber, to generate new  entries corectly

    $s_sip_values   = "";     // Contains string with all values for given attributes in SQL syntax
    $s_sip_keys     = "";     // Contains all needed attributes to generate sip entry in DB

    $s_sip_key      = "";     // Key for SIP entry index
    $s_sip_val      = "";     // Value for SIP entry index

    $b_first_deleted= false;  // Only delete first entry,
    $s_telenums     = "";     // for each value variable

    $i_is_accounted = false;  // Ensure that extension entry, for name to number is only once in table

    /* Prepare some basic attributes */
    $oldnums = array();

    foreach($this->a_old_telenums as $tele){
      $oldnums[]= $this->stripInvalidChars($tele);
    }
    foreach($this->phoneNumbers as $tele){
      $newnums[]= $this->stripInvalidChars($tele);
    }

    if(empty($this->uid)) trigger_error("Uid is empty.");


    /* Create voicemail entry
     */
    if ((!isset($this->cn)) || (empty($this->cn))) {
      $CNname= $this->uid;
    } else {
      $CNname= $this->cn;
    }

    $s_mail = "";
    if (isset($this->parent->by_object['mailAccount']->mail)) {
      $s_mail = $this->parent->by_object['mailAccount']->mail;
    }

    /* Get phonehardware to setup sip entry  */
    $ldap         = $this->config->get_ldap_link();
    $ldap->cd ($this->config->current['BASE']);
    $r_res        = $ldap->search("(&(objectClass=goFonHardware)(cn=".$this->goFonHardware."))", array('*'));
    $a_ldap_attrs = $ldap->fetch();

    /* Check selected phone hardware, is a default IP set? */
    if(((isset($a_ldap_attrs['goFonDefaultIP'][0]))&&($a_ldap_attrs['goFonDefaultIP'][0] != "dynamic"))){
      $s_ip       = $a_ldap_attrs['goFonDefaultIP'][0];
      $s_host     = $s_ip;
    }else{
      $s_ip       = NULL;
      $s_host     = "dynamic";
    }

    // Attribute GoFonQualify set ?
    if(isset($a_ldap_attrs['goFonQualify'])){
      $s_qualify = $a_ldap_attrs['goFonQualify'][0];
    }

    // Attribute GoFonPIN set ?
    if(isset($this->goFonPIN)){
      $s_pin      = $this->goFonPIN;
    }

    // Attribute GoFonType set ?
    if(isset($a_ldap_attrs['goFonType'])){
      $s_type = $a_ldap_attrs['goFonType'][0];
    }

    if(isset($a_ldap_attrs['goFonDmtfMode'][0])){
      $sip_data_array['dtmfmode']     = $a_ldap_attrs['goFonDmtfMode'][0];
    }else{
      $sip_data_array['dtmfmode']     ="rfc2833";
    }

    /* Check if phone number is used */
    if($this->is_number_used()){
      $this->generate_error = $this->is_number_used();
      return false;
    }



    /**********************
     * Check Server Connection Information
     **********************/

    /* Create handle for the current goFonHomeServer, if possible
     * Get configuration to old asterisk home server
     */
    $a_New = $this->goFonHomeServers[$this->goFonHomeServer];  // DB Configuration

    $new_connection = databaseManagement::connectDatabase($a_New);
    if (PEAR::isError($new_connection)) {
      $this->generate_error =  msgPool::dbconnect($a_New['SERVER'],$new_connection->getMessage(),
          _("Abort saving entries to keep the database consistent."));
      new log("debug","gofonreport/".get_class($this),"",array(),$new_connection->getMessage());
      return FALSE;
    }

    /* If the home server has changed, we must remove entries from old
     *  server and add new entries in new server.
     */
    if ($this->init_HomeServer != $this->goFonHomeServer) {

      /* Get configuration to old asterisk home server */
      $a_Remove = $this->goFonHomeServers[$this->init_HomeServer];  // DB Configuration

      /* Create connection to the database that contains the old entry.
       */
      $old_connection = databaseManagement::connectDatabase($a_Remove);
      if (PEAR::isError($old_connection)) {
        $this->generate_error =  msgPool::dbconnect($a_Remove['SERVER'],$old_connection->getMessage(),
            _("Abort saving entries to keep the database consistent."));
        new log("debug","gofonreport/".get_class($this),"",array(),$old_connection->getMessage());
        return FALSE;
      }
    }


    /* Save means that we must save changes, not only test  */
    if ($save) {

      /**********************
       * Remove entries from old home server
       **********************/

      /* Check if there is an old entry
       * If there is an old entry, get callerid and remove voicemail and extensions
       */
      if ($old_connection) {

        /* Check table definitions
         */
        if (!phoneAccount::checkRealtimeTables($a_Remove)) {
          msg_dialog::display(_("Warning"),
              sprintf(_("FusionDirectory identified problems with your database table definition!")),
              WARNING_DIALOG);
        }

        $query  = "SELECT id,name,callerid FROM ".$a_Remove['SIP_TABLE']." WHERE name='".$this->uid."';";
        $rid    =& $old_connection->query($query);
        @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,$query,
            "<i>Reguest callerid to be able to identify the user.</i>");

        /* Old entry found, remove it */
        $query_a = array();
        if ($result = $rid->fetchRow(MDB2_FETCHMODE_ASSOC)) {

          /* Set mode to strict
             Strict disallows the addition of entries that do not match the targets field length.
           */
          $query_a[]= "SET @@sql_mode = STRICT_ALL_TABLES;";
          $query_a[]= "DELETE FROM ".$a_Remove['SIP_TABLE']." WHERE name='".$this->uid."';";
          $query_a[]= "DELETE FROM ".$a_Remove['VOICE_TABLE']." WHERE customer_id='".$result['callerid']."';";
          $query_a[]= "DELETE FROM ".$a_Remove['EXT_TABLE']." WHERE exten='".$this->uid."';";
          foreach ($oldnums as $s_telenums) {
            $query_a[]= "DELETE FROM ".$a_Remove['EXT_TABLE']." WHERE exten='".$s_telenums."';";
          }

          if (!databaseManagement::executeQueriesInTransaction($old_connection,$query_a)) {
            return FALSE;
          }
        }
      }


      /**********************
       * Update / Insert sip_users entry
       **********************/

      /* Check table definitions
       */
      if(!phoneAccount::checkRealtimeTables($a_New)){
        msg_dialog::display(_("Warning"),
            sprintf(_("FusionDirectory identified problems with your database table definition!")),
            WARNING_DIALOG);
      }


      /* Set the first given phone number as callerid */
      reset($newnums);
      $i_new_key = key($newnums);
      $sip_data_array['callerid']  =$newnums[$i_new_key];
      $sip_data_array['mailbox']   =$newnums[$i_new_key]."@".$this->voice_context;

      /* Check if there is already an entry in sip_users for this uid */
      $SQL_query_array = array();

      /* Enforce strict mode, ensures inout validation, e.g. target field length
       */
      $SQL_query_array[] = "SET @@sql_mode = STRICT_ALL_TABLES;";

      $query = "SELECT * FROM ".$a_New['SIP_TABLE']." WHERE name='".$this->uid."';\n";
      $rid =& $new_connection->query($query);
      @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,$query, "Receive current database entries.");
      if ($result = $rid->fetchRow(MDB2_FETCHMODE_ASSOC)) {

        /**********************
         * Update sip_users entry
         **********************/
        $sip_data_array['host']         = $s_host;
        $sip_data_array['qualify']      = $s_qualify;
        $sip_data_array['secret']       = $this->goFonPIN;
        $sip_data_array['type']         = $s_type ;
        $sip_data_array['username']     = $this->uid;
        $sip_data_array['ipaddr']       = $s_ip;
        $sip_data_array['context']      = $this->context;

        /* Remove not changed attributes, to avoid updating table with same values */
        foreach($sip_data_array as $name => $value) {
          if ($result[$name] == $value) {
            unset($sip_data_array[$name]);
          }
        }
        /* Only update entry if there is something to uopdate */
        if (count($sip_data_array)) {
          $query = "UPDATE ".$a_New['SIP_TABLE']." SET ";
          foreach($sip_data_array as $key => $val) {
            $query.= "".$key."='".$val."',";
          }
          $query = preg_replace("/,$/","",$query);
          $query.= " WHERE name='".$this->uid."';";
          $SQL_query_array[] = $query;
        }
      } else {

        /**********************
         * Insert sip_users entry
         **********************/
        //generate SIP entry
        $sip_data_array['name']         = $this->uid;
        $sip_data_array['accountcode']  = NULL;
        $sip_data_array['amaflags']     = NULL;
        $sip_data_array['callgroup']    = NULL;
        $sip_data_array['canreinvite']  = "no";
        $sip_data_array['context']      = $this->context;
        $sip_data_array['defaultip']    = NULL;
        $sip_data_array['fromuser']     = NULL;
        $sip_data_array['fromdomain']   = NULL;
        $sip_data_array['host']         = $s_host;
        $sip_data_array['insecure']     = NULL;
        $sip_data_array['language']     = NULL;
        $sip_data_array['mailbox']      = $newnums[$i_new_key]."@".$this->voice_context;
        $sip_data_array['md5secret']    = NULL;
        $sip_data_array['nat']          = "no";
        $sip_data_array['permit']       = NULL;
        $sip_data_array['deny']         = NULL;
        $sip_data_array['mask']         = NULL;
        $sip_data_array['pickupgroup']  = NULL;
        $sip_data_array['port']         = NULL;
        $sip_data_array['qualify']      = $s_qualify;
        $sip_data_array['restrictcid']  = "n";
        $sip_data_array['rtptimeout']   = NULL;
        $sip_data_array['rtpholdtimeout']=NULL;
        $sip_data_array['secret']       = $this->goFonPIN;
        $sip_data_array['type']         = $s_type ;
        $sip_data_array['username']     = $this->uid;
        $sip_data_array['disallow']     = NULL;
        $sip_data_array['allow']        = NULL;
        $sip_data_array['musiconhold']  = NULL;
        $sip_data_array['regseconds']   = NULL;
        $sip_data_array['ipaddr']       = $s_ip;
        $sip_data_array['regexten']     = NULL;
        $sip_data_array['cancallforward']=NULL;

        /* There is currently no entry for this user in the sip_users table.
         * We should create one i
         */
        foreach($sip_data_array as $s_sip_key=>$s_sip_val) {
          if($s_sip_val === NULL) continue;
          $s_sip_values.="'".$s_sip_val."',";
          $s_sip_keys  .="`".$s_sip_key."`,";
        }
        $s_sip_values =  preg_replace("/,$/","",$s_sip_values);
        $s_sip_keys   =  preg_replace("/,$/","",$s_sip_keys);

        /* Add sip entries to queries */
        $SQL_query_array[] ="INSERT INTO ".$a_New['SIP_TABLE']." (".$s_sip_keys.") VALUES (".$s_sip_values.");";
      }


      /**********************
       * Update / Insert Voice mail entry
       **********************/

      $customer_id = $newnums[$i_new_key];
      $query  = "SELECT id,name,callerid FROM ".$a_New['SIP_TABLE']." WHERE name='".$this->uid."';";
      $rid    =& $new_connection->query($query);

      @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,$query, "Receive callerid");
      $result = $rid->fetchRow(MDB2_FETCHMODE_ASSOC);

      $old_customer_id = "";
      if($result){
        $old_customer_id = $result['callerid'];
      }

      $voice_data_array = array(
        "customer_id" => $customer_id,
        "mailbox"     => $customer_id,
        "password"    => $this->goFonVoicemailPIN,
        "fullname"    => $CNname,
        "context"     => $this->voice_context,
        "email"       => $s_mail
      );

      $voice_data_array['pager']   = $this->pager;

      /* Check if there is already an entry in sip_users for this uid */
      $query_tmp = "SELECT * FROM ".$a_New['VOICE_TABLE']." WHERE customer_id='".$old_customer_id."';\n";
      $rid =& $new_connection->query($query_tmp);

      @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,$query_tmp, "Check if voicemail entry exists");
      if ($result = $rid->fetchRow(MDB2_FETCHMODE_ASSOC)) {

        /**********************
         * Update Voice mail entry
         **********************/

        @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"", "<i>Voicemail entry exists, adding updating to queue.</i>");

        foreach ($voice_data_array as $name => $value) {
          if ($result[$name] == $value) {
            unset($voice_data_array[$name]);
          }
        }

        /* Only update entry if there is something to update */
        if (count($voice_data_array)) {
          $query = "UPDATE ".$a_New['VOICE_TABLE']." SET ";
          foreach ($voice_data_array as $key => $val) {
            $query.= "".$key."='".$val."',";
          }
          $query = preg_replace("/,$/","",$query);
          $query.= " WHERE customer_id='".$old_customer_id."';";
          $SQL_query_array[] = $query;
        }
      } else {

        /**********************
         * Insert Voice mail entry
         **********************/
        @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"", "<i>No voicemail entry found, add 'create' to queue.</i>");
        $voice_data_array['context'] = $this->voice_context;

        /* There is currently no voice mail entry for this user.
         * We should create one
         */
        $s_voi_values = $s_voi_keys = "";
        foreach($voice_data_array as $s_voi_key=>$s_voi_val) {
          if($s_voi_val === NULL) continue;
          $s_voi_values.="'".$s_voi_val."',";
          $s_voi_keys  .="`".$s_voi_key."`,";
        }
        $s_voi_values =  preg_replace("/,$/","",$s_voi_values);
        $s_voi_keys   =  preg_replace("/,$/","",$s_voi_keys);

        /* Add sip entries to queries */
        $SQL_query_array[] ="INSERT INTO ".$a_New['VOICE_TABLE']." (".$s_voi_keys.") VALUES (".$s_voi_values.");";
      }


      /**********************
       * Remove/Insert extension entries
       **********************/

      /* Initiate transaction
       */
      $SQL_query_array[]= "DELETE FROM ".$a_New['EXT_TABLE']." WHERE exten=\"".$this->uid."\";";
      $oldnums= array();
      foreach($oldnums as $s_telenums) {
        $SQL_query_array[]= "DELETE FROM ".$a_New['EXT_TABLE']." WHERE exten=\"".$s_telenums."\";";
      }
      foreach($newnums as $s_telenums) {
        $SQL_query_array[]= "DELETE FROM ".$a_New['EXT_TABLE']." WHERE exten=\"".$s_telenums."\";";
      }

      /**********************
       * Insert extension entries
       **********************/

      // Get selected Macro Parameter and create parameter entry
      if (isset($this->macroarray[$this->macro])) {
        foreach($this->macroarray[$this->macro] as $key => $val ) {
          $s_parameter .= $val['choosen'].$this->delimiter;
        }
        $s_parameter = rtrim($s_parameter, $this->delimiter);
      }

      $i = 0;
      $EXT = array();
      if (!is_numeric($this->uid)) {
        $EXT[$i]['context'] = 'FusionDirectory';
        $EXT[$i]['exten']   = $this->uid;
        $EXT[$i]['priority']= 1;
        $EXT[$i]['app']     = "Goto";
        $EXT[$i]['appdata'] = $newnums[$i_new_key].$this->delimiter."1";
        $i ++;
      }

      // Entension entries  Hint / Dial / Goto
      foreach($newnums as $s_telenums) {

        /* Hint Entry */
        $EXT[$i]['context'] = 'FusionDirectory';
        $EXT[$i]['exten']   = $s_telenums;
        $EXT[$i]['priority']= 0;
        $EXT[$i]['app']     = 'SIP/'.$this->uid;
        $i ++;
        /* SetCID */
        //$EXT[$i]['context'] = 'FusionDirectory';
        //$EXT[$i]['exten']   = $s_telenums;
        //$EXT[$i]['priority']= 1;
        //$EXT[$i]['app']     = "SetCIDName";
        //$EXT[$i]['appdata'] = $CNname;
        //$i ++;

        // If no macro is selected use Dial
        if ($this->macro!="none") {
          $macroname = preg_replace("/,.*$/","",$this->macro);
          $macroname = preg_replace("/^.*=/","",$macroname);
          $s_app = "Macro";$macroname;
          $s_par = $macroname.$this->delimiter.$s_parameter;
        } else {
          $s_app = "Dial";
          $s_par = 'SIP/'.$this->uid.$this->delimiter."20".$this->delimiter."r";
        }

        $EXT[$i]['context'] = 'FusionDirectory';
        $EXT[$i]['exten']   = $s_telenums;
        $EXT[$i]['priority']= 1;
        $EXT[$i]['app']     = $s_app;
        $EXT[$i]['appdata'] = $s_par;
        $i ++;
      }

      // Append all these Entries
      foreach($EXT as $entr) {
        $SQL_syn = "INSERT INTO ".$a_New['EXT_TABLE']." (";
        foreach($entr as $key2 => $val2) {
          $SQL_syn.= "`".$key2."`,";
        }
        $SQL_syn = preg_replace("/,$/","",$SQL_syn);
        $SQL_syn .= ") VALUES (";
        foreach($entr as $key2 => $val2){
          $SQL_syn .= "'".$val2."',";
        }
        $SQL_syn = preg_replace("/,$/","",$SQL_syn);
        $SQL_syn .=");\n";

        $SQL_query_array[] =$SQL_syn;
        $SQL_syn ="";
      }

      if (!databaseManagement::executeQueriesInTransaction($new_connection,$SQL_query_array)) {
        return FALSE;
      }
    }
    $new_connection->disconnect();
    return true;
  }


  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Log view */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","user/".get_class($this),$this->dn);
    }

    $display = "";
    $SkipWrite = (!isset($this->parent) || !$this->parent) && !session::is_set('edit');

    if(empty($this->macro)&&(!empty($this->goFonMacro))){

      /* Go through already saved values, for a parameter */
      $tmp = explode("!",$this->goFonMacro);

      /* it is possible that nothing has been saved yet */
      if(is_array($tmp)){

        /* First value is the macroname */
        $this->macro = $tmp[0];

        /* Macroname saved, delete that index */
        unset($tmp[0]);

        /* Check if macro has been removed */
        if(!isset($this->macroarray[$this->macro])){
          $this->macrostillavailable = false;
        }else{
          $this->macrostillavailable = true;
        }

        /* for each parametervalues ( parameterID#value like 25#twentyfive) */
        foreach($tmp as $var){

          /* Split this, so we have $varar[0] = parameterID $varar[1] = SelectedValue */
          $varar = explode("#",$var);

          /* Only insert if the parameter still exists */
          if(isset($this->macroarray[$this->macro][$varar[0]])){
            /* Assign value */
            $this->macroarray[$this->macro][$varar[0]]['choosen']=$varar[1];
          }
        }
      }
    }

    /* Do we represent a valid account? */
    if (!$this->is_account && $this->parent === NULL){
      $display= "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
        msgPool::noValidExtension(_("Phone"))."</b>";
      $display.= back_to_main();
      return ($display);
    }

    /* Do we need to flip is_account state? */
    if (isset($_POST['modify_state'])){
      $this->is_account= !$this->is_account;
    }

    /* Do we represent a valid account? */
    if (!$this->is_account && $this->parent === NULL){
      $display= "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
        msgPool::noValidExtension(_("Phone"))."</b>";
      $display.= back_to_main();
      return($display);
    }

    $display= "";

    /* Show tab dialog headers */

    if ($this->parent !== NULL){
      if ($this->is_account){
        $display= $this->show_disable_header(_("Remove phone account"),
            msgPool::featuresEnabled(_("Phone")));
      } else {
        if(empty($this->uid)){
          $display= $this->show_enable_header(_("Create phone account"),
            msgPool::featuresDisabled(_("Phone"),_("User uid")));
        }else{
          $display= $this->show_enable_header(_("Create phone account"),
            msgPool::featuresDisabled(_("Phone")));
        }
        return ($display);
      }
    }
    /* Select no macro if, state is empty, this is the case, if the selected macro is no longer available */
    if(empty($this->macro)){
      $this->macro ="none";
    }

    /* Prepare templating */
    $smarty= get_smarty();

    /* tell user that the selected plugin is no longer available */
    if((!$this->macrostillavailable)&&($this->macro!="none")){
      msg_dialog::display(_("Error"), _("Selected macro is not available anymore!"), ERROR_DIALOG);
    }

    /* Assing macroselectbox values  */
    $smarty->assign("macros",$this->macros);
    $smarty->assign("macro", $this->macro);

    /* Assign contexts */
    $smarty->assign("voicemail_contexts",$this->voicemail_contexts);
    $smarty->assign("sip_contexts",$this->sip_contexts);
    $smarty->assign("context" ,$this->context);
    $smarty->assign("voice_context" ,$this->voice_context);

    /* check if there is a FON server created */
    if(!count($this->goFonHomeServer)){
      msg_dialog::display(_("Configuration error"), msgPool::noserver(_("GOfon")), WARNING_DIALOG);
    }

    /* Create html parameter table for selected macro parameters
     *  skip if no parameters given
     */
    if(!isset($this->macroarray[$this->macro])){
      $macrotab="";
    }else{

      $macrotab ="<table summary=\""._("Parameter")."\">";
      /* for every single parameter-> display textfile,combo, or true false switch*/

      foreach($this->phoneNumbers as $phonenum){
        $tmp[] = $phonenum;
      }

      if($this->macro != $this->lastmacro){
        /* Go through all params */
        foreach($this->macroarray[$this->macro] as $key => $paras){

          $string = $paras['default'];

          $string=preg_replace("/%uid/i",$this->uid,$string);
          $string=preg_replace("/%pager/i",$this->pager,$string);
          $string=preg_replace("/%context/i",$this->context,$string);
          $string=preg_replace("/%voicemailcontext/i",$this->voice_context,$string);

          if(isset($this->cn)){
            $string=preg_replace("/%cn/i",$this->cn,$string);
          }

          for($i = 0 ; $i < 10; $i++){
            if(isset($tmp[$i])){
              $string = preg_replace("/%telephoneNumber_".($i+1)."/i",$tmp[$i],$string);
            }
          }
          if(isset($tmp[0])){
            $string = preg_replace("/%telephoneNumber/i",$tmp[0],$string);
          }
          $this->macroarray[$this->macro][$key]['choosen']=$string;
        }
      }

      foreach($this->macroarray[$this->macro] as $paras){

        /* get al vars */
        $var        = $paras['var'];
        $name       = $paras['name'];
        $default    = $paras['default'];
        $type       = $paras['type'];
        $choosen    = $paras['choosen'] ;
        $str        = $default;

        $dis = "";
        if(!$this->acl_is_writeable("goFonMacro",$SkipWrite)){
          $dis = " disabled ";
        }

        /* in case of a combo box display a combobox with selected attr */
        $macrotab.= "<tr>";
        switch ($type){

          case "combo":
            $str= "<select name='".$var."' ".$dis." >";
          foreach(explode(":",$default) as $choice){
            if($choosen==$choice){
              $str.= "\n<option value='".$choice."' selected>".$choice."&nbsp;</option>";
            }else{
              $str.= "\n<option value='".$choice."'>".$choice."&nbsp;</option>";
            }
          }
          $str.="</select>";
          $macrotab.= "<td>".base64_decode($name)."</td><td>$str";
          break;

          case "bool":
            if(!$choosen){
              $str="\n<input type='checkbox' name='".$var."' value='1' ".$dis." >";
            }else{
              $str="\n<input type='checkbox' name='".$var."' value='1' checked  ".$dis.">";
            }
          $macrotab.= "<td colspan='2'>$str&nbsp;".base64_decode($name)."";
          break;

          case "string":
            $str="<input name='".$var."' value='".$choosen."' ".$dis." style='width:340px;'>";
          $macrotab.= "<td>".base64_decode($name)."</td><td>$str";
          break;

        }
        $macrotab.= "</td></tr>";

      }
      $macrotab.="</table><input name='post_success' type='hidden' value='1'>";
    }//is_array()

    /* Give smarty the table */
    $smarty->assign("macrotab",$macrotab);


    /* Add phone number */
    if (isset($_POST["add_phonenumber"]) && $_POST['phonenumber']){

      if(strlen(trim($_POST["phonenumber"])) > 20 ){
        msg_dialog::display(_("Error"), msgPool::toobig("Phone number"), ERROR_DIALOG);
      }elseif (tests::is_phone_nr($_POST['phonenumber'])){
        $number= trim($_POST["phonenumber"]);
        $this->phoneNumbers[$number]= $number;
        $this->is_modified= TRUE;
      } else {
        msg_dialog::display(_("Error"), msgPool::invalid("Phone number"), ERROR_DIALOG);
      }
    }

    /* Remove phone number */
    if (isset($_POST["delete_phonenumber"]) && isset($_POST["phonenumber_list"])){
      foreach ($_POST['phonenumber_list'] as $number){
        unset($this->phoneNumbers[$number]);
        $this->is_modified= TRUE;
      }
    }

    /* Assign acls */
    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name,$SkipWrite));
    }

    /* Transfer ACL's */
    foreach($this->attributes as $val){
      if(isset($this->$val)){
        $smarty->assign($val,$this->$val);
      }else{
        $smarty->assign($val,"");
      }
    }

    /* Create home server array */
    $tmp = array();
    foreach($this->goFonHomeServers as $dn => $attrs){
      if(!is_numeric($dn)){
        $tmp[$dn] = $attrs['SERVER'];
      }
    }
    $smarty->assign("goFonHomeServers",$tmp);

    /* Fill arrays */
    $smarty->assign ("goFonHardware", $this->goFonHardware);
    if (!count($this->phoneNumbers)){
      $smarty->assign ("phoneNumbers", array());
    } else {
      $smarty->assign ("phoneNumbers", $this->phoneNumbers);
    }

    $dis = "";
    if(!$this->acl_is_writeable("goFonHardware",$SkipWrite)){
      $dis= " disabled ";
    }
    $hl= "<select size=\"1\" name=\"goFonHardware\" ".$dis." title=\"".
       _("Choose your private phone")."\">\n";

    foreach ($this->hardware_list as $cn => $description){
      if ($cn == $this->goFonHardware){
        $selected= "selected";
      } else {
        $selected= "";
      }
      if (isset($this->used_hardware[$cn])){
        $color= "style=\"color:#A0A0A0\"";
      } else {
        $color= "";
      }
      $hl.= "  <option $color label=\"$cn\" value=\"$cn\" $selected>$description&nbsp;</option>\n";
    }
    $hl.= "</select>\n";
    $smarty->assign ("hardware_list", $hl);


    foreach($this->attributes as $attr){
      if(in_array($attr,$this->multi_boxes)){
        $smarty->assign("use_".$attr,TRUE);
      }else{
        $smarty->assign("use_".$attr,FALSE);
      }
    }

    foreach(array("goFonVoiceMailContext","goFonContext") as $attr){
      if(in_array($attr,$this->multi_boxes)){
        $smarty->assign("use_".$attr,TRUE);
      }else{
        $smarty->assign("use_".$attr,FALSE);
      }
    }

    /* Show main page */
    $this->lastmacro = $this->macro;
    $display.= $smarty->fetch(get_template_path('generic.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }


  function save_object()
  {
    $SkipWrite = (!isset($this->parent) || !$this->parent) && !session::is_set('edit');
    if (isset($_POST["phoneTab"])){

      plugin::save_object();

      /* Save checkbox */
      $tmp = preg_replace("/[^a-z]/i","",$this->goFonDeliveryMode);
      if($this->acl_is_writeable("goFonDeliveryMode",$SkipWrite)){
        if(isset($_POST['fon_to_mail']) && !preg_match("/M/",$this->goFonDeliveryMode)){
          $tmp .= "M";
        }elseif(!isset($_POST['fon_to_mail']) && preg_match("/M/",$this->goFonDeliveryMode)){
          $tmp  = preg_replace ("/M/","",$tmp);
        }
      }
      $this->goFonDeliveryMode= "[".$tmp."]";


      /* Every macro in the select box are available */
      if((isset($_POST['macro']))){
        $this->macro = $_POST['macro'];
        $this->macrostillavailable=true;
      }

      if(isset($_POST['macro']) && $_POST['macro'] != $this->macro){
        $this->is_modified =true;
      }

      /* Save context */
      if(isset($_POST['context'])){
        if($this->context != $_POST['context']){
          $this->is_modified= TRUE;
        }
        $this->context= $_POST['context'];
      }

      /* Save voice context */
      if(isset($_POST['voice_context'])){
        if($this->voice_context != $_POST['voice_context']){
          $this->is_modified= TRUE;
        }
        $this->voice_context= $_POST['voice_context'];
      }

      if(is_array($this->phoneNumbers)){
        foreach($this->phoneNumbers as $telenumms) {
          $nummsinorder[]=$telenumms;
        }
      }else{
        $nummsinorder=array("");
      }


      /* get all Postvars */
      if(isset($this->macroarray[$this->macro])){


        if($this->acl_is_writeable("goFonMacro",$SkipWrite)){
          foreach($this->macroarray[$this->macro] as $key => $paras){

            $old_macro_settings = $this->macroarray[$this->macro][$key];
            $backup = $this->macroarray[$this->macro][$key];

            if(isset($_POST[$paras['var']])){
              $this->macroarray[$this->macro][$key]['choosen'] = $_POST[$paras['var']];
            }

            /* Checkboxes are special, they are not Posted if they are not selected, so the won't be changed with the above code
               We need this code below to read and save checkboxes correct
             */

            if(isset($_POST['post_success'])){
              if($this->macroarray[$this->macro][$key]['type']=="bool"){
                if(isset($_POST[$this->macroarray[$this->macro][$key]['var']])) {
                  $this->macroarray[$this->macro][$key]['choosen']=1;
                }else{
                  $this->macroarray[$this->macro][$key]['choosen']=0;
                }
              }
            }
            if(array_differs($old_macro_settings,$this->macroarray[$this->macro][$key])){
              $this->is_modified = TRUE;
            }
          }

          if(count(array_diff($this->macroarray[$this->macro][$key],$backup))){
            $this->is_modified = TRUE;
          }
        }
      }
    }
  }

  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    if(!count($this->goFonHomeServers)){
      $message[] = _("There must be at least one server with an asterisk database to create a phone account.");
    }

    if(empty($this->goFonHomeServer)){
      $message[] = msgPool::invalid(_("Home server"));
    }

    if((strlen($this->goFonVoicemailPIN)==0)||(strlen($this->goFonVoicemailPIN)>4)){
      $message[]= msgPool::invalid(_("Voicemail PIN"),"","",_("Between 1-4 charactes"));
    }else{
      if(preg_match("/[^0-9]/",$this->goFonVoicemailPIN)){
        $message[]= msgPool::invalid(_("Voicemail PIN"),preg_replace("/[0-9]/","X",$this->goFonVoicemailPIN),"/X/");
      }
    }

    if(preg_match("/[^0-9a-z]/i",$this->goFonPIN)){
      $message[]= msgPool::invalid(_("Phone PIN"),preg_replace("/[0-9a-z]/i","X",$this->goFonPIN),"/X/");
    }

    if ($this->initially_was_account != $this->is_account || $this->is_modified){
      $str = $this->generateExtensionEntries(false);
      if(empty($str)){
        msg_dialog::display(_("Error"), $str, ERROR_DIALOG);
      }
    }

    /* We need at least one phone number */
    if (count($this->phoneNumbers) == 0){
      $message[]= msgPool::required("Phone number");
    }

    /* Do not allow to save duplicate phone numbers
     *  this may destroy the extensions table.
     */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $numberFilter = "";
    foreach($this->phoneNumbers as $number){
      $numberFilter .= "(telephoneNumber={$number})";
    }
    $ldap->search("(&(!(uid=".$this->uid."))(objectClass=goFonAccount)(|{$numberFilter}))",array("dn","telephoneNumber"));
    $res = array();
    while($attrs = $ldap->fetch()){
      unset($attrs['telephoneNumber']['count']);
      $res = array_merge($res,array_intersect($attrs['telephoneNumber'], $this->phoneNumbers));
    }
    $res = array_unique($res);
    if(count($res)){
      $message[] = msgPool::duplicated(_("Phone number"))."&nbsp;<br>".
        implode(array_intersect($res, $this->phoneNumbers), ", ");
    }

    /* check for ! in any parameter setting*/
    if(isset($this->macroarray[$this->macro])){
      foreach($this->macroarray[$this->macro] as $val){
        if((strstr($val['choosen'],"!"))||(strstr($val['choosen'],"#"))){
          $message[] = msgPool::invalid(sprintf(_("macro parameter %s"),$val['name']),$val['choosen'],"/[^\#]/");
        }
      }
    }
    return ($message);
  }



  function save()
  {
    plugin::save();

    /* Force saving macro again
     * This ensures that
     *  - the macro is available on the destiantion server.
     *  - the macro saved is up to date on the destination server.
     */
    if(!empty($this->macro) && $this->macro != "none")  {
      $macro_tab= new macrotabs($this->config,$this->config->data['TABS']['MACROTABS'], $this->macro,"gofonmacro");
      $macro_tab -> save();
    }

    /* Save arrays */
    $tmp_numbers = array();
    foreach ($this->phoneNumbers as $number){
      $tmp_numbers[] = $number;
    }

    /* Save settings, or remove goFonMacro attribute*/
    if($this->macro!="none"){
      $this->attrs['goFonMacro']=$this->macro;
      if(isset($this->macroarray[$this->macro])){
        foreach($this->macroarray[$this->macro] as $paras)  {
          $this->attrs['goFonMacro'].="!".$paras['id']."#".$paras['choosen'];
        }
      }
    }else{
      $this->attrs['goFonMacro']=array();
    }
    unset($this->attrs['macro'])  ;

    $this->attrs['goFonForwarding']=array();

    /*
     */
    $str = $this->generateExtensionEntries(true);
    if(!$str){
      msg_dialog::display(_("Error"),_("An error occured while updating the database entries!") , ERROR_DIALOG);
    }

    if($this->attrs['goFonMacro']==""){
      $this->attrs['goFonMacro']=array();
    }

    unset($this->attrs['cn']);

    /* Write back to ldap */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $this->cleanup();

    /* Force saving numbers, else it will be overwriten by user account. */
    $this->attrs['telephoneNumber'] =$tmp_numbers;
    $ldap->modify ($this->attrs);

    /* Log last action */
    if($this->initially_was_account){
      new log("modify","user/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("create","user/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    /* Optionally execute a command after we're done */

    if ($this->initially_was_account == $this->is_account){
      if ($this->is_modified){
        $this->handle_post_events("modify",array("uid" => $this->uid));
      }
    } else {
      $this->handle_post_events("add",array("uid" => $this->uid));
    }

  }


  function adapt_from_template($dn, $skip= array())
  {
    plugin::adapt_from_template($dn, $skip);

    /* Assemble phone numbers */
    if (isset($this->attrs['telephoneNumber']) && !in_array("telephoneNumber", $skip)){
      for ($i= 0; $i<$this->attrs['telephoneNumber']['count']; $i++){
        $number= $this->attrs['telephoneNumber'][$i];
        $this->phoneNumbers[$number]= $number;
      }
    }
  }


  function remove_from_parent()
  {
    if(!$this->initially_was_account) return;

    if (count($this->goFonHomeServers) && !empty($this->init_HomeServer)) {

      // Get Configuration for initial database Server
      $a_SETUP = $this->goFonHomeServers[$this->init_HomeServer];
      $s_parameter  ="";

      /* Check table definitions
       */
      if (!phoneAccount::checkRealtimeTables($a_SETUP)) {
        msg_dialog::display(_("Warning"),
            sprintf(_("FusionDirectory identified problems with your database table definition!")),
            WARNING_DIALOG);
      }

      // Connect to DB server
      $r_con = databaseManagement::connectDatabase($a_SETUP);
      // Check if we are  connected correctly
      if (PEAR::isError($r_con)) {
        msg_dialog::display(_("Error"), msgPool::dbconnect("GOfon",$r_con->getMessage()), ERROR_DIALOG);
        new log("debug","gofonreport/".get_class($this),"",array(),$r_con->getMessage());
        return false;
      }

      $SQL="";
      $SQL[]= "SET @@sql_mode = STRICT_ALL_TABLES;";

      $first_num = false;
      // Delete old entries
      foreach($this->a_old_telenums as $s_telenums){
        if(!$first_num){
          $first_num = $s_telenums;
        }
        $SQL[] = "DELETE FROM ".$a_SETUP['EXT_TABLE']." WHERE exten='".$s_telenums."';\n";
      }


      $query  = "SELECT id,name,callerid FROM ".$a_SETUP['SIP_TABLE']." WHERE name='".$this->uid."';";
      $rid    =& $r_con->query($query);
      @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,$query, "Database query");
      $result = $rid->fetchRow(MDB2_FETCHMODE_ASSOC);
      $callerid = $first_num;
      if ($result) {
        $callerid = $result['callerid'];
      }

      /* Set mode to strict
         Strict disallows the addition of entries that do not match the targets field length.
       */
      $SQL[] = "DELETE FROM ".$a_SETUP['VOICE_TABLE']." WHERE customer_id='".$callerid."';";
      $SQL[] = "DELETE FROM ".$a_SETUP['EXT_TABLE']." WHERE exten='".$this->uid."';\n";
      $SQL[] = "DELETE FROM ".$a_SETUP['SIP_TABLE']." WHERE name='".$this->uid."';\n";

      if (!databaseManagement::executeQueriesInTransaction($r_con,$SQL)) {
        return FALSE;
      }

    } else {
      msg_dialog::display(_("Configuration error"), _("Was not able to found any asterisk server"), WARNING_DIALOG);
      return FALSE;
    }

    /* unset macro attr, it will cause an error */
    $tmp = array_flip($this->attributes);
    unset($tmp['macro']);
    $this->attributes=array_flip($tmp);

    /* Cancel if there's nothing to do here */
    if (!$this->initially_was_account){
      return;
    }

    plugin::remove_from_parent();

    /* Just keep one phone number */
    if (count($this->telephoneNumber) && $this->telephoneNumber != ""){
      $this->attrs['telephoneNumber']= $this->telephoneNumber;
    } else {
      $this->attrs['telephoneNumber']= array();
    }


    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=goFonQueue)(member=*))", array("member"));
    while($attr = $ldap->fetch()){
      if(in_array($this->dn,$attr['member'])){
        $new =new ogrouptabs($this->config, $this->config->data['TABS']['OGROUPTABS'],$attr['dn']);
        unset($new->by_object['ogroup']->memberList[$this->dn]);
        unset($new->by_object['ogroup']->member[$this->dn]);
        $new->save();
        msg_dialog::display(_("Information"), sprintf(_("User '%s' has been removed from phone queue '%s'."), $this->cn, $new->by_object['ogroup']->cn), INFO_DIALOG);
      }
    }
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs);

    new log("remove","user/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
    }

    /* Optionally execute a command after we're done */
    $r_con->disconnect();
    $this->handle_post_events('remove',array("uid"=> $this->uid));
  }



  /* This function checks if the given phonenumbers are available or already in use*/
  function is_number_used()
  {
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(|(objectClass=goFonAccount)(objectClass=goFonQueue)(objectClass=goFonConference))", array("telephoneNumber","cn","uid"));
    while($attrs = $ldap->fetch()) {
      unset($attrs['telephoneNumber']['count']);
      foreach($attrs['telephoneNumber'] as $tele){
        if(!isset($attrs['cn'][0])) $attrs['cn'][0]=$attrs['dn'];
        if(!isset($attrs['uid'][0])) $attrs['uid'][0]=$attrs['dn'];
        $numbers[$tele]=$attrs;
      }
    }

    foreach($this->phoneNumbers as $num){
      if((isset($numbers[$num]))&&(($numbers[$num]['uid'][0]!=$this->uid))){
        if(isset($numbers[$num]['uid'][0])){
          return sprintf(_("The specified telephonenumber '%s' is already assigned to '%s'."),$num,$numbers[$num]['uid'][0]);
        }else{
          return sprintf(_("The specified telephonenumber '%s' is already assigned to '%s'."),$num,$numbers[$num]['cn'][0]);
        }
      }
    }
  }

  function allow_remove()
  {
    /* Check if previously selected server is still available */
    if ($this->initially_was_account && !isset($this->goFonHomeServers[$this->goFonHomeServer])) {
      return sprintf(_("The previously selected asterisk home server (%s) is no longer available. Remove aborted."),preg_replace("/,/",", ",$this->goFonHomeServer));
    }
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Phone"),
      "plDescription" => _("Phone account settings"),
      "plIcon"        => "plugins/asterisk/images/icon.png",
      "plSmallIcon"   => "plugins/asterisk/images/iconMini.png",
      "plSelfModify"  => TRUE,
      "plPriority"    => 16,                                 // Position in tabs
      "plObjectType"  => array("user"),

      "plProvidedAcls"  => array(
        "telephoneNumber"       => _("Telephone number"),
        "goFonMacro"            => _("Macro settings"),
        "goFonHardware"         => _("Phone hardware"),
        "goFonHomeServer"       => _("Home server"),
        "goFonContext"          => _("Phone context"),
        "goFonVoiceMailContext" => _("Voice mail context"),
        "goFonPIN"              => _("Telephone pin"),
        "goFonVoicemailPIN"     => _("Voicemail pin")
      )
    );
  }

  /* Return asterisk contexts
   * Additionaly read contexts from file.
   */
  function get_asterisk_voicemail_contexts()
  {
    return $this->get_asterisk_contexts('voicemail');
  }

  function get_asterisk_sip_contexts()
  {
    return $this->get_asterisk_contexts('sip');
  }

  function get_asterisk_contexts($prefix)
  {
    $contexts = $this->config->get_cfg_value($prefix.'Contexts', array());
    if (empty($contexts)) {
      msg_dialog::display(_('Warning'), sprintf(_('You need to configure %s contexts first'), $prefix), WARNING_DIALOG);
      $contexts[] = 'default';
    }
    return $contexts;
  }
}
?>
