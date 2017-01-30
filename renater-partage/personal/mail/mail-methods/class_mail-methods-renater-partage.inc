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
  protected $enableQuota            = TRUE;
  protected $enableVacationRange    = FALSE;
  protected $enableFolderTypes      = FALSE;

  protected $user_id = '%MAIL%';

  private $http_options = array();
  private $ssl_options = array();
  private $token;
  private $tokenTime;
  private $cachedAccount;

  /*! \brief  Connect to renater PARTAGE API and store the auth token
      @return Boolean True if connection was successful
   */
  public function connect()
  {
    parent::connect();
    $servers = static::getMailServers();
    $infos = $servers[$this->parent->gosaMailServer];

    $this->connected = FALSE;

    $ts = time();
    $preauth = hash_hmac('sha1', $infos['domain'].'|'.$ts, $infos['key']);

    $request = array(
      'domain'    => $infos['domain'],
      'timestamp' => $ts,
      'preauth'   => $preauth
    );

    $response = $this->query('Auth', $request);

    if ($response === FALSE) {
      return FALSE;
    } elseif (!isset($response['Response']['token'])) {
      $this->error = _('Server did not return auth token');
      return FALSE;
    } else {
      $this->token = $response['Response']['token'];
      $this->tokenTime = $ts;
      $this->connected = TRUE;
      return TRUE;
    }
  }

  private function query($command, $post)
  {
    $servers = static::getMailServers();
    $infos = $servers[$this->parent->gosaMailServer];

    if (is_array($post)) {
      $post = http_build_query($post);
    }

    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, htmlentities($post, ENT_COMPAT, 'UTF-8'),
      '<b>MAIL: Query</b>');

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
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, htmlentities($response, ENT_COMPAT, 'UTF-8'),
        '<b>MAIL: Answer</b>');
      $response = xml::xml2array($response, FALSE);

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

  /*! \brief  Disconnect from the API (deletes the auth token)
   */
  public function disconnect()
  {
    parent::disconnect();
    unset($this->token);
    unset($this->tokenTime);
  }

  /*! \brief  Returns the connection status of this method.
      @return Boolean True if this method is connected else false.
   */
  public function is_connected()
  {
    /* If the token is older than 5 minutes it will get rejected. 5min = 300sec */
    return (parent::is_connected() && (time() - $this->tokenTime < 250));
  }

  /*! \brief  Returns true the current object represents a valid account
      @return Boolean TRUE if this is a valid account else FALSE
  */
  public function account_exists()
  {
    $this->reset_error();
    return $this->cacheAccount();
  }

  private function cacheAccount()
  {
    if (empty($this->account_id)) {
      return FALSE;
    } elseif (isset($this->cachedAccount['account']) && ($this->account_id == $this->cachedAccount['id'])) {
      return TRUE;
    }
    $answer = $this->query('GetAccount/'.$this->token, array('name' => $this->account_id));
    if (($answer !== FALSE) && isset($answer['Response']['account'])) {
      $this->cachedAccount = array(
        'account' => $answer['Response']['account'],
        'id'      => $this->account_id,
        'time'    => time()
      );
      return TRUE;
    } else {
      return FALSE;
    }
  }

  public function updateMailbox()
  {
    parent::updateMailbox();
    $servers = static::getMailServers();
    $infos = $servers[$this->parent->gosaMailServer];

    $this->build_account_id();
    if (!$this->parent->initially_was_account) {
      /* Creation */
      if ($this->account_exists()) {
        $this->error = _('An account with the same email address already exists in Partage');
        return FALSE;
      }

      $account = array(
        'name'                                => $this->account_id,
        'zimbraCOSId'                         => $infos['cos_id'],
        'zimbraMailQuota'                     => $this->parent->gosaMailQuota,
        'zimbraPrefMailLocalDeliveryDisabled' => ($this->parent->flag_ownmailbox ? 'TRUE' : 'FALSE'),
        'zimbraPrefMailForwardingAddress'     => reset($this->parent->gosaMailForwardingAddress),
      );
      //~ carLicense -> supann edupersonprincipalname (eppn)
      //~ zimbraHideInGal -> supann liste rouge
      //~ co
      //~ company
      //~ initials
      //~ postalCode
      //~ street
      //~ zimbraAccountStatus -> ? lock account?
      //~ zimbraPasswordMustChange

      $userFields = array(
        'sn', 'givenName', 'description',
        'l', 'st',
        // postalAddress
        'telephoneNumber', 'mobile', 'pager', 'facsimileTelephoneNumber',
        'displayName', 'homePhone',
        'title', 'ou',
      );
      $userTab = $this->parent->parent->getBaseObject();
      foreach ($userFields as $field) {
        if (!empty($userTab->field)) {
          $account[$field] = $userTab->field;
        }
      }

      if ($userTab->attributesAccess['userPassword']->getClear() != '') {
        /* Password was changed so we have the clear text password */
        $account['password'] = $userTab->attributesAccess['userPassword']->getClear();
      } else {
        /* If we don’t, just give the hash */
        $account['password']      = '';
        $account['userPassword']  = $userTab->userPassword;
      }

      $answer = $this->query('CreateAccount/'.$this->token, $account);

      if ($answer === FALSE) {
        return FALSE;
      }
    } else {
      if ($this->account_id !== $this->initial_account_id) {
        /* Renaming */
        if ($this->account_exists()) {
          $this->error = _('An account with the same email address already exists in Partage');
          return FALSE;
        }
        $account = $account = array(
          'name'                                => $this->account_id,
        );
      } else {
        $account = array();
      }

      /* Here we use $this->parent->attrs to only catch fields which were modified */
      if (isset($this->parent->attrs['gosaMailForwardingAddress'])) {
        $count = count($this->parent->attrs['gosaMailForwardingAddress']);
        if ($count >= 1) {
          $address = $this->parent->attrs['gosaMailForwardingAddress'][0];
          if ($count > 1) {
            msg_dialog::display(_('Warning'), sprintf(_('Several addresses were given in gosaMailForwardingAddress but only one is supported by PARTAGE. %s will be sent to PARTAGE.'), $address));
          }
          $account['zimbraPrefMailForwardingAddress'] = $address;
        } else {
          $account['zimbraPrefMailForwardingAddress'] = '';
        }
      }
      if (isset($this->parent->attrs['gosaMailDeliveryMode'])) {
        $gosaMailDeliveryMode = $this->parent->attrs['gosaMailDeliveryMode'];
        if (is_array($gosaMailDeliveryMode)) {
          $gosaMailDeliveryMode = $gosaMailDeliveryMode[0];
        }
        $account['zimbraPrefMailLocalDeliveryDisabled'] = (preg_match('I', $gosaMailDeliveryMode) ? 'TRUE' : 'FALSE');
      }
      if (isset($this->parent->attrs['gosaMailQuota'])) {
        $account['zimbraMailQuota'] = $this->parent->attrs['gosaMailQuota'];
      }
      $userTab = $this->parent->parent->getBaseObject();
      /* TODO: call SetPassword for password change outside of creation */

      if (!empty($account)) {
        $answer = $this->query('ModifyAccount/'.$this->token, $account);

        if ($answer === FALSE) {
          return FALSE;
        }
      }
    }

    /* TODO: Handle aliases */
    //~ if (isset($this->parent->attrs['gosaMailAlternateAddress'])) {
    //~ }
    return TRUE;
  }

  public function deleteMailbox()
  {
    parent::deleteMailbox();
    return TRUE;
  }

  /*! \brief  Returns the used quota in MiB
      @return Integer Quota used.
   */
  public function getQuotaUsage()
  {
    if ($this->cacheAccount()) {
      /* Partage sends quota in bytes */
      return ($this->cachedAccount['account']['used'] / (1024*1024));
    }
    return parent::getQuotaUsage();
  }

  /*! \brief  Returns the quota restrictions in MiB.
      @return Integer Quota restrictions.
   */
  public function getQuota($quotaValue)
  {
    if ($this->cacheAccount()) {
      /* Partage sends quota in bytes */
      return ($this->cachedAccount['account']['quota'] / (1024*1024));
    }
    return $quotaValue;
  }

  /*! \brief  Sets the mail quota
   */
  public function setQuota($number)
  {
    return parent::setQuota($number);
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
            'cos_name'    => $cosName,
            'cos_id'      => $cosId
          );
        }
      }
    }

    return $serverList;
  }
}
?>
