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

require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');
require_once('webservice/class_fdRPCService.inc');

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Headers: Content-Type, Session-Token, Authorization, Access-Control-Allow-Headers, X-Requested-With');
header('Access-Control-Allow-Methods: GET, POST, DELETE, PUT, OPTIONS');

class RestServiceEndPointError extends FusionDirectoryException
{
  public function toArray ()
  {
    return [
      'message' => $this->getMessage(),
      'line'    => $this->getLine(),
      'file'    => $this->getFile(),
    ];
  }
}

class RestServiceEndPointErrors extends FusionDirectoryException
{
  protected $errors;

  public function __construct (array $errors)
  {
    parent::__construct();
    $this->errors = $errors;
    foreach ($this->errors as &$error) {
      if (is_string($error)) {
        $error = new RestServiceEndPointError($error);
      }
    }
    unset($error);
  }

  public function toJson ()
  {
    return json_encode(
      array_map(
        function ($error)
        {
          return $error->toArray();
        },
        $this->errors
      )
    );
  }
}

/*!
 * \brief This class is the JSON-RPC webservice of FusionDirectory
 *
 * It must be served through jsonRPCServer::handle
 * */
class fdRestService extends fdRPCService
{
  public static function authenticateHeader ($message = 'Authentication required')
  {
    header('Content-Type: application/json');
    parent::authenticateHeader(json_encode($message));
  }

  public function treatRequest ()
  {
    global $_SERVER;

    try {
      // Get the HTTP method, path and body of the request
      $method   = $_SERVER['REQUEST_METHOD'];
      $request  = explode('/', trim($_SERVER['PATH_INFO'], '/'));
      $rawInput = file_get_contents('php://input');
      if ($rawInput === '') {
        $input  = NULL;
      } else {
        $input  = json_decode($rawInput, TRUE);
        if (json_last_error() != JSON_ERROR_NONE) {
          throw new RestServiceEndPointError('Error while decoding input JSON: '.json_last_error_msg());
        }
      }

      if ($method == 'OPTIONS') {
        /* Used by some GUI to get headers */
        exit;
      }

      if (implode('', $request) == 'openapi.yaml') {
        $this->sendOpenAPI('yaml');
      } elseif (implode('', $request) == 'openapi.json') {
        $this->sendOpenAPI('json');
      }

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
      } elseif ($request[0] == 'directories') {
        static::initiateRPCSession(NULL, NULL, NULL, NULL, FALSE);
      } else {
        static::initiateRPCSession($_SERVER['HTTP_SESSION_TOKEN']);
      }
      Language::setHeaders(session::global_get('lang'), 'application/json');

      if (count($request) == 0) {
        throw new RestServiceEndPointError('Empty request received');
      }

      $responseCode = 200;

      $endpoint = 'endpoint_'.array_shift($request).'_'.$method.'_'.count($request);
      $result   = $this->$endpoint($responseCode, $input, ...$request);

      http_response_code($responseCode);
      if ($responseCode == 204) {
        /* 204 - No Content */
        echo "\n";
      } else {
        $json = json_encode($result);
        if (json_last_error() != JSON_ERROR_NONE) {
          throw new RestServiceEndPointError('Error while encoding JSON result: '.json_last_error_msg(), 500);
        }
        echo $json."\n";
      }
    } catch (RestServiceEndPointErrors $e) {
      http_response_code(400);
      echo $e->toJson();
    } catch (RestServiceEndPointError $e) {
      if ($e->getCode() != 0) {
        http_response_code($e->getCode());
      } else {
        http_response_code(400);
      }
      echo json_encode([$e->toArray()]);
    } catch (NonExistingLdapNodeException $e) {
      http_response_code(404);
      echo json_encode(
        [[
          'message' => $e->getMessage(),
          'line'    => $e->getLine(),
          'file'    => $e->getFile(),
        ]]
      );
    } catch (Exception $e) {
      http_response_code(400);
      echo json_encode(
        [[
          'message' => $e->getMessage(),
          'line'    => $e->getLine(),
          'file'    => $e->getFile(),
        ]]
      );
    }
  }

  public function __call ($method, $params)
  {
    if (preg_match('/^endpoint_([^_]+)_([^_]+)_([^_]+)$/', $method, $m)) {
      throw new RestServiceEndPointError(sprintf('Invalid request for endpoint %s: %s with %d path elements', $m[1], $m[2], $m[3]), 405);
    }
  }

  protected function sendOpenAPI (string $format)
  {
    global $BASE_DIR;
    header('Content-Type: application/'.$format);
    if (!is_callable('yaml_parse_file')) {
      /* Fallback when php-yaml is missing */
      if (format != 'yaml') {
        throw new RestServiceEndPointError('You need php-yaml to be able to convert to other formats', 406);
      } else {
        readfile($BASE_DIR.'/html/openapi.yaml');
      }
    }
    $data = yaml_parse_file($BASE_DIR.'/html/openapi.yaml');
    if ($data === FALSE) {
      throw new RestServiceEndPointError('Parsing openapi.yaml failed', 500);
    }
    if (!is_array($data)) {
      throw new RestServiceEndPointError('openapi.yaml is invalid', 500);
    }

    $server = 'https://';
    if (empty($_SERVER['HTTP_X_FORWARDED_HOST'])) {
      $server .= $_SERVER['HTTP_HOST'];
    } else {
      $server .= $_SERVER['HTTP_X_FORWARDED_HOST'];
    }
    $server .= preg_replace('|/openapi.'.$format.'$|', '', $_SERVER['REQUEST_URI']);
    $data['servers'] = [['url' => $server]];

    switch ($format) {
      case 'json':
        echo json_encode($data, JSON_PRETTY_PRINT);
        break;
      case 'yaml':
        echo yaml_emit($data, YAML_UTF8_ENCODING);
        break;
      default:
        throw new RestServiceEndPointError(sprintf('Unsupported openapi format: ', $format), 406);
    }
    exit;
  }

  protected function endpoint_objects_GET_1 (int &$responseCode, $input, string $type): array
  {
    return $this->_ls($type, ($_GET['attrs'] ?? NULL), ($_GET['base'] ?? NULL), ($_GET['filter'] ?? ''));
  }

  protected function endpoint_objects_GET_2 (int &$responseCode, $input, string $type, string $dn = NULL): array
  {
    return $this->endpoint_objects_GET_3($responseCode, $input, $type, $dn, NULL);
  }

  protected function endpoint_objects_GET_3 (int &$responseCode, $input, string $type, string $dn = NULL, string $tab = NULL): array
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
      if (!isset($tabobject->by_object[$tab])) {
        throw new RestServiceEndPointError('This tab does not exists: "'.$tab.'"', 404);
      }
      $object = $tabobject->by_object[$tab];
    }
    if (!is_subclass_of($object, 'simplePlugin')) {
      throw new RestServiceEndPointError('Invalid tab', 501);
    }
    $attributes = [];
    $fields = $object->attributesInfo;
    foreach ($fields as $section) {
      foreach ($section['attrs'] as $attr) {
        if ($object->acl_is_readable($attr->getAcl())) {
          $attributes[$attr->getLdapName()] = $attr->serializeValue();
        }
      }
    }
    return $attributes;
  }

  protected function endpoint_objects_GET_4 (int &$responseCode, $input, string $type, string $dn, string $tab, string $attribute)
  {
    $this->checkAccess($type, $tab, $dn);

    $tabobject = objects::open($dn, $type);

    if (!isset($tabobject->by_object[$tab])) {
      throw new RestServiceEndPointError('This tab does not exists: "'.$tab.'"', 404);
    }

    $object = $tabobject->by_object[$tab];

    if (!is_subclass_of($object, 'simplePlugin')) {
      throw new RestServiceEndPointError('Invalid tab', 501);
    }

    if (!isset($object->attributesAccess[$attribute])) {
      throw new RestServiceEndPointError('Unknown attribute', 404);
    }

    if ($object->displayHeader && !$object->is_account) {
      return NULL;
    }

    if (!$object->acl_is_readable($object->attributesAccess[$attribute]->getAcl())) {
      throw new RestServiceEndPointError('Not enough rights to read "'.$attribute.'"', 403);
    }

    return $object->attributesAccess[$attribute]->serializeValue();
  }

  protected function endpoint_objects_POST_1 (int &$responseCode, $input, string $type): string
  {
    if (!isset($input['attrs'])) {
      throw new RestServiceEndPointError('Missing parameter "attrs" in POST data');
    }
    if (isset($input['template'])) {
      $result = $this->_usetemplate($type, $input['template'], $input['attrs']);
    } else {
      $result = $this->_setfields($type, NULL, $input['attrs']);
    }

    if (is_array($result) && isset($result['errors'])) {
      throw new RestServiceEndPointErrors($result['errors']);
    }

    $responseCode = 201;

    return $result;
  }

  protected function endpoint_objects_PUT_4 (int &$responseCode, $input, string $type, string $dn, string $tab = NULL, string $attribute = NULL)
  {
    $this->checkAccess($type, $tab, $dn);

    $tabobject = objects::open($dn, $type);
    if ($tab === NULL) {
      $object = $tabobject->getBaseObject();
    } elseif (!isset($tabobject->by_object[$tab])) {
      throw new RestServiceEndPointError('This tab does not exists: "'.$tab.'"', 404);
    } else {
      $object = $tabobject->by_object[$tab];
    }
    if (!is_subclass_of($object, 'simplePlugin')) {
      throw new RestServiceEndPointError('Invalid tab', 501);
    }
    if ($tabobject->by_object[$tab]->displayHeader &&
        !$tabobject->by_object[$tab]->is_account
      ) {
      list($disabled, $buttonText, $text) = $tabobject->by_object[$tab]->getDisplayHeaderInfos();
      if ($disabled) {
        throw new RestServiceEndPointError($text);
      }
      if ($tabobject->by_object[$tab]->acl_is_createable()) {
        $tabobject->by_object[$tab]->is_account = TRUE;
      } else {
        throw new RestServiceEndPointError('You don\'t have sufficient rights to enable tab "'.$tab.'"', 403);
      }
    }
    $error = $tabobject->by_object[$tab]->deserializeValues([$attribute => $input]);
    if ($error !== TRUE) {
      throw new RestServiceEndPointError($error);
    }

    /* Should not do much as POST is empty, but in some cases is needed */
    $tabobject->current = $tab;
    $tabobject->save_object();

    $errors = $tabobject->save();
    if (!empty($errors)) {
      throw new RestServiceEndPointErrors($errors);
    }

    $responseCode = 204;
  }

  protected function endpoint_objects_POST_4 (int &$responseCode, $input, string $type, string $dn, string $tab = NULL, string $attribute = NULL)
  {
    $result = $this->_addvalues($type, $dn, [$tab => [$attribute => $input]]);

    if (is_array($result) && isset($result['errors'])) {
      throw new RestServiceEndPointErrors($result['errors']);
    }

    $responseCode = 204;
  }

  protected function endpoint_objects_DELETE_2 (int &$responseCode, $input, string $type, string $dn)
  {
    $result = $this->_delete($type, $dn);

    if (is_array($result) && isset($result['errors'])) {
      throw new RestServiceEndPointErrors($result['errors']);
    }

    $responseCode = 204;
  }

  protected function endpoint_objects_DELETE_4 (int &$responseCode, $input, string $type, string $dn, string $tab = NULL, string $attribute = NULL)
  {
    $result = $this->_delvalues($type, $dn, [$tab => [$attribute => $input]]);

    if (is_array($result) && isset($result['errors'])) {
      throw new RestServiceEndPointErrors($result['errors']);
    }

    $responseCode = 204;
  }

  protected function endpoint_token_GET_0 (int &$responseCode, $input): string
  {
    return $this->_getId();
  }

  protected function endpoint_directories_GET_0 (int &$responseCode, $input): array
  {
    global $BASE_DIR;
    $config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
    session::global_set('DEBUGLEVEL', 0);
    $ldaps  = [];
    foreach ($config->data['LOCATIONS'] as $id => $location) {
      $ldaps[$id] = $location['NAME'];
    }
    return $ldaps;
  }

  protected function endpoint_types_GET_0 (int &$responseCode, $input): array
  {
    return $this->_listTypes();
  }

  protected function endpoint_types_GET_1 (int &$responseCode, $input, string $type): array
  {
    return $this->_infos($type);
  }

  protected function endpoint_types_GET_2 (int &$responseCode, $input, string $type, string $tab): array
  {
    $this->checkAccess($type, $tab);

    $tabobject  = objects::create($type);
    if (!isset($tabobject->by_object[$tab])) {
      throw new RestServiceEndPointError('This tab does not exists: "'.$tab.'"', 404);
    }
    $object     = $tabobject->by_object[$tab];
    if (!is_subclass_of($object, 'simplePlugin')) {
      throw new RestServiceEndPointError('Invalid tab', 501);
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
