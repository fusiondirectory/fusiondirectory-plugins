<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2017  FusionDirectory

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
  protected $imap_handle  = NULL;
  protected $quota_loaded = FALSE;

  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail   = FALSE;

  /* Allow modification of the mail server attribute existing mail accounts */
  protected $modifyableServer = FALSE;

  protected $enableQuota            = TRUE;
  protected $enableVacationRange    = FALSE;
  protected $enableGroupACLs        = TRUE;

  protected function init ()
  {
    $cfg = $this->getServerConfig();

    if (isset($cfg['useSlashes']) && ($cfg['useSlashes'] === 'TRUE')) {
      $this->useSlashesInId = TRUE;
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, 'Enabled', '<b>MAIL:</b> useSlashesInId');
    } else {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, 'Disabled', '<b>MAIL:</b> useSlashesInId');
    }

    parent::init();
  }

  public function connect (): bool
  {
    global $config;
    parent::connect();
    $this->connected = FALSE;

    $cfg = $this->getServerConfig();
    if ($cfg === FALSE) {
      return FALSE;
    }

    /* Setting connect timeout to 10 seconds,
        else the FusionDirectory UI may freeze for 60 seconds.
       (PHP default is 'default_socket_timeout = 60') */
    $timeout = $config->get_cfg_value('imapTimeout', 10);
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $timeout,
          '<b>IMAP: Setting imap connect timeout to</b> (seconds)');
    imap_timeout(1, $timeout);

    $this->imap_handle = @imap_open($cfg['connect'], $cfg['admin'], $cfg['password'], OP_HALFOPEN);

    /* Mailbox reachable? */
    if ($this->imap_handle === FALSE) {
      $this->error = imap_last_error();

      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Failed</b> :".imap_last_error(),
        "<b>IMAP:</b> ".$cfg['admin']."@".$cfg['connect']);

      return FALSE;
    }
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>successful</b>",
        "<b>IMAP:</b> ".$cfg['admin']."@".$cfg['connect']);
    $this->connected = TRUE;

    return TRUE;
  }

  public function account_exists ()
  {
    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error('Method not connected, catch error.');
      return [];
    }

    /* Get server config */
    $cfg  = $this->getServerConfig();
    if ($cfg === FALSE) {
      return FALSE;
    }
    $list = @imap_listmailbox($this->imap_handle, $cfg['connect'], $this->account_id);
    $res  = (is_array($list) && count($list));
    if ($res) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->account_id, '<b>IMAP: Account exists in imap server.</b>');
    } else {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->account_id, '<b>IMAP: Account seems NOT to exists in imap server.</b>');
    }
    return $res;
  }

  public function disconnect ()
  {
    parent::disconnect();
    if ($this->is_connected()) {
      @imap_close($this->imap_handle);
    }
  }

  public function is_connected ()
  {
    $ret = parent::is_connected();
    return ($ret && $this->imap_handle);
  }

  protected function loadQuota ()
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
    $result = ["quotaUsage" => "","gosaMailQuota" => ""];
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
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $quota,
          "<b>IMAP: Successfully received account quota</b>");
    } else {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, imap_last_error(),
          "<b>IMAP: Failed to receive account quota</b>");
    }
  }

  public function getQuota ($quota)
  {
    parent::getQuota($quota);
    if (!$this->quota_loaded) {
      $this->quota_loaded = TRUE;
      $this->loadQuota();
    }
    return $this->quotaValue;
  }

  public function getQuotaUsage ()
  {
    parent::getQuotaUsage();
    if (!$this->quota_loaded) {
      $this->quota_loaded = TRUE;
      $this->loadQuota();
    }
    return $this->quotaUsage;
  }

  public function setQuota ($number)
  {
    parent::setQuota($number);

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
      $error = new FusionDirectoryError(
        htmlescape(sprintf(
          _('Cannot modify IMAP mailbox quota: %s'),
          imap_last_error()
        ))
      );
      $error->display();
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id.": (".$debug_number.")</b>",
          "<b>IMAP: Set account quota</b> on server '".$this->mailServer."' <b>".imap_last_error()."</b>");
      return FALSE;
    }
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id.": (".$debug_number.")</b>",
        "<b>IMAP: Set account quota</b> on server :".$this->mailServer);
    return TRUE;
  }

  public function updateMailbox ()
  {
    global $config;
    parent::updateMailbox();

    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return FALSE;
    }

    $this->build_account_id();

    if ($this->is_connected()) {
      $cfg  = $this->getServerConfig();
      if ($cfg === FALSE) {
        return FALSE;
      }
      $list = imap_listmailbox($this->imap_handle, $cfg['connect'], $this->account_id);
      if ($list === FALSE) {
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->account_id,
          'IMAP: Add account on server '.$this->mailServer);
        if (!imap_createmailbox($this->imap_handle, $cfg['connect'].$this->account_id)) {
          $this->error = imap_last_error();
          return FALSE;
        }

        /* Autocreate configured default folders */
        foreach ($cfg['autocreate'] as $folder) {
          @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $folder,
            'IMAP: Add account folder on server '.$this->mailServer);
          if (!imap_createmailbox($this->imap_handle, $cfg['connect'].$this->create_folder_id($folder))) {
            $this->error = imap_last_error();
            return FALSE;
          }
        }
      }
    }
    return TRUE;
  }

  public function deleteMailbox ()
  {
    global $config;
    parent::deleteMailbox();

    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return FALSE;
    }

    $cfg = $this->getServerConfig();
    if ($cfg === FALSE) {
      return FALSE;
    }

    $this->build_account_id();

    @imap_setacl($this->imap_handle, $this->account_id, $cfg["admin"], "lrswipcda");

    if ($config->get_cfg_value("cyrusDeleteMailbox", "TRUE") == "TRUE") {
      if (!imap_deletemailbox($this->imap_handle, $cfg["connect"].$this->account_id)) {
        $this->error = imap_last_error();
        return FALSE;
      }
    } else {
      msg_dialog::display(
        _('Mail info'),
        nl2br(htmlescape(sprintf(
          _("LDAP entry has been removed but cyrus mailbox (%s) is kept.\nPlease delete it manually!"),
          $this->account_id
        ))),
        INFO_DIALOG
      );
    }
    return TRUE;
  }

  public function getMailboxList ()
  {
    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return [];
    }

    $result = [];

    $cfg = $this->getServerConfig();
    if ($cfg === FALSE) {
      return FALSE;
    }

    /* Create search string
       And prepare replacements
     */
    if (preg_match("/\@/", $this->account_id)) {
      if ($this->useSlashesInId) {
        $search = preg_replace("/\@/", "/*@", $this->account_id);
      } else {
        $search = preg_replace("/\@/", ".*@", $this->account_id);
      }
      $with_domain = TRUE;
    } else {
      if ($this->useSlashesInId) {
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
        $str = preg_replace("/^.*".preg_quote($folder, '/')."/", "INBOX",
          mb_convert_encoding($str, "UTF-8", "UTF7-IMAP"));
        $result[] = $str;
      }
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim(implode(', ', $result)),
          "<b>IMAP: Received mailbox folders.</b>");
      $this->error = imap_last_error();
    } else {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, imap_last_error(),
          "<b>IMAP: Cannot receive mailbox folders.</b>");
      $this->error = imap_last_error();
      return [];
    }

    /* Append "INBOX" to the folder array if result is empty and request comes from user dialog */
    if (!count($result)) {
      $result[] = "INBOX";
    }

    return $result;
  }

  /*! \brief  Returns configured acls
   */
  public function getFolderACLs (): array
  {
    $this->reset_error();

    if (!$this->is_connected() || !$this->imap_handle) {
      trigger_error("Method not connected, catch error.");
      return [];
    }

    /* imap_getacl available? */
    if (!function_exists('imap_getacl')) {
      $this->error = _('The module imap_getacl is not implemented!');
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, 'The imap_getacl module is missing!',
          '<b>IMAP: Cannot set folder acls</b>');
      return [];
    }

    /* Get ACLs */
    $this->build_account_id();
    $acls = imap_getacl($this->imap_handle, $this->account_id);

    foreach ($acls as &$acl) {
      $acl = $this->expandAndSortACL($acl);
      if (!isset($this->acl_mapping[$acl])) {
        /* Merge given ACL with acl mapping
           This ensures that no ACL will accidentally be overwritten
         */
        $this->acl_mapping[$acl] = $acl;
      }
    }
    unset($acl);

    if (!isset($acls['anyone'])) {
      $acls['anyone'] = '';
    }

    return $acls;
  }

  /*! \brief Put ACLs rights always in the same order and expand legacy rights
   */
  protected function expandAndSortACL (string $acl): string
  {
    $letters = 'lrswipkxtea';
    if (strpos($acl, 'c') !== FALSE) {
      /* Expand c to kx */
      $acl .= 'kx';
    }
    if (strpos($acl, 'd') !== FALSE) {
      /* Expand d to xte */
      $acl .= 'xte';
    }
    $res = '';
    foreach (str_split($letters) as $letter) {
      if (strpos($acl, $letter) !== FALSE) {
        $res .= $letter;
      }
    }

    return $res;
  }

  /*! \brief  Write ACLs back to imap or what ever
   */
  public function setFolderACLs (array $acls): bool
  {
    $this->reset_error();

    /* imap_getacl available? */
    if (!function_exists('imap_getacl')) {
      $this->error = _("The module imap_getacl is not implemented!");
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "The imap_getacl module is missing!",
          "<b>IMAP: Cannot set folder acls.</b>");
      return FALSE;
    }

    $this->build_account_id();

    /* Get list of subfolders */
    $folders = $this->getMailboxList();
    foreach ($folders as $subfolder) {
      $folder_id = $this->create_folder_id($subfolder);

      /* Remove all acl's for this folder */
      $users = @imap_getacl($this->imap_handle, $folder_id);

      if (is_array($users)) {
        foreach ($users as $userid => $perms) {
          $userid = strtolower($userid);
          imap_setacl($this->imap_handle, $folder_id, $userid, "");
          @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $folder_id." -> ".$userid,
              "<b>IMAP: Removing folder permissions.</b>");
        }
      }
    }

    /* Set permissions for this folder */
    foreach ($folders as $subfolder) {
      $folder_id = $this->create_folder_id($subfolder);

      foreach ($acls as $user => $acl) {
        imap_setacl($this->imap_handle, $folder_id, $user, $acl);
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $folder_id." -> ".$user.": ".$acl,
            "<b>IMAP: Setting new folder permissions.</b>");
      }
    }
    return TRUE;
  }

  public function saveSieveSettings ()
  {
    // Sieve functionality moved to separate sieve plugin
    return parent::saveSieveSettings();
  }

  static public function get_server_list ()
  {
    global $config;
    $serverList = [];

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=fdCyrusServer)',
                  [
                    'cn','fdCyrusConnect','fdCyrusAdmin','fdCyrusPassword',
                    'fdCyrusUseSlashes','fdCyrusAutocreateFolders'
                  ]);
    while ($attrs = $ldap->fetch()) {
      $serverList[$attrs['cn'][0]] = [
        'server_dn'     => $attrs['dn'],
        'connect'       => $attrs['fdCyrusConnect'][0],
        'admin'         => $attrs['fdCyrusAdmin'][0],
        'password'      => $attrs['fdCyrusPassword'][0],
        'useSlashes'    => ($attrs['fdCyrusUseSlashes'][0] ?? 'FALSE'),
        'autocreate'    => ($attrs['fdCyrusAutocreateFolders'] ?? [])
      ];
    }

    return $serverList;
  }
}
