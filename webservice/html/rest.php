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

ini_set('session.use_cookies',      '0');
ini_set('session.use_only_cookies', '1');

require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');
require_once('webservice/class_fdRPCService.inc');

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Headers: Content-Type, Session-Token, Authorization, Access-Control-Allow-Headers, X-Requested-With');
header('Access-Control-Allow-Methods: GET, POST, DELETE, PUT, OPTIONS, PATCH');

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
    parent::authenticateHeader(static::encodeJson($message));
  }

  protected static function encodeJson ($value)
  {
    return json_encode($value, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
  }

  public function treatRequest ()
  {
    global $_SERVER;
    header('Content-Type: application/json');

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
          throw new WebServiceError('Error while decoding input JSON: '.json_last_error_msg());
        }
      }

      if ($method == 'OPTIONS') {
        /* Used by some GUI to get headers */
        exit;
      }

      $version = array_shift($request);

      if (empty($version)) {
        throw new WebServiceError('API Version is missing from request');
      } elseif ($version != 'v1') {
        if (preg_match('/^v([0-9].+)$/', $version)) {
          throw new WebServiceError('Version "'.$version.'" is either invalid or not supported');
        } else {
          throw new WebServiceError('Version is missing from requested path, try adding /v1 after rest.php');
        }
      }

      if (implode('', $request) == 'openapi.yaml') {
        $this->sendOpenAPI('yaml');
      } elseif (implode('', $request) == 'openapi.json') {
        $this->sendOpenAPI('json');
      }

      if ($request[0] == 'login') {
        if (count($request) > 1) {
          throw new WebServiceError('login endpoint has no subpath');
        }
        /* Login method have the following parameters: directory, user, password */
        static::initiateRPCSession(
          NULL,
          ($input['directory'] ?? $_GET['directory'] ?? NULL),
          ($input['user'] ?? $_GET['user'] ?? ''),
          ($input['password'] ?? $_GET['password'] ?? '')
        );
        $request  = ['token'];
        $method   = 'GET';
        $input    = NULL;
      } elseif ($request[0] == 'directories') {
        static::initiateRPCSession(NULL, NULL, NULL, NULL, FALSE);
      } else {
        static::initiateRPCSession($_SERVER['HTTP_SESSION_TOKEN']);
      }
      Language::setHeaders(session::global_get('lang'), 'application/json');

      if (count($request) == 0) {
        throw new WebServiceError('Empty request received');
      }

      $responseCode = 200;

      $endpoint = 'endpoint_'.array_shift($request).'_'.$method.'_'.count($request);
      $result   = $this->$endpoint($responseCode, $input, ...$request);

      http_response_code($responseCode);
      if ($responseCode == 204) {
        /* 204 - No Content */
        echo "\n";
      } else {
        $json = static::encodeJson($result);
        if (json_last_error() != JSON_ERROR_NONE) {
          throw new WebServiceError('Error while encoding JSON result: '.json_last_error_msg(), 500);
        }
        echo $json."\n";
      }
    } catch (WebServiceErrors $e) {
      http_response_code(400);
      echo $e->toJson();
    } catch (NonExistingLdapNodeException $e) {
      http_response_code(404);
      echo static::encodeJson(
        [[
          'class'   => get_class($this),
          'message' => $e->getMessage(),
          'line'    => $e->getLine(),
          'file'    => $e->getFile(),
        ]]
      );
    } catch (FusionDirectoryException $e) {
      if (($e instanceof WebServiceError) && ($e->getCode() != 0)) {
        http_response_code($e->getCode());
      } else {
        http_response_code(400);
      }
      echo static::encodeJson([$e->toArray()]);
    } catch (Exception $e) {
      http_response_code(400);
      echo static::encodeJson(
        [[
          'class'   => get_class($this),
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
      throw new WebServiceError(sprintf('Invalid request for endpoint %s: %s with %d path elements', $m[1], $m[2], $m[3]), 405);
    }
  }

  protected function sendOpenAPI (string $format)
  {
    global $BASE_DIR;
    header('Content-Type: application/'.$format);
    if (!is_callable('yaml_parse_file')) {
      /* Fallback when php-yaml is missing */
      if ($format != 'yaml') {
        throw new WebServiceError('You need php-yaml to be able to convert to other formats', 406);
      } else {
        readfile($BASE_DIR.'/html/openapi.yaml');
      }
    }
    $data = yaml_parse_file($BASE_DIR.'/html/openapi.yaml');
    if ($data === FALSE) {
      throw new WebServiceError('Parsing openapi.yaml failed', 500);
    }
    if (!is_array($data)) {
      throw new WebServiceError('openapi.yaml is invalid', 500);
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
        echo json_encode($data, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
        break;
      case 'yaml':
        echo yaml_emit($data, YAML_UTF8_ENCODING);
        break;
      default:
        throw new WebServiceError(sprintf('Unsupported openapi format: "%s"', $format), 406);
    }
    exit;
  }

  protected function endpoint_logout_POST_0 (int &$responseCode, $input)
  {
    $this->assertNoInput($input);

    $this->_logout();
    $responseCode = 204;
  }

  protected function endpoint_objects_GET_1 (int &$responseCode, $input, string $type): stdClass
  {
    $this->assertNoInput($input);

    return (object)$this->_ls($type, ($_GET['attrs'] ?? NULL), ($_GET['base'] ?? NULL), ($_GET['filter'] ?? ''), ($_GET['scope'] ?? 'subtree'), ($_GET['templates'] ?? FALSE));
  }

  protected function endpoint_objects_GET_2 (int &$responseCode, $input, string $type, string $dn): array
  {
    $this->assertNoInput($input);

    $tabs = $this->_listTabs($type, $dn);

    return array_map(
      function ($key, $array) {
        $array['class'] = $key;
        return $array;
      },
      array_keys($tabs),
      $tabs
    );
  }

  protected function endpoint_objects_GET_3 (int &$responseCode, $input, string $type, string $dn, string $tab = NULL): stdClass
  {
    $this->assertNoInput($input);

    global $config;

    if ($tab == 'templatefields') {
      $data = $this->_gettemplate($type, $dn);
      $data = array_map(
        function ($tabData) {
          return array_keys($tabData['attrs']);
        },
        $data
      );
      return (object)$data;
    }

    $this->checkAccess($type, $tab, $dn);

    if ((strtolower($type) == 'configuration') && ($dn != CONFIGRDN.$config->current['BASE'])) {
      throw new NonExistingLdapNodeException('Could not open configuration at dn '.$dn);
    }

    $tabobject = objects::open($dn, $type);

    if ($tab === NULL) {
      $object = $tabobject->getBaseObject();
    } else {
      if (!isset($tabobject->by_object[$tab])) {
        throw new WebServiceError('This tab does not exists: "'.$tab.'"', 404);
      }
      $object = $tabobject->by_object[$tab];
    }
    if (!is_subclass_of($object, 'simplePlugin')) {
      throw new WebServiceError('Invalid tab', 501);
    }
    if (!$object->isActive()) {
      throw new WebServiceError(sprintf('Tab "%s" is inactive', $tab));
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
    return (object)$attributes;
  }

  protected function endpoint_objects_GET_4 (int &$responseCode, $input, string $type, string $dn, string $tab, string $attribute)
  {
    $this->assertNoInput($input);

    $this->checkAccess($type, $tab, $dn);

    $tabobject = objects::open($dn, $type);

    if (!isset($tabobject->by_object[$tab])) {
      throw new WebServiceError('This tab does not exists: "'.$tab.'"', 404);
    }

    $object = $tabobject->by_object[$tab];

    if (!is_subclass_of($object, 'SimpleTab')) {
      throw new WebServiceError('Invalid tab', 501);
    }

    if (!isset($object->attributesAccess[$attribute])) {
      throw new WebServiceError('Unknown attribute', 404);
    }

    if (!$object->attrIsReadable($attribute)) {
      throw new WebServiceError('Not enough rights to read "'.$attribute.'"', 403);
    }

    if (!$object->isActive()) {
      throw new WebServiceError(sprintf('Tab "%s" is inactive', $tab));
    }

    return $object->attributesAccess[$attribute]->serializeValue();
  }

  protected function endpoint_objects_POST_1 (int &$responseCode, $input, string $type): string
  {
    $this->assertInput($input);

    if (!isset($input['attrs'])) {
      throw new WebServiceError('Missing parameter "attrs" in POST data');
    }
    if (isset($input['template'])) {
      $result = $this->_usetemplate($type, $input['template'], $input['attrs']);
    } else {
      $result = $this->_setfields($type, NULL, $input['attrs']);
    }

    $responseCode = 201;

    return $result;
  }

  protected function endpoint_objects_PATCH_2 (int &$responseCode, $input, string $type, string $dn): string
  {
    $this->assertInput($input);

    $result = $this->_setfields($type, $dn, $input);

    return $result;
  }

  protected function endpoint_objects_PATCH_3 (int &$responseCode, $input, string $type, string $dn, string $tab): string
  {
    return $this->endpoint_objects_PATCH_2($responseCode, [$tab => $input], $type, $dn);
  }

  protected function endpoint_objects_PUT_4 (int &$responseCode, $input, string $type, string $dn, string $tab = NULL, string $attribute = NULL): string
  {
    $this->assertInput($input);

    $this->checkAccess($type, $tab, $dn);

    static::addLockOrThrow($dn);
    try {
      $tabobject = objects::open($dn, $type);
      if ($tab === NULL) {
        $object = $tabobject->getBaseObject();
      } elseif (!isset($tabobject->by_object[$tab])) {
        throw new WebServiceError('This tab does not exists: "'.$tab.'"', 404);
      } else {
        $object = $tabobject->by_object[$tab];
      }
      if (!is_subclass_of($object, 'simplePlugin')) {
        throw new WebServiceError('Invalid tab', 501);
      }
      if ($tabobject->by_object[$tab]->isActivatable() &&
          !$tabobject->by_object[$tab]->isActive()
        ) {
        list($disabled, , $htmlText) = $tabobject->by_object[$tab]->getDisplayHeaderInfos();
        if ($disabled) {
          throw new WebServiceError(htmlunescape($htmlText));
        }
        if ($tabobject->by_object[$tab]->acl_is_createable()) {
          $tabobject->by_object[$tab]->is_account = TRUE;
        } else {
          throw new WebServiceError('You don\'t have sufficient rights to enable tab "'.$tab.'"', 403);
        }
      }
      $error = $tabobject->by_object[$tab]->deserializeValues([$attribute => $input]);
      if ($error !== TRUE) {
        throw new WebServiceErrors([$error]);
      }

      $tabobject->current = $tab;
      $tabobject->update();

      $errors = $tabobject->save();
      if (!empty($errors)) {
        throw new WebServiceErrors($errors);
      }

      return $tabobject->dn;
    } finally {
      Lock::deleteByObject($dn);
    }
  }

  protected function endpoint_objects_PATCH_5 (int &$responseCode, $input, string $type, string $dn, string $tab, string $attribute, string $values): string
  {
    $this->assertInput($input);

    if ($values !== 'values') {
      throw new WebServiceError('Invalid request for endpoint objects: PATCH with 5 path elements', 405);
    }

    $result = $this->_addvalues($type, $dn, [$tab => [$attribute => $input]]);

    return $result;
  }

  protected function endpoint_objects_DELETE_2 (int &$responseCode, $input, string $type, string $dn)
  {
    $this->assertNoInput($input);

    $this->_delete($type, $dn);

    $responseCode = 204;
  }

  protected function endpoint_objects_DELETE_3 (int &$responseCode, $input, string $type, string $dn, string $tab)
  {
    $this->assertNoInput($input);

    $this->_removetab($type, $dn, $tab);

    $responseCode = 204;
  }

  protected function endpoint_objects_DELETE_4 (int &$responseCode, $input, string $type, string $dn, string $tab, string $attribute)
  {
    $this->assertNoInput($input);

    $this->checkAccess($type, $tab, $dn);

    static::addLockOrThrow($dn);
    try {
      $tabobject = objects::open($dn, $type);

      if (!isset($tabobject->by_object[$tab])) {
        throw new WebServiceError('This tab does not exists: "'.$tab.'"', 404);
      }

      $object = $tabobject->by_object[$tab];

      if (!is_subclass_of($object, 'simplePlugin')) {
        throw new WebServiceError('Invalid tab', 501);
      }

      if (!isset($object->attributesAccess[$attribute])) {
        throw new WebServiceError('Unknown attribute', 404);
      }

      if (!$object->isActive()) {
        throw new WebServiceError('Inactive tab', 400);
      }

      if (!$object->acl_is_readable($object->attributesAccess[$attribute]->getAcl())) {
        throw new WebServiceError('Not enough rights to read "'.$attribute.'"', 403);
      }

      $object->attributesAccess[$attribute]->resetToDefault();

      $errors = $tabobject->save();
      if (!empty($errors)) {
        throw new WebServiceErrors($errors);
      }

      return $object->attributesAccess[$attribute]->serializeValue();
    } finally {
      Lock::deleteByObject($dn);
    }
  }

  protected function endpoint_objects_DELETE_5 (int &$responseCode, $input, string $type, string $dn, string $tab, string $attribute, string $values)
  {
    $this->assertInput($input);

    if ($values !== 'values') {
      throw new WebServiceError('Invalid request for endpoint objects: DELETE with 5 path elements', 405);
    }

    $result = $this->_delvalues($type, $dn, [$tab => [$attribute => $input]]);

    $responseCode = 204;
  }

  protected function endpoint_token_GET_0 (int &$responseCode, $input): string
  {
    $this->assertNoInput($input);

    return $this->_getId();
  }

  protected function endpoint_directories_GET_0 (int &$responseCode, $input): stdClass
  {
    $this->assertNoInput($input);

    global $BASE_DIR;
    $config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
    session::global_set('DEBUGLEVEL', 0);
    $ldaps  = [];
    foreach ($config->data['LOCATIONS'] as $id => $location) {
      $ldaps[$id] = $location['NAME'];
    }
    return (object)$ldaps;
  }

  protected function endpoint_types_GET_0 (int &$responseCode, $input): stdClass
  {
    $this->assertNoInput($input);

    return (object)$this->_listTypes();
  }

  protected function endpoint_types_GET_1 (int &$responseCode, $input, string $type): array
  {
    $this->assertNoInput($input);

    $infos = $this->_infos($type);

    /* Convert CLASS and NAME keys to lowercase */
    $infos['tabs'] = array_map('array_change_key_case', $infos['tabs']);

    return $infos;
  }

  protected function endpoint_types_GET_2 (int &$responseCode, $input, string $type, string $tab): array
  {
    $this->assertNoInput($input);

    $this->checkAccess($type, $tab);

    $tabobject  = objects::create($type);
    if (!isset($tabobject->by_object[$tab])) {
      throw new WebServiceError('This tab does not exists: "'.$tab.'"', 404);
    }
    $object     = $tabobject->by_object[$tab];
    if (!is_subclass_of($object, 'simplePlugin')) {
      throw new WebServiceError('Invalid tab', 501);
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

  protected function endpoint_userlock_GET_1 (int &$responseCode, $input, string $dn): bool
  {
    $this->assertNoInput($input);

    return $this->_isUserLocked([$dn])[$dn];
  }

  protected function endpoint_userlock_PUT_1 (int &$responseCode, $input, string $dn)
  {
    $this->assertInput($input);

    $this->_lockUser([$dn], ($input ? 'lock' : 'unlock'));

    $responseCode = 204;
  }

  protected function endpoint_recovery_GET_0 (int &$responseCode, $input): array
  {
    $this->assertNoInput($input);

    return $this->_recoveryGenToken($_GET['email'] ?? NULL);
  }

  protected function endpoint_recovery_PUT_0 (int &$responseCode, $input)
  {
    $this->assertInput($input);

    foreach (['login','token','password1','password2'] as $param) {
      if (!isset($input[$param])) {
        throw new WebServiceError('Missing parameter "'.$param.'" in POST data');
      }
    }

    $this->_recoveryConfirmPasswordChange($input['login'], $input['password1'], $input['password2'], $input['token']);

    $responseCode = 204;
  }

  private function assertNoInput ($input)
  {
    if ($input !== NULL) {
      throw new WebServiceError('Invalid request: unexpected request body, should be empty', 400);
    }
  }

  private function assertInput ($input)
  {
    if ($input === NULL) {
      throw new WebServiceError('Invalid request: body is missing', 400);
    }
  }
}

$service = new fdRestService();
$service->treatRequest();
