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

class mailMethod
{
  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail   = TRUE;

  /* Allow modification of the mail server attribute existing mail accounts */
  protected $modifyableServer = TRUE;

  /* the attribute used to create accounts */
  /* Naming attribute for accounts, e.g. imap. */
  protected $uattrib        = 'mail';

  /* The account prefixes, keep the '.' here! See FAQ cyrusUseSlashes */
  protected $user_prefix    = 'user.';
  protected $share_prefix   = 'share.';

  /* Account ID creation

      !!Depends on the attributes 'user_prefix'/'share_prefix' and the option 'cyrusUseSlashes'

      Examples - based on defaults :
        %PREFIX%    => "user." or "user/"           (Depending on cyrusUseSlashes=FALSE/TRUE)
        %CN%        => "technical"                  (The groups cn)
        %UID%       => "john"                       (The users uid)
        %MAIL%      => "john@fusiondirectory.org"   (The mail address)
        %DOMAIN%    => "fusiondirectory.org"        (The domain part of the specified mail)
        %MAILPART%  => "john"                       (The mail address without domain)
        %UATTRIB%   => "john"/"john@fusiondirectory.org"
                                            (Configured in fusiondirectory.conf mailAttribute="mail"/"uid")
   */
  protected $user_id    = "%PREFIX%%UATTRIB%";
  protected $share_id   = "%PREFIX%%UATTRIB%";

  /* Create accounts in cyrus style with '/' instead of '.' */
  protected $cyrusUseSlashes = FALSE;

  /* gosaSharedFolderTarget settings,
   *  E.g.
   *    For an accountID like: 'share/john.folder@fusiondirectory.org' the value 'dummy+'
   *    will result in gosaSharedFolderTarget: dummy+share/john.folder@fusiondirectory.org
   */
  protected $mailSharedPrefix = '';

  /* The attribute mapping for this class  Source --> Destination */
  protected $attributes         = array();
  protected $userObjectClasses  = array();
  protected $shareObjectClasses = array();

  protected $enableQuota            = TRUE;
  protected $enableVacation         = TRUE;
  protected $enableVacationRange    = FALSE;
  protected $enableFolderTypes      = FALSE;

  /* Default values */
  protected $quotaValue   = 0;
  protected $quotaUsage   = 0;

  /* Method internal */
  protected $type               = 'user';
  protected $account_id         = '';
  protected $initial_account_id = '';
  protected $connected          = FALSE;
  protected $error              = '';
  protected $parent             = NULL;

  protected $default_acls = array('__anyone__' => 'p', '__member__' => 'lrswp');

  protected $acl_map = array(
    'lrsw'      => 'read',
    'lrswp'     => 'post',
    'p'         => 'external post',
    'lrswip'    => 'append',
    'lrswipd'   => 'delete',
    'lrswipcd'  => 'write',
    'lrswipcda' => 'admin',
    ' '         => 'none'
  );

  protected $acl_mapping = array();

  /*! \brief  Constructs the mail class
      @param  Object  Plugin  The initator
      @param  String          Open "user" or "group" account.
   */
  function __construct($parent, $type = 'user')
  {
    $this->parent = $parent;

    if (!isset($this->parent->gosaMailServer)) {
      trigger_error('mailMethod with invalid parent object initialized.');
    }

    if (!in_array($this->type, array('user','group'))) {
      trigger_error('Unknown mail class type used "'.$type.'".');
    } else {
      $this->type = $type;
    }

  }

  /*! \brief  Intialize attributes and config settings.
   */
  protected function init()
  {
    global $config;
    /* Get config value for cyrusUseSlashes */
    if ($config->get_cfg_value('cyrusUseSlashes') == 'TRUE') {
      $this->cyrusUseSlashes = TRUE;
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, 'Enabled', '<b>MAIL:</b> cyrusUseSlashes');
    } else {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, 'Disabled', '<b>MAIL:</b> cyrusUseSlashes');
    }

    /* Check if the mail account identification attribute
       is overridden in the configuration file */
    if ($config->get_cfg_value('mailAttribute', '') != '') {
      $new_uattrib = strtolower($config->get_cfg_value('mailAttribute'));
      if (in_array($new_uattrib, array('mail','uid'))) {
        $this->uattrib = $new_uattrib;
      } else {
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$new_uattrib."</b>",
          "<b>MAIL:</b> Unsupported 'mailAttribute' in FusionDirectory configuration specified");
        msg_dialog::display(_("Configuration error"),
            sprintf(_("The configured mail attribute '%s' is unsupported!"), $new_uattrib), ERROR_DIALOG);
      }
    }

    /* Create ACL map */
    foreach ($this->acl_map as $acl => $name) {
      $this->acl_mapping[$acl] = _($name);
    }

    /* Check if we have an individual user/folder creation syntax */
    $tmp = $config->get_cfg_value('mailUserCreation');
    if (!empty($tmp)) {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '<i>'.$tmp.'</i>',
          '<b>MAIL:</b> User creation set to');
      $this->user_id  = $tmp;
    }
    $tmp = $config->get_cfg_value('mailFolderCreation');
    if (!empty($tmp)) {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '<i>'.$tmp.'</i>',
          '<b>MAIL:</b> Shared folder creation set to');
      $this->share_id = $tmp;
    }
    $tmp = $config->get_cfg_value('mailSharedPrefix');
    if (!empty($tmp)) {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '<i>'.$tmp.'</i>',
          '<b>MAIL:</b> Shared folder prefix set to');
      $this->mailSharedPrefix = $tmp;
    }

    $this->build_account_id();
    $this->initial_account_id = $this->account_id;
  }

  public function fixAttributesOnLoad()
  {
    foreach ($this->attributes as $source => $dest) {
      if (isset($this->parent->attrs[$source])) {
        $this->parent->attrs[$dest] = $this->parent->attrs[$source];
      }
      if (isset($this->parent->$source)) {
        $this->parent->$dest = $this->parent->$source;
      }
      if (isset($this->parent->attrs[$source][0])) {
        $this->parent->saved_attributes[$source] = $this->parent->attrs[$source][0];
      }
    }
  }

  public function fixAttributesOnRemove()
  {
    /* Remove objectClasses */
    if ($this->type == 'user') {
      $this->parent->attrs['objectClass'] =
        array_remove_entries_ics($this->userObjectClasses, $this->parent->attrs['objectClass']);
    } else {
      $this->parent->attrs['objectClass'] =
        array_remove_entries_ics($this->shareObjectClasses, $this->parent->attrs['objectClass']);
      $this->parent->attrs['gosaSharedFolderTarget'] = array();
    }
    foreach ($this->attributes as $source => $dest) {
      $this->attrs[$dest]   = array();
      $this->attrs[$source] = array();
    }
  }

  public function fixAttributesOnStore()
  {
    foreach ($this->attributes as $source => $dest) {
      if (isset($this->parent->attrs[$dest])) {
        $this->parent->attrs[$source] = $this->parent->attrs[$dest ];
      }
      if (isset($this->parent->$dest)) {
        $this->parent->$source = $this->parent->$dest;
      }
    }

    if ($this->type == 'user') {
      $ocs = $this->userObjectClasses;
    } else {
      $ocs = $this->shareObjectClasses;
    }
    foreach ($ocs as $oc) {
      if (!in_array($oc, $this->parent->attrs['objectClass'])) {
        $this->parent->attrs['objectClass'][] = $oc;
      }
    }

    // Add gosaSharedFolderTarget for groups.
    $this->build_account_id();
    if ($this->type == 'group') {
      $this->parent->attrs['gosaSharedFolderTarget'] = $this->mailSharedPrefix.$this->account_id;
    }
  }

  /*! \brief  Connect services like imap.
              Not necessary for the base class.
      @return Boolean True if this method is connected else false.
   */
  public function connect()
  {
    $this->reset_error();
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, get_class($this), '<b>MAIL: Connect method</b>');
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->parent->gosaMailServer, '<b>MAIL: Current server</b>');

    $this->connected = TRUE;
    return TRUE;
  }

  /*! \brief  Returns the connection status of this method.
      @return Boolean True if this method is connected else false.
   */
  public function is_connected()
  {
    return $this->connected;
  }

  /*! \brief  Disconnect this method. Close services like imap connection.
              Not necessary for the base class.
   */
  public function disconnect()
  {
    $this->reset_error();
    if ($this->is_connected()) {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, get_class($this), '<b>MAIL: Disconnect method</b>');
      $this->connected = FALSE;
    }
  }

  /*! \brief  Returns true the current object represents a valid account
              (Some methods may check imap accounts here.)
      @return Boolean TRUE if this is a valid account else FALSE
  */
  public function account_exists()
  {
    $this->reset_error();
    return TRUE;
  }

  /*! \brief  Returns the last error occurred
      @return String  The last error message.
   */
  public function get_error()
  {
    return $this->error;
  }

  public function isModifyableMail()
  {
    return $this->modifyableMail;
  }

  public function isModifyableServer()
  {
    return $this->modifyableServer;
  }

  /*! \brief  Returns TRUE if the action caused an error.
      @return Boolean TRUE on error else FALSE
   */
  public function is_error()
  {
    return ($this->error != '');
  }

  /*! \brief  Resets the error message.
   */
  public function reset_error()
  {
    $this->error = '';
  }

  public function get_account_id()
  {
    $this->build_account_id();
    return $this->account_id;
  }

  /*! \brief  Create a new account id, like 'user/name@domain.com'.
   */
  protected function build_account_id()
  {
    /* Build account identicator */
    if ($this->type == "user") {
      $prefix     = $this->user_prefix;
      $acc_string = $this->user_id;
    } else {
      $prefix     = $this->share_prefix;
      $acc_string = $this->share_id;
    }

    /* Create account prefix and respect "cyrusUseSlashes"
       Do not replace escaped dots for cyrusUseSlashes.
     */
    $uattrib = $this->uattrib;
    if ($this->cyrusUseSlashes) {
      $prefix     = preg_replace('/([^\\\\])\./', "\\1/", $prefix);
      $acc_string = preg_replace('/([^\\\\])\./', "\\1/", $acc_string);
    }
    $prefix     = preg_replace("/\\\\([\.\/])/", "\\1", $prefix);
    $acc_string = preg_replace("/\\\\([\.\/])/", "\\1", $acc_string);

    $domain = $mailpart = "";
    $mail   = $this->parent->mail;
    if (preg_match("/\@/", $mail)) {
      list($mailpart,$domain) = explode("@", $mail);
    }

    /* Create account_id
     */
    $from   = array("/%cn%/i","/%uid%/i","/%prefix%/i","/%uattrib%/i","/%domain%/i","/%mailpart%/i","/%mail%/i");
    $uid    = '';
    $cn     = '';
    $attrib = '';
    if (isset($this->parent->cn)) {
      $cn     = $this->parent->cn;
      $uid    = $cn;
      $attrib = $cn;
    }
    if (isset($this->parent->uid)) {
      $uid = $this->parent->uid;
    }
    if (isset($this->parent->$uattrib)) {
      $attrib = $this->parent->$uattrib;
    }
    $to     = array($cn,$uid,$prefix,$attrib,$domain,$mailpart,$mail);
    $acc_id = trim(strtolower(preg_replace($from, $to, $acc_string)));

    /* Check for not replaced pattern.
     */
    if (preg_match("/%/", $acc_id)) {
      $notr = preg_replace("/^[^%]*/", "", $acc_id);
      if (!empty($notr)) {
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Warning</b>",
            sprintf("<b>MAIL: WARNING unknown pattern in account creation string '%s' near '%s'</b>", $acc_id, $notr));

        /* Remove incomprehensible patterns */
        $acc_id = preg_replace("/%[^%]+%/", "", $acc_id);
      }
    }


    if ($this->account_id != $acc_id) {
      $this->account_id = $acc_id;
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $acc_id, "<b>MAIL:</b> AccountID generated");
    }
  }

  /*! \brief  Creates a valid folder id for a given folder name.
               e.g. $folder_id = "INBOX/test"  &&  $this->account_id = "share/mailbox@fusiondirectory.org"
               will result in "share/mailbox/test@fusiondirectory.org"
              This function is mainly used to read and write folder permissions.
      @return String A valid folder id
   */
  public function create_folder_id($folder)
  {
    if (!empty($folder)) {
      $folder = trim(preg_replace("/^INBOX[\.\/]*/i", "", $folder));
    }
    if (!empty($folder)) {
      $folder = "/".$folder;
    }

    /* Build account identicator */
    if ($this->type == "user") {
      $prefix     = $this->user_prefix;
      $acc_string = $this->user_id;
    } else {
      $prefix     = $this->share_prefix;
      $acc_string = $this->share_id;
    }

    /* Create account prefix and respect "cyrusUseSlashes"
       Do not replace escaped dots for cyrusUseSlashes.
     */
    $uattrib = $this->uattrib;
    if ($this->cyrusUseSlashes) {
      $prefix     = preg_replace('/([^\\\\])\./', "\\1/", $prefix);
      $acc_string = preg_replace('/([^\\\\])\./', "\\1/", $acc_string);
    }
    $prefix     = preg_replace("/\\\\([\.\/])/", "\\1", $prefix);
    $acc_string = preg_replace("/\\\\([\.\/])/", "\\1", $acc_string);

    $domain = $mailpart = "";
    $mail = $this->parent->mail;
    if (preg_match("/\@/", $mail)) {
      list($mailpart,$domain) = explode("@", $mail);
    }

    /* Create account_id */
    $from   = array("/%cn%/i","/%uid%/i","/%prefix%/i","/%uattrib%/i","/%domain%/i","/%mailpart%/i","/%mail%/i");
    $to     = array($this->parent->cn,$this->parent->uid,$prefix,$this->parent->$uattrib, $domain, $mailpart,$mail);
    $acc_id = trim(strtolower(preg_replace($from, $to, $acc_string)));

    /* Check for not replaced pattern */
    if (preg_match("/%/", $acc_id)) {
      $notr = preg_replace("/^[^%]*/", '', $acc_id);
      if (!empty($notr)) {
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '<b>Warning</b>',
            sprintf("<b>MAIL: WARNING unknown pattern in account creation string '%s' near '%s'</b>", $acc_id, $notr));

        /* Remove incomprehensible patterns */
        $acc_id = preg_replace("/%[^%]+%/", '', $acc_id);
      }
    }

    if (preg_match("/\@/", $acc_id)) {
      list($mail,$domain) = explode('@', $acc_id);
      $str = trim($mail . $folder . '@' . $domain);
    } else {
      $str = trim($acc_id . $folder);
    }
    return $str;
  }

  /*! \brief  Returns the configured mail method for the given parent object,
                initialized and read for use.
      @return mailMethod  The configured mailMethod.
   */
  public function get_method()
  {
    global $config;
    $methods = mailMethod::get_methods();
    if ($config->get_cfg_value('mailmethod') != '') {
      $method = $config->get_cfg_value('mailmethod');
      $cls = get_correct_class_name('mailMethod$method');
      if (isset($methods[$cls])) {
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $cls, '<b>MAIL:</b> Selected mailMethod');
        $tmp = new $cls($this->parent, $this->type);
        $tmp->init();
        return $tmp;
      } else {
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '', '<b>MAIL: Invalid mailMethod defined: '.$cls.
            ' falling back to '.get_class($this).'</b>');

        /* Print out configuration errors directly, we can't catch them everywhere */
        msg_dialog::display(_('Configuration error'),
            sprintf(_('Mail method "%s" is unknown!'), $method), ERROR_DIALOG);
      }
    }

    /* If no valued mailMethod could be detected, return the base class */
    $this->init();
    return $this;
  }

  /*! \brief Saves sieve settings
   */
  public function saveSieveSettings()
  {
    $this->reset_error();
    return TRUE;
  }

  /*! \brief  Creates or Updates the mailAccount represented by this class.
   */
  public function updateMailbox()
  {
    $this->reset_error();
    return TRUE;
  }

  /*! \brief  Removes the mailbox represented by this class,
               and update shared folder ACLs .
   */
  public function deleteMailbox()
  {
    $this->reset_error();
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id."</b>",
        "<b>MAIL: Remove account</b> from server :".$this->parent->gosaMailServer);

    return TRUE;
  }

  /*! \brief  Returns the used mail attribute (mail,uid)
      @param  String  One out of 'mail','uid'
   */
  public function getUAttrib()
  {
    return $this->uattrib;
  }

  /*! \brief  Returns the used mail attribute (mail,uid)
      @param  String  One out of 'mail','uid'
   */
  public function getUAttribValue()
  {
    $uattrib = $this->getUAttrib();
    return $this->parent->$uattrib;
  }

  /*! \brief  Returns whether the quota settings are enabled or not
      @return Boolean TRUE if enabled else FALSE
   */
  public function quotaEnabled()
  {
    return $this->enableQuota;
  }

  /*! \brief  Returns the used quota
      @return Integer Quota used.
   */
  public function getQuotaUsage()
  {
    return -1;
  }

  /*! \brief  Returns the quota restrictions.
      @return Integer Quota restrictions.
   */
  public function getQuota($quotaValue)
  {
    return $quotaValue;
  }

  /*! \brief  Sets the mail quota
   */
  public function setQuota($number)
  {
    if (!is_numeric($number)) {
      $number = (int) $number;
      if (!$number) {
        $number = 0;
      }
    }
    $this->quotaValue = $number;
    return TRUE;
  }

  /*! \brief  Returns the list of configured mailbox folders
      @return Array The mailbox folders.
   */
  public function getMailboxList()
  {
    return array("INBOX");
  }

  /*! \brief  Returns whether the vacation is enabled
      @return Boolean TRUE, FALSE
   */
  public function vacationEnabled()
  {
    return $this->enableVacation;
  }

  /*! \brief  Returns whether the vacation range is selectable or not
      @return Boolean TRUE, FALSE
   */
  public function vacationRangeEnabled()
  {
    return ($this->vacationEnabled() && $this->enableVacationRange);
  }

  /*! \brief  Checks dependencies to other FusionDirectory plugins.
   */
  public function accountCreateable(&$reason = "")
  {
    return TRUE;
  }

  /*! \brief  Checks whether this account is removeable or not.
              There may be some dependencies left, eg. kolab.
   */
  public function accountRemoveable(&$reason = "")
  {
    return TRUE;
  }


  /*! \brief  Returns all mail servers configured in FusionDirectory
      @return Array  All useable mail servers
  */
  static public function getMailServers()
  {
    if (session::global_is_set('mailServers')) {
      return session::global_get('mailServers');
    }
    $methods = mailMethod::get_methods();
    $servers = array();
    foreach ($methods as $class) {
      $servers = array_merge($servers, $class::get_server_list());
    }
    session::global_set('mailServers', $servers);
    return $servers;
  }

  static public function resetMailServersCache()
  {
    session::global_un_set('mailServers');
  }

  /*! \brief  Returns the available mailMethods
      @return Array   A list of all avaialable mailMethods_
   */
  static public function get_methods()
  {
    global $class_mapping;
    $available = array('mailMethod' => 'mailMethod');
    foreach ($class_mapping as $class => $path) {
      if (preg_match('/^mailMethod.+/', $class)) {
        $available[$class] = $class;
      }
    }
    return $available;
  }

  static public function getServerMethod($server)
  {
    $methods = mailMethod::get_methods();
    foreach ($methods as $class) {
      if ($class::serverMatch($server)) {
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $class, 'Mail method for server "'.$server.'"');
        return $class;
      }
    }
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '', 'No method found for server "'.$server.'"');
    return 'mailMethod';
  }

  static public function serverMatch($server)
  {
    return isset(static::get_server_list()[$server]);
  }

  static public function getInstance($class, $parent)
  {
    $obj = new $class($parent);
    $obj->init();
    return $obj;
  }

  /*! \brief  Returns configured acls
   */
  public function  getFolderACLs($folder_acls)
  {
    /* Merge given ACL with acl mapping
       This ensures that no ACL will accidentally overwritten by fusiondirectory.
     */
    foreach ($folder_acls as $acl) {
      if (!isset($this->acl_mapping[$acl])) {
        $this->acl_mapping[$acl] = $acl;
      }
    }

    return $folder_acls;
  }

  /*! \brief  Write ACLs back to imap or what ever
   */
  public function  setFolderACLs($array)
  {
    return TRUE;
  }

  /*! \brief  Returns a list of all possible acls.
      @return Array   ACLs.
  */
  public function getAclTypes()
  {
    return $this->acl_mapping;
  }

  /*! \brief  Returns the configured mailMethod
      @return String  the configured mail method or ""
   */
  static public function get_current_method()
  {
    global $class_mapping, $config;
    $method = $config->get_cfg_value("mailmethod");
    $cls    = get_correct_class_name("mailMethod$method");
    foreach ($class_mapping as $class => $path) {
      if ($class == $cls) {
        return $class;
      }
    }
    return "";
  }

  static function quota_to_image($use, $quota)
  {
    if ($use == -1) {
      return "&nbsp;&nbsp;"._("Unknown");
    } elseif (empty($quota)) {
      return "&nbsp;&nbsp;"._("Unlimited");
    } else {
      $usage = round(($use / $quota) * 100);
      return "<img src='progress.php?x=100&amp;y=17&amp;p=$usage'>";
    }
  }

  static public function get_server_list()
  {
    global $config;
    $serverList = array();

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=fdImapServer)', array('cn'));
    while ($attrs = $ldap->fetch()) {
      $serverList[$attrs['cn'][0]] = array(
        'server_dn'     => $attrs['dn'],
        'connect'       => '',
        'admin'         => '',
        'password'      => '',
        'sieve_server'  => '',
        'sieve_option'  => '',
        'sieve_port'    => ''
      );
    }

    return $serverList;
  }

  /*! \brief  Returns the default sharedFolder ACLs for this method.
      @return Array Returns an array containg acls for __member__ and __anyone__
   */
  public function getDefaultACLs()
  {
    $tmp = $this->default_acls;
    if (!isset($tmp['__member__'])) {
      $tmp['__member__'] = " ";
    }
    if (!isset($tmp['__anyone__'])) {
      $tmp['__anyone__'] = " ";
    }
    return $tmp;
  }

  public function additionalInformations()
  {
    return array();
  }
}
?>
