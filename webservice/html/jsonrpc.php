<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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
 * \file jsonrpc.php
 * \brief This file is a webservice for FusionDirectory
 *
 * It's a JSON-RPC service usually used through HTTPS. It should be put in the html folder of FusionDirectory
 * Then the url of the webservice will be the url of your FusionDirectory instance followed by /jsonrpc.php
 */

ini_set('session.use_cookies',      '0');
ini_set('session.use_only_cookies', '1');

require_once("../include/php_setup.inc");
require_once("functions.inc");
require_once("variables.inc");
require_once("jsonrpcphp/jsonRPCServer.php");
require_once("webservice/class_fdRPCService.inc");

class fdJSONRPCService extends fdRPCService
{
  function __call ($method, $params)
  {
    global $config, $BASE_DIR;

    if (preg_match('/^_(.*)$/', $method, $m)) {
      throw new FusionDirectoryException("Non existing method '$m[1]'");
    }

    if ($method == 'listLdaps') {
      $config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
      $ldaps = [];
      foreach ($config->data['LOCATIONS'] as $id => $location) {
        $ldaps[$id] = $location['NAME'];
      }
      return $ldaps;
    } elseif ($method == 'login') {
      /* Login method have the following parameters: LDAP, user, password */
      static::initiateRPCSession(NULL, array_shift($params), array_shift($params), array_shift($params));
      $method = 'getId';
    } else {
      static::initiateRPCSession(array_shift($params));
    }

    try {
      return call_user_func_array([$this, '_'.$method], $params);
    } catch (WebServiceErrors $e) {
      return ['errors' => $e->toStringArray()];
    } catch (WebServiceError $e) {
      return ['errors' => [$e->getMessage()]];
    }
  }
}

$service = new fdJSONRPCService();
if (!jsonRPCServer::handle($service)) {
  echo "no request\n";
  echo session_id()."\n";
}
