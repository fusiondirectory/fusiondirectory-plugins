<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2020-2021  FusionDirectory

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

namespace ZimbraSoap;

/*
 * Missing:
 * GetGroup (getDistributionListRequest)
 * CreateGroup
 * ModifyGroup
 * RemoveDistributionListAlias
 * AddDistributionListAlias
 * RemoveGroupMembers
 * AddGroupMembers
 * RenameGroup
 * DeleteGroup
 */

class Request
{
  public static function getMapping (): array
  {
    return [
      'getAccountRequest'           => 'ZimbraSoap\\getAccountRequest',
      'getAccountResponse'          => 'ZimbraSoap\\genericAccountResponse',
      'authResponse'                => 'ZimbraSoap\\authResponse',
      'createAccountRequest'        => 'ZimbraSoap\\createAccountRequest',
      'createAccountResponse'       => 'ZimbraSoap\\genericAccountResponse',
      'modifyAccountRequest'        => 'ZimbraSoap\\modifyAccountRequest',
      'modifyAccountResponse'       => 'ZimbraSoap\\genericAccountResponse',
      'setPasswordRequest'          => 'ZimbraSoap\\setPasswordRequest',
      'setPasswordResponse'         => 'ZimbraSoap\\setPasswordResponse',
      'renameAccountRequest'        => 'ZimbraSoap\\renameAccountRequest',
      'renameAccountResponse'       => 'ZimbraSoap\\genericAccountResponse',
      'deleteAccountRequest'        => 'ZimbraSoap\\deleteAccountRequest',
      'deleteAccountResponse'       => 'ZimbraSoap\\emptyResponse',
      'addAccountAliasRequest'      => 'ZimbraSoap\\addAccountAliasRequest',
      'addAccountAliasResponse'     => 'ZimbraSoap\\emptyResponse',
      'removeAccountAliasRequest'   => 'ZimbraSoap\\removeAccountAliasRequest',
      'removeAccountAliasResponse'  => 'ZimbraSoap\\emptyResponse',
      'getMailboxRequest'           => 'ZimbraSoap\\getMailboxRequest',
      'getMailboxResponse'          => 'ZimbraSoap\\getMailboxResponse',
      'endSessionRequest'           => 'ZimbraSoap\\endSessionRequest',
      'endSessionResponse'          => 'ZimbraSoap\\emptyResponse',
      'getDistributionListRequest'  => 'ZimbraSoap\\getDistributionListRequest',
      'getDistributionListResponse' => 'ZimbraSoap\\getDistributionListResponse',
    ];
  }

  public function soapCall (\SoapClient $client): Response
  {
    return $client->__soapCall((new \ReflectionClass($this))->getShortName(), [new \SoapVar($this, SOAP_ENC_OBJECT)]);
  }

  /* Build request object from PARTAGE command and data array */
  static public function build (string $command, array $data): Request
  {
    switch ($command) {
      case 'Auth':
        return new authRequest($data);
      case 'CreateAccount':
        return new createAccountRequest($data);
      case 'ModifyAccount':
        return new modifyAccountRequest($data);
      case 'GetMailbox':
        return new getMailboxRequest($data);
      case 'GetAccount':
        return new getAccountRequest($data);
      case 'SetPassword':
        return new setPasswordRequest($data);
      case 'RenameAccount':
        return new renameAccountRequest($data);
      case 'DeleteAccount':
        return new deleteAccountRequest($data);
      case 'AddAccountAlias':
        return new addAccountAliasRequest($data);
      case 'RemoveAccountAlias':
        return new removeAccountAliasRequest($data);
      case 'EndSession':
        return new endSessionRequest($data);
      case 'GetGroup':
        return new getDistributionListRequest($data);
      default:
        throw new FusionDirectoryException('Unsupported command '.$command);
    }
  }

  static protected function attributesListFromArray ($a): array
  {
    $data = [];
    foreach ($a as $k => $v) {
      if (is_array($v)) {
        $values = $v;
      } else {
        $values = [$v];
      }
      foreach ($values as $value) {
        $o = new \stdClass();
        $o->n = $k;
        $o->_ = $value;
        $data[] = $o;
      }
    }
    return $data;
  }
}

abstract class Response
{
  public abstract function toArray (): array;

  static protected function attributesListToArray ($a): array
  {
    $data = [];
    foreach ($a as $n) {
      if (isset($data[$n->n])) {
        if (!is_array($data[$n->n])) {
          $data[$n->n] = [$data[$n->n]];
        }
        $data[$n->n][] = $n->_;
      } else {
        $data[$n->n] = $n->_;
      }
    }
    return $data;
  }
}

class emptyResponse extends Response
{
  public function toArray (): array
  {
    return [];
  }
}

class authRequest extends Request {
  /* authRequest do not work through a class, use an array */
  public $data;

  public function __construct (array $data)
  {
    $this->data = $data;
  }

  public function soapCall (\SoapClient $client): Response
  {
    return $client->__soapCall((new \ReflectionClass($this))->getShortName(), [$this->data]);
  }
}

class authResponse extends Response {
  public $authToken;
  public $lifetime;

  public function toArray (): array
  {
    return ['token' => $this->authToken, 'lifetime' => $this->lifetime];
  }
}

class getAccountRequest extends Request {
  public $account;
  public $applyCos;
  public $attrs;

  public function __construct (array $data)
  {
    $this->account = new \stdClass();
    $this->account->by = 'name';
    $this->account->_ = $data['name'];
    $this->applyCos = FALSE;
    $this->attrs = $data['attrs'] ?? NULL;
  }
}

class genericAccountResponse extends Response {
  public $account;

  public function toArray (): array
  {
    if (!is_array($this->account->a)) {
      $this->account->a = [$this->account->a];
    }

    return ['account' => static::attributesListToArray($this->account->a)];
  }
}

class createAccountRequest extends Request {
  public $name;
  public $password;
  public $a;

  public function __construct (array $data)
  {
    $this->name      = ($data['name'] ?? NULL);
    $this->password  = ($data['password'] ?? NULL);
    unset($data['name']);
    unset($data['password']);
    $this->a         = static::attributesListFromArray($data);
  }
}

class modifyAccountRequest extends Request {
  /* string */
  public $id;
  public $a;

  public function __construct (array $data)
  {
    $this->id = ($data['zimbraId'] ?? NULL);
    unset($data['zimbraId']);
    $this->a  = static::attributesListFromArray($data);
  }
}

class setPasswordRequest extends Request {
  /* string */
  public $id;
  /* string */
  public $newPassword;

  public function __construct (array $data)
  {
    $this->id           = ($data['zimbraId'] ?? NULL);
    $this->newPassword  = ($data['password'] ?? NULL);
  }
}

class setPasswordResponse extends Response {
  /* string */
  public $message;

  public function toArray (): array
  {
    return ['message' => $this->message];
  }
}

class renameAccountRequest extends Request {
  /* string */
  public $id;
  /* string */
  public $newName;

  public function __construct (array $data)
  {
    $this->id       = ($data['zimbraId'] ?? NULL);
    $this->newName  = ($data['newName'] ?? NULL);
  }
}

class deleteAccountRequest extends Request {
  /* string */
  public $id;

  public function __construct (array $data)
  {
    $this->id = ($data['zimbraId'] ?? NULL);
  }
}

class addAccountAliasRequest extends Request {
  /* string */
  public $id;
  /* string */
  public $alias;

  public function __construct (array $data)
  {
    $this->id     = ($data['zimbraId'] ?? NULL);
    $this->alias  = ($data['alias'] ?? NULL);
  }
}

class removeAccountAliasRequest extends addAccountAliasRequest {
}

class getMailboxRequest extends Request {
  public $mbox;

  public function __construct (array $data)
  {
    $this->mbox  = ['id' => ($data['zimbraId'] ?? NULL)];
  }
}

class getMailboxResponse extends Response {
  public $mbox;

  public function toArray (): array
  {
    return [
      'id'    => $this->mbox->mbxid,
      'usage' => $this->mbox->s,
    ];
  }
}

class endSessionRequest extends Request {
  /* boolean */
  public $logoff;
  /* boolean */
  public $all;
  /* boolean */
  public $excludeCurrent;
  /* string */
  public $sessionId;

  public function __construct (array $data)
  {
    $this->logoff         = ($data['logoff'] ?? NULL);
    $this->all            = ($data['all'] ?? NULL);
    $this->excludeCurrent = ($data['excludeCurrent'] ?? NULL);
    $this->sessionId      = ($data['sessionId'] ?? NULL);
  }
}

class getDistributionListRequest extends Request {
  /* distributionListSelector */
  public $dl;
  /* int  */
  public $limit;
  /* int  */
  public $offset;
  /* boolean  */
  public $sortAscending;

  public function __construct (array $data)
  {
    $this->dl = new \stdClass();
    $this->dl->by = 'name';
    $this->dl->_ = $data['name'];
    $this->limit          = ($data['limit'] ?? NULL);
    $this->offset         = ($data['offset'] ?? NULL);
    $this->sortAscending  = ($data['sortAscending'] ?? NULL);
  }
}

class getDistributionListResponse extends Response {
  /* distributionListInfo */
  public $dl;
  /* boolean */
  public $more;
  /* int */
  public $total;

  public function toArray (): array
  {
    return [
      'dl'    => $this->dl,
      'more'  => $this->more,
      'total' => $this->total,
    ];
  }
}
