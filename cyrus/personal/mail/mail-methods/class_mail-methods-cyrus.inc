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

class mailMethodCyrus extends mailMethod
{

  protected $ServerList   = array();
  protected $imap_handle  = NULL;
  protected $quota_loaded = FALSE;

  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail   = FALSE;

  /* Allow modification of the mail server attribute existing mail accounts */
  protected $modifyableServer = FALSE;

  /* Enforces same value for 'mail' as used for 'cn' */
  protected $mailEqualsCN   = FALSE;

  protected $enableDomainSelection  = FALSE;
  protected $enableQuota            = TRUE;
  protected $enableSieveManager     = TRUE;
  protected $enableVacationRange    = TRUE;
  protected $enableFolderTypes      = FALSE;

  protected function init()
  {
    mailMethod::init();
    if (isset($this->config->data['SERVERS']['IMAP'])) {
      $this->ServerList = $this->config->data['SERVERS']['IMAP'];
    }
  }


  public function connect()
  {
    mailMethod::connect();

    if (!count($this->ServerList)) {
      $this->error = _("There are no IMAP compatible mail servers defined!");
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,
          "<b>IMAP: No mail servers configured, check systems->server->service->imap.</b>", "");
      return FALSE;
    } elseif (!isset($this->ServerList[$this->parent->gosaMailServer])) {
      $this->error = _("Mail server for this account is invalid!");
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,
          "<b>IMAP: The selected mail server '".$this->parent->gosaMailServer."' is invalid.</b>", "");
      return FALSE;
    } else {
      $cfg = $this->ServerList[$this->parent->gosaMailServer];
    }

    /* For some reason, hiding errors with @ does not wor here... */
    if (!isset($cfg['connect'])) {
      $cfg['connect']   = "";
    }
    if (!isset($cfg['admin'])) {
      $cfg['admin']     = "";
    }
    if (!isset($cfg['password'])) {
      $cfg['password']  = "";
    }

    /* Setting connect timeout to 10 seconds,
        else the FusionDirectory UI may freeze for 60 seconds.
       (PHP default is 'default_socket_timeout = 60') */
    $timeout = $this->config->get_cfg_value("imapTimeout", 10);
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $timeout,
          "<b>IMAP: Setting imap connect timeout to</b> (seconds)");
    imap_timeout(1, $timeout);

    $this->imap_handle = @imap_open($cfg['connect'], $cfg['admin'], $cfg['password'], OP_HALFOPEN);

    /* Mailbox reachable? */
    if ($this->imap_handle === FALSE) {
      $this->error = imap_last_error();

      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Failed</b> :".imap_last_error(),
        "<b>IMAP:</b> ".$cfg['admin']."@".$cfg['connect']);

      return FALSE;
      $this->connected = FALSE;
    }
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>successful</b>",
        "<b>IMAP:</b> ".$cfg['admin']."@".$cfg['connect']);
    $this->connected = TRUE;

    return TRUE;
  }


  public function account_exists()
  {
    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return array();
    }

    /* Get server config */
    $cfg  = $this->ServerList[$this->parent->gosaMailServer];
    $list = @imap_listmailbox($this->imap_handle, $cfg["connect"], $this->account_id);
    $res  = (is_array($list) && count($list));
    if ($res) {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "", "<b>IMAP: Account exists in imap server.</b>");
    } else {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "", "<b>IMAP: Account seems NOT to exists in imap server.</b>");
    }
    return $res;
  }

  public function disconnect()
  {
    mailMethod::disconnect();
    if ($this->is_connected()) {
      @imap_close ($this->imap_handle);
    }
  }


  public function is_connected()
  {
    $ret = mailMethod::is_connected();
    return ($ret && $this->imap_handle);
  }

  protected function loadQuota()
  {
    if (!$this->quotaEnabled()) {
      return TRUE;
    }
    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return FALSE;
    }

    $this->reset_error();

    /* Load quota settings */
    $result = array("quotaUsage" => "","gosaMailQuota" => "");
    $quota_value = @imap_get_quota($this->imap_handle, $this->account_id);

    /* Reset error queue, imap_qet_quota() will fail if the quota wasn't set yet.
     */
    imap_errors();

    if (is_array($quota_value) && count($quota_value)) {
      if (isset($quota_value["STORAGE"]) && is_array($quota_value["STORAGE"])) {

        /* use for PHP >= 4.3 */
        if ($quota_value["STORAGE"]['limit'] == 2147483647) {
          $result['quotaUsage']     = (int) ($quota_value["STORAGE"]['usage'] / 1024);
          $result['gosaMailQuota']  = "";
        } else {
          $result['quotaUsage']     = (int) ($quota_value["STORAGE"]['usage'] / 1024);
          $result['gosaMailQuota']  = (int) ($quota_value["STORAGE"]['limit'] / 1024);
        }
      } else {

        /* backward icompatible */
        if ($quota_value['usage'] == 2147483647) {
          $result['quotaUsage']     = (int) ($quota_value['usage'] / 1024);
          $result['gosaMailQuota']  = "";
        } else {
          $result['quotaUsage']     = (int) ($quota_value['usage'] / 1024);
          $result['gosaMailQuota']  = (int) ($quota_value['limit'] / 1024);
        }
      }
    }
    $this->quotaValue = $result['gosaMailQuota'];
    $this->quotaUsage = $result['quotaUsage'];

    /* Write debug output */
    if (is_array($quota_value)) {
      if ($this->quotaValue == "") {
        $quota = "(".$this->quotaUsage." / unlimited)";
      } else {
        $quota = "(".$this->quotaUsage." / ".$this->quotaValue.")";
      }
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $quota,
          "<b>IMAP: Successfully received account quota</b>");
    } else {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, imap_last_error(),
          "<b>IMAP: Failed to receive account quota</b>");
    }
  }

  public function getQuota($quota)
  {
    mailMethod::getQuota($quota);
    if (!$this->quota_loaded) {
      $this->quota_loaded = TRUE;
      $this->loadQuota();
    }
    return $this->quotaValue;
  }

  public function getQuotaUsage()
  {
    mailMethod::getQuotaUsage();
    if (!$this->quota_loaded) {
      $this->quota_loaded = TRUE;
      $this->loadQuota();
    }
    return $this->quotaUsage;
  }

  public function setQuota($number)
  {
    mailMethod::setQuota($number);

    if (!$this->quotaEnabled()) {
      return TRUE;
    }
    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return FALSE;
    }

    $this->build_account_id();

    /* Workaround for the php imap extension */
    if (($this->quotaValue == "") || ($this->quotaValue == "2147483647")) {
      $this->quotaValue = "2147483647";
    } elseif ($this->quotaValue > 0) {
      $this->quotaValue = $this->quotaValue * 1024;
    }
    $debug_number = $this->quotaValue." KB";
    if ($this->quotaValue == "2147483647") {
      $debug_number .= "<i>Unlimited</i>";
    }

    if (!imap_set_quota($this->imap_handle, $this->account_id, $this->quotaValue)) {
      msg_dialog::display(_("IMAP error"), sprintf(_("Cannot modify IMAP mailbox quota: %s"),
            '<br><br><i>'.imap_last_error().'</i>'), ERROR_DIALOG);
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id.": (".$debug_number.")</b>",
          "<b>IMAP: Set account quota</b> on server '".$this->parent->gosaMailServer."' <b>".imap_last_error()."</b>");
      return FALSE;
    }
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id.": (".$debug_number.")</b>",
        "<b>IMAP: Set account quota</b> on server :".$this->parent->gosaMailServer);
    return TRUE;
  }


  public function updateMailbox()
  {
    mailMethod::updateMailbox();

    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return FALSE;
    }

    $this->build_account_id ();
    if ($this->is_connected()) {
      $cfg  = $this->ServerList[$this->parent->gosaMailServer];
      $list = imap_listmailbox($this->imap_handle, $cfg["connect"], $this->account_id);
      if ($list === FALSE) {
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id."</b>",
          "<b>IMAP: Add/Update account</b> on server :".$this->parent->gosaMailServer);
        if (!imap_createmailbox($this->imap_handle, $cfg["connect"].$this->account_id)) {
          $this->error = imap_last_error();
          return FALSE;
        }

        /* Autocreate configured default folders */
        $folders = $this->config->get_cfg_value("cyrusAutocreateFolders", NULL);
        if ($folders !== NULL) {
          $cyrus_delim      = ($this->cyrusUseSlashes?"/":".");

          // Walk thru list of specified folders
          foreach ($folders as $folder) {
            @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id."</b>",
              "<b>IMAP: Add/Update account folder ".$folder."</b> on server :".$this->parent->gosaMailServer);
            if (!imap_createmailbox($this->imap_handle, $cfg["connect"].$this->account_id.$cyrus_delim.$folder)) {
              $this->error = imap_last_error();
              return FALSE;
            }
          }

        }
      }
    }
    return TRUE;
  }


  public function deleteMailbox()
  {
    mailMethod::deleteMailbox();

    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return FALSE;
    }

    $this->build_account_id ();

    $cfg = $this->ServerList[$this->parent->gosaMailServer];
    @imap_setacl ($this->imap_handle, $this->account_id, $cfg["admin"], "lrswipcda");

    if ($this->config->get_cfg_value("cyrusDeleteMailbox", "true") == "true") {
      if (!imap_deletemailbox($this->imap_handle, $cfg["connect"].$this->account_id)) {
        $this->error = imap_last_error();
        return FALSE;
      }
    } else {
      msg_dialog::display(_("Mail info"),
        sprintf(_("LDAP entry has been removed but cyrus mailbox (%s) is kept.\nPlease delete it manually!"),
        $this->account_id), INFO_DIALOG);
    }
    return TRUE;
  }


  public function getMailboxList()
  {
    mailMethod::getMailboxList();

    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return array();
    }

    $result = array();

    /* Get server config */
    $cfg = $this->ServerList[$this->parent->gosaMailServer];

    /* Create search string
       And prepare replacements
     */
    if (preg_match("/\@/", $this->account_id)) {
      if ($this->cyrusUseSlashes) {
        $search = preg_replace("/\@/", "/*@", $this->account_id);
      } else {
        $search = preg_replace("/\@/", ".*@", $this->account_id);
      }
      $with_domain = TRUE;
    } else {
      if ($this->cyrusUseSlashes) {
        $search = $this->account_id."/*";
      } else {
        $search = $this->account_id.".*";
      }
      $with_domain = FALSE;
    }
    $folder = $this->account_id;
    if (preg_match("/\@/", $folder)) {
      $folder = preg_replace("/\@.*$/", "", $folder);
    }

    /* Contact imap server */
    $list   = @imap_listmailbox($this->imap_handle, $cfg["connect"], $this->account_id);
    $list2  = @imap_listmailbox($this->imap_handle, $cfg["connect"], $search);

    /* Create list of returned folder names */
    if (is_array($list)) {

      /* Merge in subfolders */
      if (is_array($list2)) {
        $list = array_merge($list, $list2);
      }

      foreach ($list as $val) {
        $str = trim(preg_replace("/^\{[^\}]*+\}/", "", $val));
        if ($with_domain) {
          $str = trim(preg_replace("/\@.*$/", "", $str));
        }
        $str = preg_replace ("/^.*".preg_quote($folder, '/')."/", "INBOX",
          mb_convert_encoding($str, "UTF-8", "UTF7-IMAP"));
        $result[] = $str;
      }
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim(implode($result, ", "), ", "),
          "<b>IMAP: Received mailbox folders.</b>");
      $this->error = imap_last_error();
    } else {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, imap_last_error(),
          "<b>IMAP: Cannot receive mailbox folders.</b>");
      $this->error = imap_last_error();
      return array();
    }

    /* Append "INBOX" to the folder array if result is empty and request comes from user dialog */
    if (!count($result)) {
      $result[] = "INBOX";
    }

    return $result;
  }


  /*! \brief  Returns configured acls
   */
  public function  getFolderACLs($folder_acls)
  {
    $this->reset_error();

    /* imap_getacl available? */
    if (!function_exists('imap_getacl')) {
      $this->error = _("The module imap_getacl is not implemented!");
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "The imap_getacl module is missing!",
          "<b>IMAP: Cannot set folder acls.</b>");
      return $folder_acls;
    }

    /* Get ACLs and merge them with the already given acls (ldap)
     */
    $this->build_account_id();
    $acls = imap_getacl ($this->imap_handle, $this->account_id);
    foreach ($acls as $user => $acl) {
      if ($user == "anyone") {
        $user = "__anyone__"; // Map to internal placeholder
      }
      $folder_acls[$user] = $acl;
    }

    /* Merge given ACL with acl mapping
       This ensures that no ACL will accidentally overwritten by fusiondirectory.
     */
    foreach ($folder_acls as $user => $acl) {
      if (!isset($this->acl_mapping[$acl])) {
        $this->acl_mapping[$acl] = $acl;
      }
    }

    return $folder_acls;
  }



  /*! \brief  Write ACLs back to imap or what ever
   */
  public function  setFolderACLs($permissions)
  {
    $this->reset_error();

    /* imap_getacl available? */
    if (!function_exists('imap_getacl')) {
      $this->error = _("The module imap_getacl is not implemented!");
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "The imap_getacl module is missing!",
          "<b>IMAP: Cannot set folder acls.</b>");
      return FALSE;
    }

    /* Get list of subfolders */
    $folders = $this->getMailboxList();
    foreach ($folders as $subfolder) {
      $folder_id = $this->create_folder_id($subfolder);

      /* Remove all acl's for this folder */
      $users = @imap_getacl ($this->imap_handle, $folder_id);

      if (is_array($users)) {
        foreach ($users as $userid => $perms) {
          $userid = strtolower($userid);
          imap_setacl ($this->imap_handle, $folder_id, $userid, "");
          @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $folder_id." -> ".$userid,
              "<b>IMAP: Removing folder permissions.</b>");
        }
      }
    }

    /* Set permissions for this folder */
    foreach ($folders as $subfolder) {
      $folder_id = $this->create_folder_id($subfolder);

      foreach ($permissions as $user => $acl) {
        imap_setacl ($this->imap_handle, $folder_id, $user, $acl);
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $folder_id." -> ".$user.": ".$acl,
            "<b>IMAP: Setting new folder permissions.</b>");
      }
    }
    return(TRUE);
  }


  public function saveSieveSettings()
  {
    mailMethod::saveSieveSettings();

    /* Map attribute from parent class
     */
    $mail                     = $this->parent->mail;
    $gosaMailDeliveryMode     = $this->parent->gosaMailDeliveryMode;
    $gosaMailAlternateAddress = $this->parent->gosaMailAlternateAddress;
    $gosaMailMaxSize          = $this->parent->gosaMailMaxSize;
    $gosaSpamMailbox          = $this->parent->gosaSpamMailbox;
    $gosaSpamSortLevel        = $this->parent->gosaSpamSortLevel;
    $gosaVacationMessage      = $this->parent->gosaVacationMessage;

    /* Try to login into sieve
     */
    $cfg    = $this->ServerList[$this->parent->gosaMailServer];
    $sieve  = new sieve($cfg["sieve_server"], $cfg["sieve_port"], $this->getUAttribValue(),
                        $cfg["password"], $cfg["admin"], $cfg["sieve_option"]);
    if (!$sieve->sieve_login()) {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $sieve->error_raw, "<b>SIEVE: login failed.</b>");
      $this->error = $sieve->error_raw;
      return FALSE;
    }

    /* Build spamlevel. Spamassassin tags mails with "*" for each integer
       point of spam. So a spam level of 5.3 gets "*****" which can be
       checked easily by spam filters */
    $spamlevel = str_pad("", (int) $gosaSpamSortLevel, "*");

    /* Get current sieve script named 'fusiondirectory'.
        Check if it valid ("###FUSIONDIRECTORY" must be the first string).
        If it is valid just replace it, if it is NOT valid
         create a backup of the old
     */
    if ($sieve->sieve_listscripts()) {
      if (in_array('fusiondirectory', $sieve->response)) {
        $script = "";
        if (!$sieve->sieve_getscript('fusiondirectory')) {
          $this->error = sprintf(_('Cannot retrieve SIEVE script: %s'), to_string($sieve->error_raw));
          @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $sieve->error_raw,
              "<b>SIEVE: Connot read 'fusiondirectory' sieve script.</b>");
          $this->error = $sieve->error_raw;
          return FALSE;
        }

        $is_valid_script = FALSE;
        foreach ($sieve->response as $line) {
          if (empty($line)) {
            continue;
          }
          if (preg_match ('/^###FUSIONDIRECTORY/', $line) && strlen($script) == 0) {
            $is_valid_script = TRUE;
          }
          $line   = rtrim($line);
          $script .= $line;
        }

        if ($is_valid_script || strlen($script) == 0 || empty($script)) {
          @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "",
              "<b>SIEVE</b>: Sieve script 'fusiondirectory' was a valid FusionDirectory script and will be replaced.");
        } else {
          $new_name = "non_fusiondirectory_".date("Ymd_H-i-s");
          $sieve->sieve_sendscript($new_name, $script);
          @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->sieve->error_raw,
              "<b>SIEVE</b>: Non FusionDirectory sieve script. <b>Creating backup of the current sieve script '".$new_name."'.</b>");
        }
      }
    }


    /*****
      Build up new sieve script here.
     *****/

    /* Only create a new one, if it is not empty */
    $smarty = get_smarty();
    if (is_integer(strpos($gosaMailDeliveryMode, "R")) ||
        is_integer(strpos($gosaMailDeliveryMode, "C")) ||
        is_integer(strpos($gosaMailDeliveryMode, "I")) ||
        is_integer(strpos($gosaMailDeliveryMode, "V")) ||
        is_integer(strpos($gosaMailDeliveryMode, "S"))) {
      /* Add anti-spam code */
      $smarty->assign('spamfilter', is_integer(strpos($gosaMailDeliveryMode, "S")));
      $smarty->assign('spambox', $gosaSpamMailbox);
      $smarty->assign('spamlevel', $spamlevel);

      /* Add "reject due to mailsize" code, message is currently not
         adjustable through FusionDirectory. */
      $smarty->assign('sizefilter', is_integer(strpos($gosaMailDeliveryMode, "R")));
      $smarty->assign('maxsize', $gosaMailMaxSize);

      /* Add vacation information */
      $smarty->assign('vacation', is_integer(strpos($gosaMailDeliveryMode, "V")));
      if (is_integer(strpos($gosaMailDeliveryMode, "V"))) {
        /* Sieve wants all destination addresses for the
           vacation message, so we've to assemble them from
           mail and mailAlternateAddress */
        $addrlist = '"'.$mail.'"';
        foreach ($gosaMailAlternateAddress as $val) {
          $addrlist .= ", \"$val\"";
        }
        $smarty->assign('addrlist', $addrlist);
        $smarty->assign('vacmsg', addslashes(addslashes($gosaVacationMessage)));
      }

      /* If no local delivery is wanted, tell the script to discard the mail */
      $smarty->assign('dropownmail', is_integer(strpos($gosaMailDeliveryMode, "I")));

      $script = $smarty->fetch(get_template_path('sieve_script.tpl', TRUE, dirname(__FILE__)));
    } else {
      $script = '';
    }
    /****
      Sieve script build complete
     ****/

    /* Upload script and make it the default one */
    if (!$sieve->sieve_sendscript("fusiondirectory", $script)) {
      $this->error = sprintf(_("Cannot store SIEVE script: %s"), to_string($sieve->error_raw));
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "Error was: ".to_string($sieve->error_raw),
        "<b>SIEVE: Writing new Sieve script failed!</b>");
      return FALSE;
    }

    if (!$sieve->sieve_setactivescript("fusiondirectory")) {
      $this->error = sprintf(_("Cannot activate SIEVE script: %s"), to_string($sieve->error_raw));
      return FALSE;
    }

    $sieve->sieve_logout();
  }

  static public function get_server_list($config)
  {
    $serverList = array();

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search ("(objectClass=fdCyrusServer)",
                  array('cn', 'fdCyrusConnect', 'fdCyrusAdmin', 'fdCyrusPassword', 'fdCyrusSieveServer'));
    while ($attrs = $ldap->fetch()) {

      /* Check if the given fdCyrusSieveServer is in the new style "{cn:port/option}"
         or the old style just "cn".
       */
      if (preg_match("/\{/", $attrs['fdCyrusSieveServer'][0])) {
        sscanf($attrs['fdCyrusSieveServer'][0], '{%[^{}:]:%d%[^{}]}', $sieve_server, $sieve_port, $sieve_option);
      } else {
        $sieve_server = $attrs['fdCyrusSieveServer'][0];
        $sieve_option = "";
        $sieve_port   = "";
      }

      $serverList[$attrs['cn'][0]] = array(
        "server_dn"     => $attrs['dn'],
        "connect"       => $attrs['fdCyrusConnect'][0],
        "admin"         => $attrs['fdCyrusAdmin'][0],
        "password"      => $attrs['fdCyrusPassword'][0],
        "sieve_server"  => $sieve_server,
        "sieve_option"  => $sieve_option,
        "sieve_port"    => $sieve_port
      );
    }

    return $serverList;
  }
}

?>
