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

  /* The account prefixes, keep the '.' here! See FAQ useSlashesInId */
  protected $user_prefix    = 'user.';
  protected $share_prefix   = 'share.';

  /* Account ID creation

      !!Depends on the attributes 'user_prefix'/'share_prefix' and the option 'useSlashesInId'

      Examples - based on defaults :
        %PREFIX%    => "user." or "user/"           (Depending on useSlashesInId=FALSE/TRUE)
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

  /* Create accounts with '/' instead of '.' */
  protected $useSlashesInId = FALSE;

  /* gosaSharedFolderTarget settings,
   *  E.g.
   *    For an accountID like: 'share/john.folder@fusiondirectory.org' the value 'dummy+'
   *    will result in gosaSharedFolderTarget: dummy+share/john.folder@fusiondirectory.org
   */
  protected $mailSharedPrefix = '';

  /* The attribute mapping for this class  Source --> Destination */
  protected $attributes         = [];
  protected $userObjectClasses  = [];
  protected $shareObjectClasses = [];

  /* Features */
  protected $enableQuota            = TRUE;
  protected $enableVacation         = TRUE;
  protected $enableVacationRange    = FALSE;
  protected $enableGroupForwarding  = TRUE;
  protected $enableLocalOnly        = TRUE;
  protected $enableForwardOnly      = TRUE;
  protected $enableGroupACLs        = FALSE;

  /* Default values */
  protected $quotaValue   = 0;
  protected $quotaUsage   = 0;

  /* Method internal */
  protected $type               = 'user';
  protected $account_id         = '';
  protected $initial_account_id = '';
  protected $connected          = FALSE;
  protected $error              = '';
  /**
   * @var simplePlugin
   */
  protected $parent;
  protected $mailServer         = NULL;

  protected $acl_map = [
    ''            => 'none',
    'p'           => 'external post',
    'lrs'         => 'read',
    'lrsp'        => 'post',
    'lrspxte'     => 'delete',
    'lrsip'       => 'append',
    'lrswipkxte'  => 'write',
    'lrswipkxtea' => 'admin',
  ];

  protected $acl_mapping = [];

  /*! \brief  Constructs the mail class
      @param  simplePlugin  Plugin  The initator
      @param  string        Open "user" or "group" account.
   */
  function __construct ($parent, $type = 'user')
  {
    $this->parent = $parent;

    if (!isset($this->parent->gosaMailServer)) {
      throw new FusionDirectoryException('mailMethod with invalid parent object initialized.');
    }

    $this->mailServer = $this->parent->gosaMailServer;

    if (!in_array($this->type, ['user','group'])) {
      trigger_error('Unknown mail class type used "'.$type.'".');
    } else {
      $this->type = $type;
    }
  }

  /*! \brief  Intialize attributes and config settings.
   */
  protected function init ()
  {
    global $config;

    /* Check if the mail account identification attribute
       is overridden in the configuration file */
    if ($config->get_cfg_value('mailAttribute', '') != '') {
      $new_uattrib = strtolower($config->get_cfg_value('mailAttribute'));
      if (in_array($new_uattrib, ['mail','uid'])) {
        $this->uattrib = $new_uattrib;
      } else {
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$new_uattrib."</b>",
          "<b>MAIL:</b> Unsupported 'mailAttribute' in FusionDirectory configuration specified");
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('The configured mail attribute "%s" is unsupported!'),
            $new_uattrib
          ))
        );
        $error->display();
      }
    }

    /* Create ACL map */
    foreach ($this->acl_map as $acl => $name) {
      $this->acl_mapping[$acl] = sprintf(_('%s (%s)'), _($name), $acl);
    }

    /* Check if we have an individual user/folder creation syntax */
    $tmp = $config->get_cfg_value('mailUserCreation');
    if (!empty($tmp)) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '<i>'.$tmp.'</i>',
          '<b>MAIL:</b> User creation set to');
      $this->user_id  = $tmp;
    }
    $tmp = $config->get_cfg_value('mailFolderCreation');
    if (!empty($tmp)) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '<i>'.$tmp.'</i>',
          '<b>MAIL:</b> Shared folder creation set to');
      $this->share_id = $tmp;
    }
    $tmp = $config->get_cfg_value('mailSharedPrefix');
    if (!empty($tmp)) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '<i>'.$tmp.'</i>',
          '<b>MAIL:</b> Shared folder prefix set to');
      $this->mailSharedPrefix = $tmp;
    }

    $this->build_account_id();
    $this->initial_account_id = $this->account_id;
  }

  public function fixAttributesOnLoad ()
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

  public function fixAttributesOnRemove ()
  {
    /* Remove objectClasses */
    if ($this->type == 'user') {
      $this->parent->attrs['objectClass'] =
        array_remove_entries_ics($this->userObjectClasses, $this->parent->attrs['objectClass']);
    } else {
      $this->parent->attrs['objectClass'] =
        array_remove_entries_ics($this->shareObjectClasses, $this->parent->attrs['objectClass']);
      $this->parent->attrs['gosaSharedFolderTarget'] = [];
    }
    foreach ($this->attributes as $source => $dest) {
      $this->parent->attrs[$dest]   = [];
      $this->parent->attrs[$source] = [];
    }
  }

  public function fixAttributesOnStore ()
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
  public function connect (): bool
  {
    $this->reset_error();
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, get_class($this), '<b>MAIL: Connect method</b>');
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->mailServer, '<b>MAIL: Current server</b>');

    $this->connected = TRUE;
    return TRUE;
  }

  /*! \brief  Returns the connection status of this method.
      @return Boolean True if this method is connected else false.
   */
  public function is_connected ()
  {
    return $this->connected;
  }

  /*! \brief  Disconnect this method. Close services like imap connection.
              Not necessary for the base class.
   */
  public function disconnect ()
  {
    $this->reset_error();
    if ($this->is_connected()) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, get_class($this), '<b>MAIL: Disconnect method</b>');
      $this->connected = FALSE;
    }
  }

  /*! \brief Checks data consistency
  */
  public function check ()
  {
    return [];
  }

  /*! \brief  Returns true the current object represents a valid account
              (Some methods may check imap accounts here.)
      @return Boolean TRUE if this is a valid account else FALSE
  */
  public function account_exists ()
  {
    $this->reset_error();
    return TRUE;
  }

  /*! \brief  Returns the last error occurred
      @return String  The last error message.
   */
  public function get_error ()
  {
    return $this->error;
  }

  public function isModifyableMail ()
  {
    return $this->modifyableMail;
  }

  public function isModifyableServer ()
  {
    return $this->modifyableServer;
  }

  /*! \brief  Returns TRUE if the action caused an error.
      @return Boolean TRUE on error else FALSE
   */
  public function is_error ()
  {
    return ($this->error != '');
  }

  /*! \brief  Resets the error message.
   */
  public function reset_error ()
  {
    $this->error = '';
  }

  public function get_account_id ()
  {
    $this->build_account_id();
    return $this->account_id;
  }

  /*! \brief  Create a new account id, like 'user/name@domain.com'.
   */
  protected function build_account_id ()
  {
    /* Build account identicator */
    if ($this->type == "user") {
      $prefix     = $this->user_prefix;
      $acc_string = $this->user_id;
    } else {
      $prefix     = $this->share_prefix;
      $acc_string = $this->share_id;
    }

    /* Create account prefix and respect "useSlashesInId"
       Do not replace escaped dots for useSlashesInId.
     */
    $uattrib = $this->uattrib;
    if ($this->useSlashesInId) {
      $prefix     = preg_replace('/([^\\\\])\./', "\\1/", $prefix);
      $acc_string = preg_replace('/([^\\\\])\./', "\\1/", $acc_string);
    }
    $prefix     = preg_replace("/\\\\([\.\/])/", "\\1", $prefix);
    $acc_string = preg_replace("/\\\\([\.\/])/", "\\1", $acc_string);

    $mail = $domain = $mailpart = '';
    if (isset($this->parent->mail)) {
      $mail = $this->parent->mail;
      if (preg_match('/\@/', $mail)) {
        list($mailpart, $domain) = explode('@', $mail);
      }
    }

    /* Create account_id
     */
    $from   = ["/%cn%/i","/%uid%/i","/%prefix%/i","/%uattrib%/i","/%domain%/i","/%mailpart%/i","/%mail%/i"];
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
    $to     = [$cn,$uid,$prefix,$attrib,$domain,$mailpart,$mail];
    $acc_id = trim(strtolower(preg_replace($from, $to, $acc_string)));

    /* Check for not replaced pattern.
     */
    if (preg_match("/%/", $acc_id)) {
      $notr = preg_replace("/^[^%]*/", "", $acc_id);
      if (!empty($notr)) {
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Warning</b>",
            sprintf("<b>MAIL: WARNING unknown pattern in account creation string '%s' near '%s'</b>", $acc_id, $notr));

        /* Remove incomprehensible patterns */
        $acc_id = preg_replace("/%[^%]+%/", "", $acc_id);
      }
    }


    if ($this->account_id != $acc_id) {
      $this->account_id = $acc_id;
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $acc_id, "<b>MAIL:</b> AccountID generated");
    }
  }

  /*! \brief  Creates a valid folder id for a given folder name.
               e.g. $folder_id = "INBOX/test"  &&  $this->account_id = "share/mailbox@fusiondirectory.org"
               will result in "share/mailbox/test@fusiondirectory.org"
              This function is mainly used to read and write folder permissions.
      @return String A valid folder id
   */
  protected function create_folder_id ($folder)
  {
    $delim = ($this->useSlashesInId ? '/' : '.');
    if (!empty($folder)) {
      $folder = trim(preg_replace("/^INBOX[\.\/]*/i", '', $folder));
    }
    if (!empty($folder)) {
      $folder = $delim.$folder;
    }

    /* We need to move the domain to the end
     * Example: user/john/Drafts@domain.com instead of user/john@domain.com/Drafts
    */
    $account  = $this->account_id;
    $suffix   = '';
    if (preg_match('/^(.+)(\@[^@]+)$/', $this->account_id, $m)) {
      $account  = $m[1];
      $suffix   = $m[2];
    }

    return trim($account.$folder.$suffix);
  }

  /*! \brief Saves sieve settings
   */
  public function saveSieveSettings ()
  {
    $this->reset_error();
    return TRUE;
  }

  /*! \brief  Creates or Updates the mailAccount represented by this class.
   */
  public function updateMailbox ()
  {
    $this->reset_error();
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id."</b>",
        "<b>MAIL: Update account</b> from server :".$this->mailServer);

    return TRUE;
  }

  /*! \brief  Removes the mailbox represented by this class,
               and update shared folder ACLs .
   */
  public function deleteMailbox ()
  {
    $this->reset_error();
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id."</b>",
        "<b>MAIL: Remove account</b> from server :".$this->mailServer);

    return TRUE;
  }

  /*! \brief  Returns the used mail attribute (mail,uid)
      @param  String  One out of 'mail','uid'
   */
  public function getUAttrib ()
  {
    return $this->uattrib;
  }

  /*! \brief  Returns the used mail attribute (mail,uid)
      @param  String  One out of 'mail','uid'
   */
  public function getUAttribValue ()
  {
    $uattrib = $this->getUAttrib();
    return $this->parent->$uattrib;
  }

  /*! \brief  Returns whether the quota settings are enabled or not
      @return Boolean TRUE if enabled else FALSE
   */
  public function quotaEnabled ()
  {
    return $this->enableQuota;
  }

  /*! \brief  Returns the used quota
      @return Integer Quota used.
   */
  public function getQuotaUsage ()
  {
    return -1;
  }

  /*! \brief  Returns the quota restrictions.
      @return int|null
   */
  public function getQuota ($quotaValue)
  {
    return $quotaValue;
  }

  /*! \brief  Sets the mail quota
   */
  public function setQuota ($number)
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
  public function getMailboxList ()
  {
    return ["INBOX"];
  }

  /*! \brief  Returns whether the vacation is enabled
      @return Boolean TRUE, FALSE
   */
  public function vacationEnabled ()
  {
    return $this->enableVacation;
  }

  /*! \brief  Returns whether the vacation range is selectable or not
      @return Boolean TRUE, FALSE
   */
  public function vacationRangeEnabled ()
  {
    return ($this->vacationEnabled() && $this->enableVacationRange);
  }

  /*! \brief  Returns whether forwarding to group non-members is possible
      @return Boolean TRUE, FALSE
   */
  public function groupForwardingEnabled ()
  {
    return $this->enableGroupForwarding;
  }

  /*! \brief  Returns whether IMAP ACLs are supported for group members
   */
  public function groupAclsEnabled (): bool
  {
    return $this->enableGroupACLs;
  }

  /*! \brief  Returns whether forcing to only send/recieve local mail is possible
      @return Boolean TRUE, FALSE
   */
  public function localOnlyEnabled ()
  {
    return $this->enableLocalOnly;
  }

  /*! \brief  Returns whether forwarding without storing the mail is possible
      @return Boolean TRUE, FALSE
   */
  public function forwardOnlyEnabled ()
  {
    return $this->enableForwardOnly;
  }

  /*! \brief  Checks dependencies to other FusionDirectory plugins.
   */
  public function accountCreateable (&$reason = "")
  {
    return TRUE;
  }

  /*! \brief  Checks whether this account is removeable or not.
              There may be some dependencies left, eg. kolab.
   */
  public function accountRemoveable (&$reason = "")
  {
    return TRUE;
  }

  /*! \brief  Returns configuration of selected mail server
      @return Array configuration
  */
  protected function getServerConfig ()
  {
    $serverList = static::getMailServers();

    if (!count($serverList)) {
      $this->error = _('There are no IMAP compatible mail servers defined!');
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,
          '<b>IMAP: No mail servers configured, check systems->server->services</b>', '');
      return FALSE;
    } elseif (!isset($serverList[$this->mailServer])) {
      $this->error = _('Mail server for this account is invalid!');
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->mailServer,
          '<b>IMAP: The selected mail server is invalid</b>');
      return FALSE;
    } else {
      return $serverList[$this->mailServer];
    }
  }

  /*! \brief  Returns all mail servers configured in FusionDirectory
      @return Array  All useable mail servers
  */
  static public function getMailServers ()
  {
    if (session::global_is_set('mailServers')) {
      return session::global_get('mailServers');
    }
    $methods = static::get_methods();
    $servers = [];
    foreach ($methods as $class) {
      $servers = array_merge($servers, $class::get_server_list());
    }
    session::global_set('mailServers', $servers);
    return $servers;
  }

  static public function resetMailServersCache ()
  {
    session::global_un_set('mailServers');
  }

  /*! \brief  Returns the available mailMethods
      @return Array   A list of all avaialable mailMethods_
   */
  static public function get_methods ()
  {
    global $class_mapping;
    $available = ['mailMethod' => 'mailMethod'];
    foreach ($class_mapping as $class => $path) {
      if (preg_match('/^mailMethod.+/', $class)) {
        $available[$class] = $class;
      }
    }
    return $available;
  }

  static public function getServerMethod ($server)
  {
    $methods = mailMethod::get_methods();
    foreach ($methods as $class) {
      if ($class::serverMatch($server)) {
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $class, 'Mail method for server "'.$server.'"');
        return $class;
      }
    }
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '', 'No method found for server "'.$server.'"');
    return 'mailMethod';
  }

  static public function serverMatch ($server)
  {
    return isset(static::get_server_list()[$server]);
  }

  static public function getInstance ($class, $parent, $type = NULL)
  {
    if ($parent instanceof mailAccount) {
      $type = 'user';
    } elseif ($type === NULL) {
      $type = 'group';
    }
    $obj = new $class($parent, $type);
    $obj->init();
    return $obj;
  }

  static public function getInstanceFromServer (string $server, $parent, string $type = NULL)
  {
    /* Intialize the used mailMethod */
    if ($server == '') {
      $method = 'mailMethod';
    } else {
      $method = static::getServerMethod($server);
    }
    $mailMethod = static::getInstance($method, $parent, $type);
    $mailMethod->fixAttributesOnLoad();

    return $mailMethod;
  }

  /*! \brief  Returns configured acls
   */
  public function getFolderACLs (): array
  {
    return [];
  }

  /*! \brief  Write ACLs back to imap or what ever
   */
  public function setFolderACLs (array $acls): bool
  {
    return TRUE;
  }

  /*! \brief  Returns a list of all possible acls.
      @return Array   ACLs.
  */
  public function getAclTypes (): array
  {
    return $this->acl_mapping;
  }

  /*! \brief  Returns the configured mailMethod
      @return String  the configured mail method or ""
   */
  static public function get_current_method ()
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

  static function quota_to_image ($use, $quota)
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

  static public function get_server_list ()
  {
    global $config;
    $serverList = [];

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=fdImapServer)', ['cn']);
    while ($attrs = $ldap->fetch()) {
      $serverList[$attrs['cn'][0]] = [
        'server_dn'     => $attrs['dn'],
        'connect'       => '',
        'admin'         => '',
        'password'      => '',
        'sieve_server'  => '',
        'sieve_option'  => '',
        'sieve_port'    => ''
      ];
    }

    return $serverList;
  }

  public function additionalInformations ()
  {
    return [];
  }
}
