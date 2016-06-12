<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class mailMethodDovecot extends mailMethod
{
  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail   = FALSE;

  /* Allow modification of the mail server attribute existing mail accounts */
  protected $modifyableServer = FALSE;

  /* internal */
  protected $imap_handle  = NULL;
  protected $quota_loaded = FALSE;

  public function is_connected()
  {
    return (parent::is_connected() && $this->imap_handle);
  }

  public function connect()
  {
    global $config;
    parent::connect();

    if ($config->get_cfg_value("mailMethodDisabled", "FALSE") == "TRUE") {
      return FALSE;
    }

    $servers = parent::getMailServers();

    if (!count($servers)) {
      $this->error = _("There are no IMAP compatible mail servers defined!");
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,
          "<b>IMAP: No mail servers configured, check systems->server->service->imap.</b>", "");
      return FALSE;
    } elseif (!isset($servers[$this->parent->gosaMailServer])) {
      $this->error = _("Mail server for this account is invalid!");
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,
          "<b>IMAP: The selected mail server '".$this->parent->gosaMailServer."' is invalid.</b>", "");
      return FALSE;
    } else {
      $cfg = $servers[$this->parent->gosaMailServer];
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

    $this->build_account_id();
    $cfg['admin'] = $this->account_id.'*'.$cfg['admin'];

    /* Setting connect timeout to 10 seconds,
        else the FusionDirectory UI may freeze for 60 seconds.
       (PHP default is 'default_socket_timeout = 60') */
    $timeout = $config->get_cfg_value("imapTimeout", 10);
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $timeout,
          "<b>IMAP: Setting imap connect timeout to</b> (seconds)");
    imap_timeout(1, $timeout);

    $this->imap_handle = @imap_open($cfg['connect'], $cfg['admin'], $cfg['password'], OP_HALFOPEN);

    /* Mailbox reachable? */
    if ($this->imap_handle === FALSE) {
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Failed</b> :".imap_last_error(),
        "<b>IMAP:</b> ".$cfg['admin']."@".$cfg['connect']);
      if ($cfg['mkdir']) {
        if ($this->argonautCreateFolder()) {
          $this->imap_handle = @imap_open($cfg['connect'], $cfg['admin'], $cfg['password'], OP_HALFOPEN);
        } else {
          return FALSE;
        }
      }
      if ($this->imap_handle === FALSE) {
        $this->error = imap_last_error();

        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Failed</b> :".imap_last_error(),
          "<b>IMAP:</b> ".$cfg['admin']."@".$cfg['connect']);

        $this->connected = FALSE;
        return FALSE;
      }
    }
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>successful</b>",
        "<b>IMAP:</b> ".$cfg['admin']."@".$cfg['connect']);
    $this->connected = TRUE;

    return TRUE;
  }

  protected function loadQuota()
  {
    global $config;
    if (!$this->quotaEnabled()) {
      return TRUE;
    }
    if ($config->get_cfg_value("mailMethodDisabled", "FALSE") == "TRUE") {
      return FALSE;
    }
    if (!$this->is_connected()) {
      trigger_error('Method not connected, catch error.');
      return FALSE;
    }

    $this->reset_error();

    /* Load quota settings */
    $quota_value = @imap_get_quotaroot($this->imap_handle, 'INBOX'); /* FIXME: is this always INBOX? */

    if (is_array($quota_value)) {
      if (count($quota_value)) {
        $this->quotaUsage   = $quota_value['STORAGE']['usage'] / 1024;
        if (($quota_value['STORAGE']['limit'] == 2147483647) || ($quota_value['STORAGE']['limit'] <= 0)) {
          $this->quotaValue = '';
        } else {
          $this->quotaValue = $quota_value['STORAGE']['limit'] / 1024;
        }
        $this->quota_loaded = TRUE;

        /* Write debug output */
        if ($this->quotaValue == "") {
          $quota = "(".$this->quotaUsage." / unlimited)";
        } else {
          $quota = "(".$this->quotaUsage." / ".$this->quotaValue.")";
        }
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $quota,
            "<b>IMAP: Successfully received account quota</b>");
      } else {
        $this->quotaUsage = '';
        $this->quotaValue = '';
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, '',
            "<b>IMAP: Received empty account quota information</b>");
      }
    } else {
      $this->error = imap_last_error();
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, imap_last_error(),
          "<b>IMAP: Failed to receive account quota</b>");
    }
  }

  /* Get quota usage in MB */
  public function getQuotaUsage()
  {
    mailMethod::getQuotaUsage();
    if (!$this->quota_loaded) {
      $this->loadQuota();
    }
    return $this->quotaUsage;
  }

  /*
   * Create the folder for the user INBOX through Argonaut
   */
  private function argonautCreateFolder()
  {
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id."</b>",
      "<b>Attempting to create folder through Argonaut</b> on server :".$this->parent->gosaMailServer);

    $servers  = parent::getMailServers();
    $cfg      = $servers[$this->parent->gosaMailServer];

    $o_queue = new supportDaemon();
    if (!$o_queue->is_error()) {
      $o_queue->append_call('Dovecot.create_mailbox',
                            array($cfg['mac']),
                            array('args' => array($this->account_id, $this->parent->uidNumber, $this->parent->gidNumber)));
    }
    /* If we got an error while connecting or sending the call */
    if ($o_queue->is_error()) {
      $this->error = sprintf(_('Error while trying to contact Dovecot server through Argonaut: %s'), $o_queue->get_error());
      return FALSE;
    }

    return TRUE;
  }

  /*! \brief  Checks whether this account is removeable or not.
   */
  public function accountRemoveable (&$reason = "")
  {
    return FALSE;
  }

  static public function get_server_list()
  {
    global $config;
    $serverList = array();

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search ('(objectClass=fdDovecotServer)',
                  array('cn', 'fdDovecotConnect', 'fdDovecotAdmin', 'fdDovecotPassword', 'fdDovecotArgonautMkdir', 'macAddress'));
    while ($attrs = $ldap->fetch()) {
      $serverList[$attrs['cn'][0]] = array(
        'server_dn' => $attrs['dn'],
        'connect'   => $attrs['fdDovecotConnect'][0],
        'admin'     => $attrs['fdDovecotAdmin'][0],
        'password'  => $attrs['fdDovecotPassword'][0],
        'mkdir'     => (isset($attrs['fdDovecotArgonautMkdir'])?(strtoupper($attrs['fdDovecotArgonautMkdir'][0]) == 'TRUE'):TRUE),
        'mac'       => $attrs['macAddress'][0],
      );
    }

    return $serverList;
  }
}
?>
