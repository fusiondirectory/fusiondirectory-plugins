<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2019  FusionDirectory

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

/*!
 * \file rest.php
 * \brief This file is a REST API for FusionDirectory
 *
 */

ini_set('session.use_cookies', 0);
ini_set('session.use_only_cookies', 1);

require_once("../include/php_setup.inc");
require_once("functions.inc");
require_once("variables.inc");
require_once("webservice/class_fdRPCService.inc");

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Headers: Content-Type, Session-Token, Authorization");
header("Access-Control-Allow-Methods: GET, POST, DELETE, PUT");

/*!
 * \brief This class is the JSON-RPC webservice of FusionDirectory
 *
 * It must be served through jsonRPCServer::handle
 * */
class fdRestService extends fdRPCService
{
  public function treatRequest ()
  {
    global $_SERVER;

    // Get the HTTP method, path and body of the request
    $method   = $_SERVER['REQUEST_METHOD'];
    $request  = explode('/', trim($_SERVER['PATH_INFO'], '/'));
    $input    = json_decode(file_get_contents('php://input'), TRUE);

    if ($request[0] == 'login') {
      /* Login method have the following parameters: LDAP, user, password */
      static::initiateRPCSession(
        NULL,
        ($input['ldap'] ?? $_GET['ldap'] ?? NULL),
        ($input['user'] ?? $_GET['user'] ?? ''),
        ($input['password'] ?? $_GET['password'] ?? '')
      );
      $request  = ['token'];
      $method   = 'GET';
    } elseif ($request[0] != 'ldaps') {
      static::initiateRPCSession($_SERVER['HTTP_SESSION_TOKEN']);
    }

    if (count($request) == 0) {
      http_response_code(400);
      die('No request');
    }

    try {
      $endpoint = 'endpoint_'.array_shift($request).'_'.$method.'_'.count($request);
      $result   = $this->$endpoint($input, ...$request);

      echo json_encode($result)."\n";
    } catch (Exception $e) {
      http_response_code(400);
      die('Error: '.$e->getMessage());
    }
  }

  public function __call ($method, $params)
  {
    if (preg_match('/^endpoint_([^_]+)_([^_]+)_([^_]+)$/', $method, $m)) {
      throw new FusionDirectoryException(sprintf('Invalid request for endpoint %s: %s with %d path elements', $m[1], $m[2], $m[3]));
    }
  }

  protected function endpoint_objects_GET_1 ($input, string $type): array
  {
    return $this->_ls($type, ($_GET['attrs'] ?? NULL), ($_GET['base'] ?? NULL), ($_GET['filter'] ?? ''));
  }

  protected function endpoint_objects_GET_2 ($input, string $type, string $dn = NULL): array
  {
    return $this->endpoint_objects_GET_3($input, $type, $dn, NULL);
  }

  protected function endpoint_objects_GET_3 ($input, string $type, string $dn = NULL, string $tab = NULL): array
  {
    $this->checkAccess($type, $tab, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    if ($tab === NULL) {
      $object = $tabobject->getBaseObject();
    } else {
      $object = $tabobject->by_object[$tab];
    }
    if (!is_subclass_of($object, 'simplePlugin')) {
      throw new FusionDirectoryException('Invalid tab');
    }
    $attributes = [];
    $fields = $object->attributesInfo;
    foreach ($fields as $section) {
      foreach ($section['attrs'] as $attr) {
        if ($object->acl_is_readable($attr->getAcl())) {
          $attributes[$attr->getLdapName()] = $attr->getValue();
        }
      }
    }
    return $attributes;
  }

  protected function endpoint_token_GET_0 ($input): string
  {
    return $this->_getId();
  }

  protected function endpoint_ldaps_GET_0 ($input): array
  {
    $config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
    session::global_set('DEBUGLEVEL', 0);
    $ldaps  = [];
    foreach ($config->data['LOCATIONS'] as $id => $location) {
      $ldaps[$id] = $location['NAME'];
    }
    return $ldaps;
  }

  protected function endpoint_types_GET_0 ($input): array
  {
    return $this->_listTypes();
  }

  protected function endpoint_types_GET_1 ($input, string $type): array
  {
    return $this->_infos($type);
  }

  protected function endpoint_types_GET_2 ($input, string $type, string $tab): array
  {
    $this->checkAccess($type, $tab);

    $tabobject  = objects::create($type);
    $object     = $tabobject->by_object[$tab];
    if (!is_subclass_of($object, 'simplePlugin')) {
      throw new FusionDirectoryException('Invalid tab');
    }
    $fields = $object->attributesInfo;
    foreach ($fields as &$section) {
      $attributes = [];
      foreach ($section['attrs'] as $attr) {
        if ($object->acl_is_readable($attr->getAcl())) {
          $attributes[] = $attr->getLdapName();
        }
      }
      $section['attrs'] = $attributes;
    }
    unset($section);
    return ['sections' => $fields, 'sections_order' => array_keys($fields)];
  }
}

$service = new fdRestService();
$service->treatRequest();
