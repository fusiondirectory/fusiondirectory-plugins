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
  protected $enableVacation         = FALSE;
  protected $enableFolderTypes      = FALSE;
  protected $enableGroupForwarding  = FALSE;
  protected $enableLocalOnly        = FALSE;
  protected $enableForwardOnly      = FALSE;

  protected $user_id  = '%MAIL%';
  protected $share_id = '%MAIL%';

  private $http_options = array();
  private $ssl_options = array();
  private $token;
  private $tokenTime;
  private $cachedAccount;

  /*! \brief  Connect to renater PARTAGE API and store the auth token
      @return Boolean True if connection was successful
   */
  public function connect ()
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
      $this->token      = $response['Response']['token'];
      $this->tokenTime  = $ts;
      $this->connected  = TRUE;
      return TRUE;
    }
  }

  private function query ($command, $post)
  {
    $servers = static::getMailServers();
    $infos = $servers[$this->parent->gosaMailServer];

    if (is_array($post)) {
      $post = http_build_query($post);
      /* PARTAGE API does not support array[0]= syntax, it wants array[]= */
      $post = preg_replace('/%5B[0-9]+%5D/simU', '%5B%5D', $post);
    }

    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $command, '<b>MAIL: Command</b>');
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, htmlentities($post, ENT_COMPAT, 'UTF-8'),
      '<b>MAIL: Query</b>');

    // performs the HTTP(S) POST
    $opts = array (
      'http' => array_merge(
        array (
          'method'      => 'POST',
          'user_agent'  => $infos['user_agent'],
          'header'      => 'Content-type: application/x-www-form-urlencoded'."\r\n"
                          .'Content-Length: '.strlen($post)."\r\n",
          'content'     => $post
        ),
        $this->http_options
      ),
      'ssl' => $this->ssl_options
    );

    if ($command != 'Auth') {
      $command .= '/'.$this->token;
    }

    $context  = stream_context_create($opts);
    $tries    = 0;
    do {
      /* Because of what seems to be a PARTAGE bug there is a 403 when you try
       *  to access an entry which is not finished being created */
      if ($tries) {
        /* When retrying after 403, wait 10 seconds */
        sleep(10);
      }
      $fp = fopenWithErrorHandling($infos['uri'].'/'.$command, 'r', FALSE, $context);
      $tries++;
    } while (is_array($fp) && preg_match('/403/', end($fp)) && ($tries < 3));
    if (!is_array($fp)) {
      $response = '';
      while ($row = fgets($fp)) {
        $response .= trim($row)."\n";
      }
      $response = xml::xml2array($response, FALSE);
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $response, '<b>MAIL: Answer</b>');

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
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $fp, '<b>MAIL: No Answer</b>');
      if (!empty($fp)) {
        $this->error = sprintf(_('Unable to connect to %s: %s'), $infos['uri'], implode("\n", $fp));
      } else {
        $this->error = sprintf(_('Unable to connect to %s'), $infos['uri']);
      }
      return FALSE;
    }
  }

  /*! \brief  Disconnect from the API (deletes the auth token)
   */
  public function disconnect ()
  {
    parent::disconnect();
    unset($this->token);
    unset($this->tokenTime);
  }

  /*! \brief Checks data consistency
  */
  public function check ()
  {
    $messages = parent::check();
    $servers  = static::getMailServers();
    $infos    = $servers[$this->parent->gosaMailServer];

    $alternates = $this->parent->gosaMailAlternateAddress;
    foreach ($alternates as $alternate) {
      if (!preg_match('/@'.preg_quote($infos['domain'], '/').'$/', $alternate)) {
        $messages[] = sprintf(_('Email address "%s" does not use the correct partage domain "%s"'), $alternate, $infos['domain']);
      }
    }

    return $messages;
  }

  /*! \brief  Returns the connection status of this method.
      @return Boolean True if this method is connected else false.
   */
  public function is_connected ()
  {
    /* If the token is older than 5 minutes it will get rejected. 5min = 300sec */
    return (parent::is_connected() && (time() - $this->tokenTime < 250));
  }

  /*! \brief  Returns true the current object represents a valid account
      @return Boolean TRUE if this is a valid account else FALSE
  */
  public function account_exists ()
  {
    $this->reset_error();
    return $this->cacheAccount();
  }

  private function cacheAccount ()
  {
    if (empty($this->account_id)) {
      return FALSE;
    } elseif (isset($this->cachedAccount['account']) && ($this->account_id == $this->cachedAccount['id'])) {
      return TRUE;
    }
    if ($this->type == 'user') {
      $command    = 'GetAccount';
      $answerkey  = 'account';
    } else {
      $command    = 'GetGroup';
      $answerkey  = 'group';
    }
    $answer = $this->query($command, array('name' => $this->account_id));
    if (($answer !== FALSE) && isset($answer['Response'][$answerkey])) {
      $this->cachedAccount = array(
        'account' => $answer['Response'][$answerkey],
        'id'      => $this->account_id,
        'time'    => time()
      );
      $this->cleanCachedAccountArrays();
      return TRUE;
    } else {
      return FALSE;
    }
  }

  /*
   * PARTAGE API returns weird stuff like:
   * [members] => Array ( member => Array ( user1, user2 ) )
   * or:
   * [members] => Array ( member => user1 )
   * We want:
   * [members] => Array ( user1, user2 )
   * */
  private function cleanCachedAccountArrays ()
  {
    $keys = array('members', 'zimbraMailAlias', 'zimbraAllowFromAddress');
    foreach ($keys as $key) {
      if (isset($this->cachedAccount['account'][$key])) {
        $this->cachedAccount['account'][$key] = array_values($this->cachedAccount['account'][$key]);
        if (isset($this->cachedAccount['account'][$key][0]) && is_array($this->cachedAccount['account'][$key][0])) {
          $this->cachedAccount['account'][$key] = $this->cachedAccount['account'][$key][0];
        }
      }
    }
  }

  private function getAccountArray ($infos)
  {
    $account = array(
      'name'                                => $this->account_id,
    );
    if ($this->type == 'user') {
      $account['zimbraCOSId']                         = $infos['cos_id'];
      $account['zimbraMailQuota']                     = $this->parent->gosaMailQuota * 1024 * 1024;
      $account['zimbraPrefMailLocalDeliveryDisabled'] = ($this->parent->flag_ownmailbox ? 'TRUE' : 'FALSE');
      if (!empty($this->parent->gosaMailForwardingAddress)) {
        $count = count($this->parent->gosaMailForwardingAddress);
        $address = $this->parent->gosaMailForwardingAddress[0];
        if ($count > 1) {
          msg_dialog::display(_('Warning'), sprintf(_('Several addresses were given in gosaMailForwardingAddress but only one is supported by PARTAGE. %s will be sent to PARTAGE.'), $address));
        }
        $account['zimbraPrefMailForwardingAddress'] = $address;
      }
    }

    if ($this->type == 'user') {
      $mainFields = array(
        'sn', 'givenName', 'description',
        'l', 'st',
        'telephoneNumber', 'mobile', 'pager', 'facsimileTelephoneNumber',
        'displayName', 'homePhone',
        'title', 'ou',
        'street', 'postalCode', 'postOfficeBox'
      );
    } else {
      $mainFields = array(
        'description'
      );
    }
    $mainTab = $this->parent->parent->getBaseObject();
    foreach ($mainFields as $field) {
      if (isset($mainTab->$field)) {
        $account[$field] = $mainTab->$field;
      }
    }

    if ($this->type == 'user') {
      $userMatchingFields = array(
        'o' => 'company',
        'roomNumber' => 'physicalDeliveryOfficeName'
      );
      foreach ($userMatchingFields as $fdField => $partageField) {
        if (isset($mainTab->$fdField)) {
          $account[$partageField] = $mainTab->$fdField;
        }
      }
      $account['zimbraAccountStatus'] = ($mainTab->attributesAccess['userPassword']->isLocked() ? 'locked' : 'active');
      $account['initials'] = mb_strtoupper(mb_substr($mainTab->givenName, 0, 1).mb_substr($mainTab->sn, 0, 1));

      /* Sync fields from SUPANN if tab is active */
      if (isset($this->parent->parent->by_object['supannAccount']) && $this->parent->parent->by_object['supannAccount']->is_account) {
        $supannTab = $this->parent->parent->by_object['supannAccount'];

        $account['carLicense']      = $supannTab->eduPersonPrincipalName;
        $account['zimbraHideInGal'] = ($supannTab->supannListeRouge ? 'TRUE' : 'FALSE');
      }

      /* Sync fields from personal if tab is active */
      if (isset($this->parent->parent->by_object['personalInfo']) && $this->parent->parent->by_object['personalInfo']->is_account) {
        $personalTab    = $this->parent->parent->by_object['personalInfo'];
        $account['co']  = $personalTab->co;
      }
    } else {
      /* Sync partage group fields */
      $partageTabName = 'partageGroup';
      if ($this->type == 'sympaAlias') {
        $partageTabName = 'sympaAliasPartage';
      }
      if (isset($this->parent->parent->by_object[$partageTabName]) && $this->parent->parent->by_object[$partageTabName]->is_account) {
        $partageFields = array(
          'displayName' => 'displayName',
          'fdRenaterPartageGroupMailStatus' => 'zimbraMailStatus',
          'fdRenaterPartageGroupNotes' => 'zimbraNotes'
        );
        $partageTab = $this->parent->parent->by_object[$partageTabName];
        foreach ($partageFields as $fdField => $partageField) {
          if (isset($partageTab->$fdField)) {
            $account[$partageField] = $partageTab->$fdField;
          }
        }
        $account['zimbraHideInGal'] = ($partageTab->fdRenaterPartageGroupHideInGal ? 'TRUE' : 'FALSE');
        $account['zimbraDistributionListSendShareMessageToNewMembers'] = ($partageTab->fdRenaterPartageGroupSendShareMessageToNewMembers ? 'TRUE' : 'FALSE');
      }
    }

    return $account;
  }

  private function createAccount ($infos)
  {
    $account = $this->getAccountArray($infos);

    /* Remove empty values */
    foreach ($account as $key => $value) {
      if (empty($value)) {
        unset($account[$key]);
      }
    }

    if ($this->type == 'user') {
      $command = 'CreateAccount';
      $userTab = $this->parent->parent->getBaseObject();
      if ($userTab->attributesAccess['userPassword']->getClear() != '') {
        /* Password was changed so we have the clear text password */
        $account['password'] = $userTab->attributesAccess['userPassword']->getClear();
      } else {
        /* If we don’t, just give the hash */
        $account['password']      = '';
        $account['userPassword']  = $userTab->userPassword;
      }
    } else {
      $command = 'CreateGroup';
    }

    return $this->query($command, $account);
  }

  private function updateAccount ($infos)
  {
    /* Step 1 - We fill $account */
    $account = $this->getAccountArray($infos);

    /* Step 2 - We compute diff from PARTAGE data */
    $this->cacheAccount();
    foreach ($account as $key => $value) {
      if (isset($this->cachedAccount['account'][$key]) && ($this->cachedAccount['account'][$key] == $value)) {
        unset($account[$key]);
      } elseif (!isset($this->cachedAccount['account'][$key]) && empty($value)) {
        unset($account[$key]);
      }
    }
    /* If value of zimbraAccountStatus is not something we understand, do not touch it (example: closed) */
    if (isset($this->cachedAccount['account']['zimbraAccountStatus']) && !in_array($this->cachedAccount['account']['zimbraAccountStatus'], array('active', 'locked'))) {
      unset($account['zimbraAccountStatus']);
    }

    /* Step 3 - We send update order if diff is not empty */
    if (!empty($account)) {
      $account['name'] = $this->account_id;
      if ($this->type == 'user') {
        $command = 'ModifyAccount';
      } else {
        $command = 'ModifyGroup';
      }
      $answer = $this->query($command, $account);

      if ($answer === FALSE) {
        return FALSE;
      }
    }

    /* Step 4 - Call SetPassword for password change if needed */
    if ($this->type == 'user') {
      $userTab = $this->parent->parent->getBaseObject();
      if ($userTab->attributesAccess['userPassword']->getClear() != '') {
        $account = array(
          'name'      => $this->account_id,
          'password'  => $userTab->attributesAccess['userPassword']->getClear()
        );
        $answer = $this->query('SetPassword', $account);

        if ($answer === FALSE) {
          return FALSE;
        }
      }
    }

    return TRUE;
  }

  private function setAliases ($initialAliases, $aliases)
  {
    $remove = array_diff($initialAliases, $aliases);
    $add    = array_diff($aliases, $initialAliases);
    if ($this->type == 'user') {
      $command = 'RemoveAccountAlias';
    } else {
      $command = 'RemoveDistributionListAlias';
    }
    foreach ($remove as $alias) {
      $answer = $this->query($command, array('name' => $this->account_id, 'alias' => $alias));
      if ($answer === FALSE) {
        return FALSE;
      }
    }
    if ($this->type == 'user') {
      $command = 'AddAccountAlias';
    } else {
      $command = 'AddDistributionListAlias';
    }
    foreach ($add as $alias) {
      $answer = $this->query($command, array('name' => $this->account_id, 'alias' => $alias));
      if ($answer === FALSE) {
        return FALSE;
      }
    }
    return TRUE;
  }

  private function updateMembers ()
  {
    global $config;

    if (!in_array($this->type, array('group','sympaAlias'))) {
      return TRUE;
    }

    $this->cacheAccount();
    $oldmembers = array();
    if (isset($this->cachedAccount['account']['members'])) {
      $oldmembers = $this->cachedAccount['account']['members'];
    }
    $mainTab = $this->parent->parent->getBaseObject();
    $newmembers = array();
    if ($this->type == 'sympaAlias') {
      $newmember = '';
      /* We take the shortest mail to filter out -editor and other addresses */
      foreach ($mainTab->mail as $mail) {
        if (empty($newmember) || (strlen($mail) < strlen($newmember))) {
          $newmember = $mail;
        }
      }
      $newmembers = array($newmember);
    } elseif (isset($mainTab->member)) {
      $ldap = $config->get_ldap_link();
      $dns = $mainTab->member;
      foreach ($dns as $dn) {
        $ldap->cat($dn);
        $attrs = $ldap->fetch();
        if (isset($attrs['mail'][0])) {
          $newmembers[] = $attrs['mail'][0];
        }
      }
    } elseif (isset($mainTab->memberUid)) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $uids = $mainTab->memberUid;
      foreach ($uids as $uid) {
        $ldap->search('(&(objectClass=inetOrgPerson)(uid='.$uid.'))');
        if ($ldap->count() > 1) {
          msg_dialog::display(
            _('PARTAGE error'),
            sprintf(_('Several users have uid "%s". Ignoring this member.'), $uid),
            ERROR_DIALOG
          );
        } elseif ($ldap->count() == 1) {
          $attrs = $ldap->fetch();
          if (isset($attrs['mail'][0])) {
            $newmembers[] = $attrs['mail'][0];
          }
        }
      }
    }
    $remove = array_diff($oldmembers, $newmembers);
    $add    = array_diff($newmembers, $oldmembers);
    if (!empty($remove)) {
      $answer = $this->query('RemoveGroupMembers', array('name' => $this->account_id, 'members' => $remove));
      if ($answer === FALSE) {
        return FALSE;
      }
    }
    if (!empty($add)) {
      $answer = $this->query('AddGroupMembers', array('name' => $this->account_id, 'members' => $add));
      if ($answer === FALSE) {
        return FALSE;
      }
    }

    return TRUE;
  }

  public function updateMailbox ()
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

      $answer = $this->createAccount($infos);

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

        $account = array(
          'name'    => $this->initial_account_id,
          'newname' => $this->account_id
        );
        if ($this->type == 'user') {
          $command = 'RenameAccount';
        } else {
          $command = 'RenameGroup';
        }
        $answer = $this->query($command, $account);

        if ($answer === FALSE) {
          return FALSE;
        }

        if (!$this->updateAccount($infos)) {
          return FALSE;
        }
      } elseif (!$this->account_exists()) {
        $answer = $this->createAccount($infos);

        if ($answer === FALSE) {
          return FALSE;
        }
      } else {
        if (!$this->updateAccount($infos)) {
          return FALSE;
        }
      }
    }

    /* Handle aliases */
    if (isset($this->parent->attrs['gosaMailAlternateAddress'])) {
      $initialAliases = $this->parent->attributesAccess['gosaMailAlternateAddress']->getInitialValue();
      $aliases        = $this->parent->attributesAccess['gosaMailAlternateAddress']->getValue();
      if (!$this->setAliases($initialAliases, $aliases)) {
        return FALSE;
      }
    }

    if (!$this->updateMembers($infos)) {
      return FALSE;
    }

    return TRUE;
  }

  public function deleteMailbox ()
  {
    parent::deleteMailbox();
    $servers = static::getMailServers();
    $infos = $servers[$this->parent->gosaMailServer];
    $this->account_id = $this->initial_account_id;
    if ($this->account_exists()) {
      if ($this->type == 'user') {
        if ($infos['delete_type'] == 'delete') {
          $answer = $this->query('DeleteAccount', array('name' => $this->initial_account_id));
        } elseif ($infos['delete_type'] == 'disable') {
          $answer = $this->query('ModifyAccount', array('name' => $this->initial_account_id, 'zimbraAccountStatus' => 'closed'));
        } else {
          $answer = FALSE;
          $this->error = _('Invalid value in fdRenaterPartageServerDeletionType');
        }
      } else {
        $answer = $this->query('DeleteGroup', array('name' => $this->initial_account_id));
      }
      return ($answer !== FALSE);
    }
    return TRUE;
  }

  /*! \brief  Returns the used quota in MiB
      @return Integer Quota used.
   */
  public function getQuotaUsage ()
  {
    if ($this->cacheAccount()) {
      /* Partage sends quota in bytes */
      return $this->cachedAccount['account']['used'] / (1024 * 1024);
    }
    return parent::getQuotaUsage();
  }

  /*! \brief  Returns the quota restrictions in MiB.
      @return Integer Quota restrictions.
   */
  public function getQuota ($quotaValue)
  {
    if ($this->cacheAccount()) {
      /* Partage sends quota in bytes */
      return $this->cachedAccount['account']['quota'] / (1024 * 1024);
    }
    return $quotaValue;
  }

  public function additionalInformations ()
  {
    if ($this->cacheAccount()) {
      $lastlogin = _('Never');
      if (isset($this->cachedAccount['account']['zimbraLastLogonTimestamp'])) {
        $date = LdapGeneralizedTime::fromString($this->cachedAccount['account']['zimbraLastLogonTimestamp']);
        $date->setTimezone(timezone::getDefaultTimeZone());
        $lastlogin = $date->format('Y-m-d H:i:s');
      }
      return array(
        _('Last login')     => $lastlogin,
        _('Account status') => (isset($this->cachedAccount['account']['zimbraAccountStatus']) ? $this->cachedAccount['account']['zimbraAccountStatus'] : _('Unknown')),
      );
    }
    return array();
  }

  static public function get_server_list ()
  {
    global $config;
    $serverList = array();

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdRenaterPartageServer)(fdRenaterPartageServerMailDomain=*))',
                  array('cn', 'fdRenaterPartageServerUri', 'fdRenaterPartageServerMailDomain', 'fdRenaterPartageServerUserAgent', 'fdRenaterPartageServerDeletionType'));
    while ($attrs = $ldap->fetch()) {
      unset($attrs['fdRenaterPartageServerMailDomain']['count']);
      foreach ($attrs['fdRenaterPartageServerMailDomain'] as $domainAndKey) {
        list($domain, $key, $cosString) = explode(':', $domainAndKey, 3);
        $cosArray = explode(',', $cosString);
        foreach ($cosArray as $cos) {
          list ($cosName, $cosId) = explode('|', $cos);
          $deleteMode = (isset($attrs['fdRenaterPartageServerDeletionType'][0]) ? $attrs['fdRenaterPartageServerDeletionType'][0] : 'delete');
          $serverList[$attrs['cn'][0].' - '.$domain.' - '.$cosName] = array(
            'server_dn'   => $attrs['dn'],
            'user_agent'  => $attrs['fdRenaterPartageServerUserAgent'][0],
            'uri'         => $attrs['fdRenaterPartageServerUri'][0],
            'delete_type' => $deleteMode,
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
