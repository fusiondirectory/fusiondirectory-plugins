<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018 FusionDirectory

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

ini_set('session.use_cookies', 0);
ini_set('session.use_only_cookies', 1);

require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');

class sinapsHandler extends standAlonePage
{
  protected $dumpFolder;
  protected $tokens;

  protected $request;

  function readLdapConfig ()
  {
    global $config;

    $this->dumpFolder             = $config->get_cfg_value('SinapsDumpFolder');
    $this->tokens                 = $config->get_cfg_value('SinapsFDToken', []);

    if ($config->get_cfg_value('SinapsEnabled') != 'TRUE') {
      $this->returnError(400, 1, 'SINAPS integration is disabled'."\n");
    } elseif (!empty($this->dumpFolder) && !is_dir($this->dumpFolder)) {
      if (!mkdir($this->dumpFolder, 0777, TRUE)) {
        $this->returnError(200, 21, 'Failed to create dump folder '.$this->dumpFolder, FALSE);
      }
    }
    return TRUE;
  }

  function execute ()
  {
    global $config;

    $http_raw_post_data = file_get_contents('php://input');

    if (!$http_raw_post_data) {
      return;
    }

    try {
      $this->request = new sinapsRequest($http_raw_post_data);
    } catch (Exception $e) {
      $this->returnError(500, 1, 'Parsing failed: '.$e->getMessage()."\n");
    }

    if (!isset($_GET['token']) || !in_array($_GET['token'], $this->tokens)) {
      $this->returnError(500, 2, 'No token or invalid token was provided'."\n");
    }

    $this->dumpFile(
      $this->request->identifiantTransaction().'.xml',
      $http_raw_post_data
    );

    if (($this->request->codeOperation() == 'DIFFUSION') && (($this->request->codeDomaine() == 'STRUCTURE') || ($this->request->codeDomaine() == 'PERSONNE'))) {
      $job = new sinapsDiffusionHandlerJob($this->request);
      $job->handleRequest();
    } else {
      $this->returnError(400, 2, 'Cannot handle '.$this->request->codeDomaine().' '.$this->request->codeOperation().' '.$this->request->operationVersion()."\n");
    }
  }

  function dumpFile ($fileName, $fileContent)
  {
    if (empty($this->dumpFolder)) {
      return;
    }

    $fileName = $this->dumpFolder.'/'.$fileName;

    $fp = fopenWithErrorHandling($fileName, 'w');
    if (!is_array($fp)) {
      fwrite($fp, $fileContent);
      fclose($fp);
    } else {
      if (!empty($fp)) {
        $errormsg = implode("\n", $fp);
      } else {
        $errormsg = 'Unable to dump in '.$fileName;
      }
      $this->returnError(200, 22, $errormsg, FALSE);
    }
  }

  function returnError ($responseCode, $codeAcquittement, $errorText, $dump = TRUE)
  {
    $acquittement = sinapsRequest::acquittementFonctionnel($responseCode, $codeAcquittement, $errorText);
    echo "$acquittement\n";
    if ($dump) {
      if (is_object($this->request)) {
        $filename = $this->request->identifiantTransaction().'-answer-error.xml';
      } else {
        $filename = date('c').'-answer-error.xml';
      }
      $this->dumpFile($filename, $acquittement);
    }
    error_log('Error: '.$errorText);
    exit();
  }

  function outputAcquittementTechnique ($acquittement)
  {
    echo "$acquittement\n";
    $this->dumpFile(
      $this->request->identifiantTransaction().'-answer.xml',
      $acquittement
    );
  }
}

$sinapsHandler = new sinapsHandler();

$sinapsHandler->execute();

exit();
