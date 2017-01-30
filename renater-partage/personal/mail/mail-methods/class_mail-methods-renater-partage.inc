<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2017  FusionDirectory

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

class mailMethodRenaterPartage extends mailMethod
{
  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail   = FALSE;

  /* Allow modification of the mail server attribute existing mail accounts */
  protected $modifyableServer = FALSE;

  protected $enableQuota            = TRUE;
  protected $enableVacationRange    = FALSE;
  protected $enableFolderTypes      = FALSE;

  protected $user_id = '%MAIL%';

  private $debug = FALSE;
  private $http_options = array();
  private $ssl_options = array();

  public function updateMailbox()
  {
    parent::updateMailbox();
    return TRUE;
  }

  public function deleteMailbox()
  {
    parent::deleteMailbox();
    return TRUE;
  }

  /*! \brief  Connect services like imap.
              Not necessary for the base class.
      @return Boolean True if this method is connected else false.
   */
  public function connect()
  {
    parent::connect();
    $servers = static::getMailServers();
    $infos = $servers[$this->parent->gosaMailServer];

    $this->connected = FALSE;

    $ts = time();
    $preauth = hash_hmac('sha1', $infos['domain'].'|'.$ts, $infos['key']);

    $request = http_build_query(
      array(
        'domain'    => $infos['domain'],
        'timestamp' => $ts,
        'preauth'   => $preauth
      )
    );

    $response = $this->query('Auth', $request);

    if ($response === FALSE) {
      return FALSE;
    } elseif (!isset($response['Response']['token'])) {
      $this->error = _('Server did not return auth token');
      return FALSE;
    } else {
      $this->token = $response['Response']['token'];
      $this->connected = TRUE;
      return TRUE;
    }
  }

  private function query($command, $post)
  {
    $servers = static::getMailServers();
    $infos = $servers[$this->parent->gosaMailServer];

    $this->debug && $debug = '***** Request *****'."\n".htmlentities($post, ENT_COMPAT, 'UTF-8')."\n".'***** End Of request *****'."\n\n";

    // performs the HTTP(S) POST
    $opts = array (
      'http' => array_merge(
        array (
          'method'      => 'POST',
          'user_agent'  => $infos['user_agent'],
          'content'     => $post
        ),
        $this->http_options
      ),
      'ssl' => $this->ssl_options
    );

    $context  = stream_context_create($opts);
    if ($fp = @fopen($infos['uri'].'/'.$command, 'r', false, $context)) {
      $response = '';
      while($row = fgets($fp)) {
        $response.= trim($row)."\n";
      }
      $this->debug && $debug .= '***** Server response *****'."\n".htmlentities($response, ENT_COMPAT, 'UTF-8').'***** End of server response *****'."\n";
      $response = xml::xml2array($response, FALSE);

      // debug output
      if ($this->debug) {
        echo nl2br($debug);
      }

      if (!isset($response['Response']['status'])) {
        $this->error = _('Partage API answer malformated');
        return FALSE;
      } elseif ($response['Response']['status'] != 0) {
        if (isset($response['Response']['message'])) {
          $this->error = sprintf(_('Partage API Auth failed: %s'), $response['Response']['message']);
        } else {
          $this->error = _('Partage API Auth failed with no error message');
        }
        return FALSE;
      }

      return $response;
    } else {
      $this->error = sprintf(_('Unable to connect to %s'), $infos['uri']);
      return FALSE;
    }
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
    if (empty($this->account_id)) {
      return;
    }
    $request  = http_build_query(array('name' => $this->account_id));
    $answer   = $this->query('GetAccount/'.$this->token, $request);
    return (($answer !== FALSE) && isset($answer['Response']['account']));
  }

  static public function get_server_list()
  {
    global $config;
    $serverList = array();

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search ('(&(objectClass=fdRenaterPartageServer)(fdRenaterPartageServerMailDomain=*))',
                  array('cn', 'fdRenaterPartageServerUri', 'fdRenaterPartageServerMailDomain', 'fdRenaterPartageServerUserAgent'));
    while ($attrs = $ldap->fetch()) {
      unset($attrs['fdRenaterPartageServerMailDomain']['count']);
      foreach($attrs['fdRenaterPartageServerMailDomain'] as $domainAndKey) {
        list($domain, $key, $cosString) = explode(':', $domainAndKey, 3);
        $cosArray = explode(',', $cosString);
        foreach ($cosArray as $cos) {
          list ($cosName, $cosId) = explode('|', $cos);
          $serverList[$attrs['cn'][0].' - '.$domain.' - '.$cosName] = array(
            'server_dn'   => $attrs['dn'],
            'user_agent'  => $attrs['fdRenaterPartageServerUserAgent'][0],
            'uri'         => $attrs['fdRenaterPartageServerUri'][0],
            'domain'      => $domain,
            'key'         => $key,
            'CosName'     => $cosName,
            'CosId'       => $cosId
          );
        }
      }
    }

    return $serverList;
  }
}
?>
