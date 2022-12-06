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
  protected $enableGroupForwarding  = FALSE;
  protected $enableLocalOnly        = FALSE;
  protected $enableForwardOnly      = TRUE;

  protected $user_id  = '%MAIL%';
  protected $share_id = '%MAIL%';

  private $http_options = [];
  private $ssl_options = [];
  private $token;
  private $tokenTime;
  private $cachedAccount;

  /*! \brief  Connect to renater PARTAGE API and store the auth token
      @return Boolean True if connection was successful
   */
  public function connect (): bool
  {
      parent::connect();
    $infos = $this->getServerConfig();
    if ($infos === FALSE) {
      return FALSE;
    }

    // Verification if token exists and is expired, true if not expired.
    if ($this->verifTokenExpiration($infos['domain'])) {

      $partageConf = $this->getPartageBackendConfiguration();
      $allTokens = $partageConf['fdRenaterPartageConfToken'];
      unset($allTokens['count']);

      // Find the proper token for the assign domain.
      $dn = $infos['domain'];
      foreach ($allTokens as $token) {
        if (preg_grep("/^dn:$dn\|/", [$token])) {
          $this->token = $token;
        }
      }

      $ts = microtime(TRUE);
      $this->tokenTime  = $ts;

      $this->connected = TRUE;
      return TRUE;

      // token is expired or a new one is required.
    } else {

      $this->connected = FALSE;

      //Microtime is used instead of time function due to a peculiar bug while clicking on Apply.
      $ts = microtime(TRUE);
      $preauth = hash_hmac('sha1', $infos['domain'].'|'.$ts, $infos['key']);

      $request = [
        'domain'    => $infos['domain'],
        'timestamp' => $ts,
        'preauth'   => $preauth
      ];

      $response = $this->query('Auth', $request);

      if ($response === FALSE) {

        return FALSE;
      } elseif (!isset($response['Response']['token'])) {
        $this->error = _('Server did not return auth token');

        return FALSE;
      } else {

        $this->token  = $response['Response']['token'];

        // Save token within LDAP with domain and time.
        $this->registerNewTokenInformation('dn:'.$infos['domain'].'|token:'.$this->token);

        // Verify if the follow is actually benificial, triggers is_connected method!
        $this->tokenTime  = $ts;

        $this->connected  = TRUE;
        return TRUE;
      }
    }
  }

  private function query ($command, $post)
  {
    $infos = $this->getServerConfig();
    if ($infos === FALSE) {
      return FALSE;
    }

    if (is_array($post)) {
      $post = http_build_query($post);
      /* PARTAGE API does not support array[0]= syntax, it wants array[]= */
      $post = preg_replace('/%5B[0-9]+%5D/simU', '%5B%5D', $post);
    }

    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $command, '<b>MAIL: Command</b>');
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, htmlescape($post),
      '<b>MAIL: Query</b>');

    // performs the HTTP(S) POST
    $opts = [
      'http' => array_merge(
        [
          'method'      => 'POST',
          'user_agent'  => $infos['user_agent'],
          'header'      => 'Content-type: application/x-www-form-urlencoded'."\r\n"
                          .'Content-Length: '.strlen($post)."\r\n",
          'content'     => $post
        ],
        $this->http_options
      ),
      'ssl' => $this->ssl_options
    ];

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
    $errors = parent::check();
    $infos  = $this->getServerConfig();
    if ($infos === FALSE) {
      return FALSE;
    }

    $alternates = $this->parent->gosaMailAlternateAddress;
    foreach ($alternates as $alternate) {
      if (!preg_match('/@'.preg_quote($infos['domain'], '/').'$/', $alternate)) {
        $errors[] = new SimplePluginCheckError(
          $this->parent,
          htmlescape(sprintf(_('Email address "%s" does not use the correct partage domain "%s"'), $alternate, $infos['domain']))
        );
      }
    }

    return $errors;
  }

  /*! \brief  Returns the connection status of this method.
      @return Boolean True if this method is connected else false.
   */
  public function is_connected ()
  {
    /* If the token is older than 5 minutes it will get rejected. 5min = 300sec */
    return (parent::is_connected() && (time() - $this->tokenTime < 299));
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
    $answer = $this->query($command, ['name' => $this->account_id]);
    if (($answer !== FALSE) && isset($answer['Response'][$answerkey])) {
      $this->cachedAccount = [
        'account' => $answer['Response'][$answerkey],
        'id'      => $this->account_id,
        'time'    => time()
      ];
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
    $keys = ['members', 'zimbraMailAlias', 'zimbraAllowFromAddress'];
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
    $account = [
      'name'                                => $this->account_id,
    ];
    if ($this->type == 'user') {
      $account['zimbraCOSId']                         = $infos['cos_id'];
      if (is_numeric($this->parent->gosaMailQuota)) {
        $account['zimbraMailQuota']                   = $this->parent->gosaMailQuota * 1024 * 1024;
      }
      $account['zimbraPrefMailLocalDeliveryDisabled'] = ($this->parent->flag_ownmailbox ? 'TRUE' : 'FALSE');
      if (!empty($this->parent->gosaMailForwardingAddress)) {
        $count = count($this->parent->gosaMailForwardingAddress);
        $address = $this->parent->gosaMailForwardingAddress[0];
        if ($count > 1) {
          $warning = new FusionDirectoryWarning(
            htmlescape(sprintf(
              _('Several addresses were given in gosaMailForwardingAddress but only one is supported by PARTAGE. %s will be sent to PARTAGE.'),
              $address
            ))
          );
          $warning->display();
        }
        $account['zimbraPrefMailForwardingAddress'] = $address;
      }
    }

    if ($this->type == 'user') {
      $mainFields = [
        'sn', 'givenName', 'description',
        'l', 'st',
        'telephoneNumber', 'mobile', 'pager', 'facsimileTelephoneNumber',
        'displayName', 'homePhone',
        'title', 'ou',
        'street', 'postalCode', 'postOfficeBox'
      ];
    } else {
      $mainFields = [
        'description'
      ];
    }
    $mainTab = $this->parent->parent->getBaseObject();
    foreach ($mainFields as $field) {
      if (isset($mainTab->$field)) {
        $account[$field] = $mainTab->$field;
      }
    }

    if ($this->type == 'user') {
      $userMatchingFields = [
        'o' => 'company',
        'roomNumber' => 'physicalDeliveryOfficeName'
      ];
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
        $partageFields = [
          'displayName' => 'displayName',
          'fdRenaterPartageGroupMailStatus' => 'zimbraMailStatus',
          'fdRenaterPartageGroupNotes' => 'zimbraNotes'
        ];
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
    if (isset($this->cachedAccount['account']['zimbraAccountStatus']) && !in_array($this->cachedAccount['account']['zimbraAccountStatus'], ['active', 'locked'])) {
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
        $account = [
          'name'      => $this->account_id,
          'password'  => $userTab->attributesAccess['userPassword']->getClear()
        ];
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
      $answer = $this->query($command, ['name' => $this->account_id, 'alias' => $alias]);
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
      $answer = $this->query($command, ['name' => $this->account_id, 'alias' => $alias]);
      if ($answer === FALSE) {
        return FALSE;
      }
    }
    return TRUE;
  }

  private function updateMembers ()
  {
    global $config;

    if (!in_array($this->type, ['group','sympaAlias'])) {
      return TRUE;
    }

    $this->cacheAccount();
    $oldmembers = [];
    if (isset($this->cachedAccount['account']['members'])) {
      $oldmembers = $this->cachedAccount['account']['members'];
    }
    $mainTab = $this->parent->parent->getBaseObject();
    $newmembers = [];
    if ($this->type == 'sympaAlias') {
      $newmember = '';
      /* We take the shortest mail to filter out -editor and other addresses */
      foreach ($mainTab->mail as $mail) {
        if (empty($newmember) || (strlen($mail) < strlen($newmember))) {
          $newmember = $mail;
        }
      }
      $newmembers = [$newmember];
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
          $error = new FusionDirectoryError(
            htmlescape(sprintf(
              _('Several users have uid "%s". Ignoring this member.'),
              $uid
            ))
          );
          $error->display();
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
      $answer = $this->query('RemoveGroupMembers', ['name' => $this->account_id, 'members' => $remove]);
      if ($answer === FALSE) {
        return FALSE;
      }
    }
    if (!empty($add)) {
      $answer = $this->query('AddGroupMembers', ['name' => $this->account_id, 'members' => $add]);
      if ($answer === FALSE) {
        return FALSE;
      }
    }

    return TRUE;
  }

  /*
   * Allows simple verification if the account is in another status than active.
   */
  public function isAccountNotActive () : BOOL
  {
    if (!in_array($this->cachedAccount['account']['zimbraAccountStatus'], ['active'])) {
      return TRUE;
    }
    return FALSE;
  }

  /*
   * Modifies the status of the processed account to active.
   */
  public function setActiveStatus () : BOOL
  {
    $this->account_id = $this->initial_account_id;

    if ($this->account_exists()) {
      if ($this->type == 'user') {
        $answer = $this->query('ModifyAccount', ['name' => $this->initial_account_id, 'zimbraAccountStatus' => 'active']);
        return ($answer !== FALSE);
      }
    }
    return FALSE;
  }

  /*
   * Retrieve backend configuration attributes values
   */
  public function getPartageBackendConfiguration ()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdRenaterPartageConf))');
    $attrs = $ldap->fetch();

    // This contains a full array of the backend configuration attributes.
    return $attrs;
  }

  /*
   * Verify the backend config if the non active account can be re-activated to updates Partage.
   */
  public function canBeActivated () : BOOL
  {
    $result = $this->getPartageBackendConfiguration();

    if ($result['fdRenaterPartageConfActivateAccount'][0] === 'TRUE') {
      return TRUE;
    }

    return FALSE;
  }


  /*
   * Verify the backend config if LDAP can be trusted and pushes its updates to Partage.
   */
  public function ldapOverridePartage () : BOOL
  {
    $result = $this->getPartageBackendConfiguration();

    if ($result['fdRenaterPartageConfOverride'][0] === 'TRUE') {
      return TRUE;
    }

    return FALSE;
  }

  /*
   * Register the new information about the access token received.
   * Can return an LDAP error.
   */
  public function registerNewTokenInformation (string $newToken)
  {
    $partageConf = $this->getPartageBackendConfiguration();
    $tokenLifetime = $partageConf['fdRenaterPartageConfTokenLifetime'][0];

    // strtotime added time will be replace by the seconds from backend configuration.
    $dateToken = date('Y-m-d-H-i-s', strtotime('+'.$tokenLifetime.' sec'));
    $dateEx = explode('-', $dateToken);
    $dateToken = implode("", $dateEx);

    $finalToken = $newToken.'|date:'.$dateToken;

    if (isset($partageConf['fdRenaterPartageConfToken']) && !empty($partageConf['fdRenaterPartageConfToken'])) {
      $existingTokens = $partageConf['fdRenaterPartageConfToken'];
      unset($existingTokens['count']);
    }

    // Push the new token within existing ones.
    $existingTokens[] = $finalToken;
    $attrs = ['fdRenaterPartageConfToken' => $existingTokens];

    $this->modifyTokenInLdap($attrs, TRUE);
  }

  /*
   * Updates or Delete tokens within LDAP base on time expiry
   * Args takes an associative key/value array and a mode. TRUE to modify, FALSE to delete.
   */
  public function modifyTokenInLdap (array $attrs, bool $mode)
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $dn = 'cn=config,'.$config->data['OBJECTS']['CONFIGURATION']['ou'].$config->current['BASE'];
    $ldap->cd($dn);

    if ($mode === TRUE) {
      $ldap->modify($attrs);
    } else {
      $ldap->rm($attrs);
    }

    if (!$ldap->success()) {
      $error = new FusionDirectoryLdapError($dn, LDAP_MOD, $ldap->get_error(), $ldap->get_errno());
      $error->display();
      return NULL;
    }

  }

   /*
   * Verification if the access-token expiry time is expired and supress it from LDAP if expired.
   * and a previous token was set. (In case no token was ever received).
   */
  public function verifTokenExpiration (string $domain) : BOOL
  {
    // retrieve the previous token set within conf.
    $partageConf = $this->getPartageBackendConfiguration();

    // If no previous token was ever registered for that domain, return FALSE.
    if (isset($partageConf['fdRenaterPartageConfToken']) && !empty($partageConf['fdRenaterPartageConfToken'])) {

      $allTokens = $partageConf['fdRenaterPartageConfToken'];
      unset($allTokens['count']);
    } else {

      return FALSE;
    }

    // Find the proper token for the assign domain.
    foreach ($allTokens as $token) {
      if (preg_grep("/^dn:$domain\|/", [$token])) {
        $validToken = $token;
      }
    }

    // FALSE if token is non existent for that domain. Creation required.
    if (isset($validToken) && !empty($validToken)) {
      preg_match('/\|date:(\d*)/', $validToken, $match);
      $tokenRecTime = $match[1];

    } else {
      return FALSE;
    }

    $date = (new DateTime)->format('Y-m-d-H-i-s');
    $dateEx = explode('-', $date);
    $dateStringerized = implode("", $dateEx);

    // Token is not expired on TRUE, delete from LDAP on FALSE.
    if ($tokenRecTime > $dateStringerized) {

      return TRUE;
    } else {

      $attrs = ['fdRenaterPartageConfToken' => $validToken];
      $this->modifyTokenInLdap($attrs, FALSE);

      return FALSE;
    }
  }


  /*
   * This method will create and update mailbox.
   * In case the mailbox exists within Renater, it might be updated.
   * Depending on the flags in the backend configuration.
   */
  public function updateMailbox ()
  {
    parent::updateMailbox();
    $infos = $this->getServerConfig();
    if ($infos === FALSE) {
      return FALSE;
    }

    $this->build_account_id();
    if (!$this->parent->initially_was_account) {

      if ($this->account_exists()) {
        // Switch from creation to Update logic and verify if LDAP can overidde partage.
        if (!$this->ldapOverridePartage()) {
          $this->error = _('This account already exists and is not allowed to be updated !');

          return FALSE;
        }
        // Verify if the account is in active state and can be activated if not.
        if ($this->isAccountNotActive()) {
          if ($this->canBeActivated()) {

            // Activate the account
            if (!$this->setActiveStatus()) {
              $this->error = _('This account was not set into active state for some reasons...');

              return FALSE;
            }
            // As the account can be activated and updated -> update Partage.
            $answer = $this->updateAccount($infos);

            if ($answer === FALSE) {
              return FALSE;
            }

            return TRUE;

          } else {

            // Account can be updated but not set to active state.
            $this->error = _('This account is not in active status, activate first !');

            return FALSE;
          }
        }
        // Update the account as LDAP override is ticked and account is Active.
        $answer = $this->updateAccount($infos);

        if ($answer === FALSE) {
          return FALSE;
        }

        return TRUE;

      } else {

        // Continue the creation of the account as it does not exist yet.
        $answer = $this->createAccount($infos);
        if ($answer === FALSE) {
          return FALSE;
        }

        return TRUE;
      }

    } else {
      // This means a modification of e-mails address which is not allowed.
      if ($this->account_id !== $this->initial_account_id) {
        $this->error = _('Attempt to rename address-email or e-mail address already exist !');

        return FALSE;
      }

      // Update the account information (username / lastname ) as account exists and email did not change.
      if (!$this->updateAccount($infos)) {
        return FALSE;
      }

      /* Handle aliases */
      if (isset($this->parent->attrs['gosaMailAlternateAddress'])) {
        $initialAliases = $this->parent->attributesAccess['gosaMailAlternateAddress']->getInitialValue();
        $aliases        = $this->parent->attributesAccess['gosaMailAlternateAddress']->getValue();
        if (!$this->setAliases($initialAliases, $aliases)) {
          return FALSE;
        }
      }

      if (!$this->updateMembers()) {
        return FALSE;
      }

      return TRUE;
    }
  }

  public function deleteMailbox ()
  {
    parent::deleteMailbox();

    $deletion_mode = $this->getPartageBackendConfiguration();
    $deletion_mode = $deletion_mode['fdRenaterPartageConfDeleteOption'][0];

    $infos = $this->getServerConfig();
    if ($infos === FALSE) {
      return FALSE;
    }

    $this->account_id = $this->initial_account_id;
    if ($this->account_exists()) {
      if ($this->type == 'user') {
        if ($deletion_mode == 'delete') {

          $answer = $this->query('DeleteAccount', ['name' => $this->initial_account_id]);

        } elseif ($deletion_mode == 'disable') {
          $answer = $this->query('ModifyAccount', ['name' => $this->initial_account_id, 'zimbraAccountStatus' => 'closed']);

        } else {
          $answer = FALSE;
          $this->error = _('Invalid value in fdRenaterPartageServerDeletionType');
        }
      } else {
        $answer = $this->query('DeleteGroup', ['name' => $this->initial_account_id]);
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
      return (int)($this->cachedAccount['account']['quota'] / (1024 * 1024));
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
      return [
        _('Last login')     => $lastlogin,
        _('Account status') => (isset($this->cachedAccount['account']['zimbraAccountStatus']) ? $this->cachedAccount['account']['zimbraAccountStatus'] : _('Unknown')),
      ];
    }
    return [];
  }

  static public function get_server_list ()
  {
    global $config;
    $serverList = [];

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdRenaterPartageServer)(fdRenaterPartageServerMailDomain=*))',
                  ['cn', 'fdRenaterPartageServerUri', 'fdRenaterPartageServerMailDomain', 'fdRenaterPartageServerUserAgent']);
    while ($attrs = $ldap->fetch()) {
      unset($attrs['fdRenaterPartageServerMailDomain']['count']);
      foreach ($attrs['fdRenaterPartageServerMailDomain'] as $domainAndKey) {
        list($domain, $key, $cosString) = explode(':', $domainAndKey, 3);
        $cosArray = explode(',', $cosString);
        foreach ($cosArray as $cos) {
          list ($cosName, $cosId) = explode('|', $cos);
          $serverList[$attrs['cn'][0].' - '.$domain.' - '.$cosName] = [
            'server_dn'   => $attrs['dn'],
            'user_agent'  => $attrs['fdRenaterPartageServerUserAgent'][0],
            'uri'         => $attrs['fdRenaterPartageServerUri'][0],
            'domain'      => $domain,
            'key'         => $key,
            'cos_name'    => $cosName,
            'cos_id'      => $cosId
          ];
        }
      }
    }

    return $serverList;
  }
}
