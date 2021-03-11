<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2021  FusionDirectory

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

require_once(__DIR__ . '/zimbraSoap.php');

class mailMethodZimbra extends mailMethod
{
  protected $enableQuota            = TRUE;
  protected $enableVacation         = FALSE;
  protected $enableGroupForwarding  = FALSE;
  protected $enableLocalOnly        = FALSE;
  protected $enableForwardOnly      = TRUE;

  protected $user_id  = '%MAIL%';
  protected $share_id = '%MAIL%';

  private $soapClient;
  protected $token;
  protected $tokenTime;
  protected $tokenLifetime;
  protected $cachedAccount;

  /*! \brief  Connect to Zimbra API and store the auth token
      @return Boolean True if connection was successful
   */
  public function connect ()
  {
    parent::connect();
    $servers = static::getMailServers();
    $infos = $servers[$this->parent->gosaMailServer];

    $this->connected = FALSE;

    $ts = time();
    $this->soapClient = new SoapClient(
      $infos['uri'].'/service/wsdl/ZimbraAdminService.wsdl',
      [
        'location'      => $infos['uri'].':7071/service/admin/soap',
        'trace'         => TRUE,
        'exceptions'    => TRUE,
        'classmap'     => ZimbraSoap\Request::getMapping(),
      ]
    );

    $request = [
      'name'      => $infos['login'],
      'password'  => $infos['password'],
    ];

    $response = $this->query('Auth', $request);

    if ($response === FALSE) {
      return FALSE;
    } elseif (!isset($response['token'])) {
      $this->error = _('Server did not return auth token');
      return FALSE;
    } else {
      $this->token          = $response['token'];
      $this->tokenLifetime  = $response['lifetime'];
      $this->tokenTime      = $ts;
      $this->connected      = TRUE;
      return TRUE;
    }
  }

  protected function query (string $command, $data)
  {
    $request = ZimbraSoap\Request::build($command, $data);

    try {
      $response = $request->soapCall($this->soapClient)->toArray();
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, htmlescape($this->soapClient->__getLastRequest() ?? ''), '<b>MAIL: Query</b>');
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, htmlescape($this->soapClient->__getLastResponse() ?? ''), '<b>MAIL: Answer</b>');
      return $response;
    } catch (SoapFault $sf) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, htmlescape($this->soapClient->__getLastRequest() ?? ''), '<b>MAIL: Query</b>');
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, htmlescape($this->soapClient->__getLastResponse() ?? ''), '<b>MAIL: Answer</b>');
      $this->error = sprintf(_('Error while contacting Zimbra API: %s'), $sf->getMessage());
      return FALSE;
    }
  }

  /*! \brief  Disconnect from the API (deletes the auth token)
   */
  public function disconnect ()
  {
    parent::disconnect();
    $this->query('EndSession', []);
    unset($this->token);
    unset($this->tokenTime);
    unset($this->tokenLifetime);
  }

  /*! \brief Checks data consistency
  */
  public function check ()
  {
    $errors   = parent::check();
    $servers  = static::getMailServers();
    $infos    = $servers[$this->parent->gosaMailServer];

    $alternates = $this->parent->gosaMailAlternateAddress;
    foreach ($alternates as $alternate) {
      if (!preg_match('/@'.preg_quote($infos['domain'], '/').'$/', $alternate)) {
        $errors[] = new SimplePluginCheckError(
          $this->parent,
          htmlescape(sprintf(_('Email address "%s" does not use the correct domain "%s"'), $alternate, $infos['domain']))
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
    /* If the token is older than its lifetime it will get rejected */
    return (parent::is_connected() && (time() - $this->tokenTime < $this->tokenLifetime));
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
    if (($answer !== FALSE) && isset($answer[$answerkey])) {
      $this->cachedAccount = [
        'account'   => $answer[$answerkey],
        'id'        => $this->account_id,
        'zimbraId'  => $answer[$answerkey]['zimbraId'],
        'time'      => time()
      ];
      $this->cleanCachedAccountArrays();
      return TRUE;
    } else {
      return FALSE;
    }
  }

  /*
   * */
  private function cleanCachedAccountArrays ()
  {
  }

  private function getAccountArray ($infos)
  {
    $account = [
      'name' => $this->account_id,
    ];
    if ($this->type == 'user') {
      $account['zimbraCOSId']                         = $infos['cos_id'];
      $account['zimbraMailQuota']                     = $this->parent->gosaMailQuota * 1024 * 1024;
      $account['zimbraPrefMailLocalDeliveryDisabled'] = ($this->parent->flag_ownmailbox ? 'TRUE' : 'FALSE');
      if (!empty($this->parent->gosaMailForwardingAddress)) {
        $count = count($this->parent->gosaMailForwardingAddress);
        $address = $this->parent->gosaMailForwardingAddress[0];
        if ($count > 1) {
          /* TODO - check this is true for Zimbra */
          $warning = new FusionDirectoryWarning(
            htmlescape(sprintf(
              _('Several addresses were given in gosaMailForwardingAddress but only one is supported by Zimbra. %s will be sent to Zimbra.'),
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
    unset($account['name']);

    /* Step 2 - We compute diff from server data */
    $this->cacheAccount();
    foreach ($account as $key => $value) {
      if (isset($this->cachedAccount['account'][$key])) {
        if (is_array($value) && !is_array($this->cachedAccount['account'][$key])) {
          $this->cachedAccount['account'][$key] = [$this->cachedAccount['account'][$key]];
        }
        if ($this->cachedAccount['account'][$key] == $value) {
          unset($account[$key]);
        }
      } elseif (empty($value)) {
        unset($account[$key]);
      }
    }
    /* If value of zimbraAccountStatus is not something we understand, do not touch it (example: closed) */
    if (isset($this->cachedAccount['account']['zimbraAccountStatus']) && !in_array($this->cachedAccount['account']['zimbraAccountStatus'], ['active', 'locked'])) {
      unset($account['zimbraAccountStatus']);
    }

    /* Step 3 - We send update order if diff is not empty */
    if (!empty($account)) {
      $account['zimbraId'] = $this->cachedAccount['zimbraId'];
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
          'zimbraId'  => $this->cachedAccount['zimbraId'],
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
      $answer = $this->query($command, ['zimbraId' => $this->cachedAccount['zimbraId'], 'alias' => $alias]);
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
      $answer = $this->query($command, ['zimbraId' => $this->cachedAccount['zimbraId'], 'alias' => $alias]);
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
      $answer = $this->query('RemoveGroupMembers', ['zimbraId' => $this->cachedAccount['zimbraId'], 'members' => $remove]);
      if ($answer === FALSE) {
        return FALSE;
      }
    }
    if (!empty($add)) {
      $answer = $this->query('AddGroupMembers', ['zimbraId' => $this->cachedAccount['zimbraId'], 'members' => $add]);
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
        $this->error = _('An account with the same email address already exists in the email server');
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
          $this->error = _('An account with the same email address already exists in the email server');
          return FALSE;
        }

        $account = [
          'zimbraId'  => $this->cachedAccount['zimbraId'],
          'newName'   => $this->account_id,
        ];
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

    if (!$this->updateMembers()) {
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
          $answer = $this->query('DeleteAccount', ['zimbraId' => $this->cachedAccount['zimbraId']]);
        } elseif ($infos['delete_type'] == 'disable') {
          $answer = $this->query('ModifyAccount', ['zimbraId' => $this->cachedAccount['zimbraId'], 'zimbraAccountStatus' => 'closed']);
        } else {
          $answer = FALSE;
          $this->error = _('Invalid deletion mode');
        }
      } else {
        $answer = $this->query('DeleteGroup', ['zimbraId' => $this->cachedAccount['zimbraId']]);
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
      if (!isset($this->cachedAccount['used'])) {
        $answer = $this->query('GetMailbox', ['zimbraId' => $this->cachedAccount['zimbraId']]);
        if ($answer === FALSE) {
          return parent::getQuotaUsage();
        }
        $this->cachedAccount['used'] = $answer['usage'];
      }
      /* Server sends quota in bytes */
      return $this->cachedAccount['used'] / (1024 * 1024);
    }
    return parent::getQuotaUsage();
  }

  /*! \brief  Returns the quota restrictions in MiB.
      @return Integer Quota restrictions.
   */
  public function getQuota ($quotaValue)
  {
    if ($this->cacheAccount()) {
      /* Server sends quota in bytes */
      return (int)($this->cachedAccount['account']['zimbraMailQuota'] / (1024 * 1024));
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
    $ldap->search('(&(objectClass=fdZimbraServer)(fdZimbraServerMailDomain=*))',
                  ['cn', 'fdZimbraServerUri', 'fdZimbraServerMailDomain',
                  'fdZimbraServerUserAgent', 'fdZimbraServerDeletionType',
                  'fdZimbraServerLogin', 'fdZimbraServerPassword']);
    while ($attrs = $ldap->fetch()) {
      unset($attrs['fdZimbraServerMailDomain']['count']);
      foreach ($attrs['fdZimbraServerMailDomain'] as $domainAndCos) {
        list($domain, $cosString) = explode(':', $domainAndCos, 3);
        $cosArray = explode(',', $cosString);
        foreach ($cosArray as $cos) {
          list ($cosName, $cosId) = explode('|', $cos);
          $deleteMode = (isset($attrs['fdZimbraServerDeletionType'][0]) ? $attrs['fdZimbraServerDeletionType'][0] : 'delete');
          $serverList[$attrs['cn'][0].' - '.$domain.' - '.$cosName] = [
            'server_dn'   => $attrs['dn'],
            'user_agent'  => $attrs['fdZimbraServerUserAgent'][0],
            'uri'         => $attrs['fdZimbraServerUri'][0],
            'login'       => $attrs['fdZimbraServerLogin'][0],
            'password'    => $attrs['fdZimbraServerPassword'][0],
            'delete_type' => $deleteMode,
            'domain'      => $domain,
            'cos_name'    => $cosName,
            'cos_id'      => $cosId
          ];
        }
      }
    }

    return $serverList;
  }
}