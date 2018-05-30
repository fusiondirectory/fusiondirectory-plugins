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
  protected $ackUrl;
  protected $login;
  protected $password;

  function readLdapConfig()
  {
    global $config;

    $this->ackUrl   = $config->get_cfg_value('SinapsAcknowledgmentURL');
    $this->login    = $config->get_cfg_value('SinapsLogin');
    $this->password = $config->get_cfg_value('SinapsPassword');

    if ($config->get_cfg_value('SinapsEnabled') != 'TRUE') {
      $this->returnError($request, 'SINAPS integration is disabled'."\n");
    } else {
      return TRUE;
    }
  }

  function execute()
  {
    $http_raw_post_data = file_get_contents('php://input');

    if (!$http_raw_post_data) {
      return;
    }

    $request = new sinapsRequest($http_raw_post_data);

    if (($request->codeOperation() == 'DIFFUSION') && ($request->codeDomaine() == 'STRUCTURE')) {
      echo $request->acquittementTechnique(200, 'Diffusion de structure reçue')."\n";
      $this->handleStructureDiffusion($request, $request->getSupannEntiteValues());
    } elseif (($request->codeOperation() == 'DIFFUSION') && ($request->codeDomaine() == 'PERSONNE')) {
      echo $request->acquittementTechnique(200, 'Diffusion de personne reçue')."\n";
      $this->handlePersonneDiffusion($request, $request->getUserValues());
    } else {
      $this->returnError($request, 'Cannot handle '.$request->codeDomaine().' '.$request->codeOperation().' '.$request->operationVersion()."\n");
    }
  }

  function returnError($request, $errorText)
  {
    echo $request->acquittementTechnique(500, $errorText)."\n";
    error_log('Error: '.$errorText);
    exit();
  }

  function sendAcquittement($xml)
  {
    // performs the HTTP(S) POST
    $opts = array (
      'http' => array (
        'method'  => 'POST',
        'header'  => 'Content-type: application/xml'."\r\n".'Authorization: Basic '.base64_encode($this->login.':'.$this->password),
        'content' => $xml,
      ),
      'ssl' => array()
    );

    $context  = stream_context_create($opts);
    $fp = fopenWithErrorHandling($this->ackUrl, 'r', FALSE, $context);
    if (!is_array($fp)) {
      echo '***** Request *****'."\n".$xml.'***** End of request *****'."\n";
      $response = '';
      while ($row = fgets($fp)) {
        $response .= trim($row)."\n";
      }
      echo '***** Server response *****'."\n".$response.'***** End of server response *****'."\n";
    } else {
      if (!empty($fp)) {
        $errormsg = implode("\n", $fp);
      } else {
        $errormsg = 'Unable to connect to '.$this->ackUrl;
      }
      $this->returnError($errormsg);
    }
    exit();
  }

  function handleStructureDiffusion($request, $values)
  {
    $uuid = $values['entite']['supannRefId'][0];
    $entites = objects::ls('entite', 'ou', NULL, '(supannRefId='.$uuid.')');
    $message = 'Entite created';
    if (!empty($entites)) {
      if (count($entites) > 1) {
        $error = 'Multiple entite matches id '.$uuid;
        $this->sendAcquittement($request->acquittementFonctionnel(500, 10, $error, $dn));
        exit();
      } else {
        $dn = key($entites);
        $message = 'Entite updated';
      }
    } else {
      $dn = '';
    }
    $error = $this->fillObject('entite', $values, $dn);
    if ($error !== TRUE) {
      $this->sendAcquittement($request->acquittementFonctionnel(500, 10, print_r($error, TRUE), $uuid));
    } else {
      $this->sendAcquittement($request->acquittementFonctionnel(200, 0, $message, $uuid));
    }
  }

  function handlePersonneDiffusion($request, $values)
  {
    $uuid = $values['supannAccount']['supannRefId'][0];
    $entites = objects::ls('user', 'ou', NULL, '(supannRefId='.$uuid.')');
    $message = 'User created';
    if (!empty($entites)) {
      if (count($entites) > 1) {
        $error = 'Multiple user matches id '.$uuid;
        $this->sendAcquittement($request->acquittementFonctionnel(500, 10, $error, $dn));
        exit();
      } else {
        $dn = key($entites);
        $message = 'User updated';
      }
    } else {
      $dn = '';
    }
    $error = fillObject('user', $values, $dn);
    if ($error !== TRUE) {
      $this->sendAcquittement($request->acquittementFonctionnel(500, 10, $error, $uuid));
    } else {
      $this->sendAcquittement($request->acquittementFonctionnel(200, 0, $message, $uuid));
    }
  }

  function fillObject($type, $values, &$dn)
  {
    if (empty($dn)) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    foreach ($values as $tab => $tabvalues) {
      if (!isset($tabobject->by_object[$tab])) {
        return 'This tab does not exists: "'.$tab.'"';
      }
      if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
          $tabobject->by_object[$tab]->displayHeader &&
          !$tabobject->by_object[$tab]->is_account
        ) {
        list($disabled, $buttonText, $text) = $tabobject->by_object[$tab]->getDisplayHeaderInfos();
        if ($disabled) {
          return $text;
        }
        $tabobject->by_object[$tab]->is_account = TRUE;
      }
      $error = $tabobject->by_object[$tab]->deserializeValues($tabvalues);
      if ($error !== TRUE) {
        return $error;
      }
      $tabobject->current = $tab;
      $tabobject->save_object(); /* Should not do much as POST is empty, but in some cases may be needed */
    }
    $errors = $tabobject->save();
    if (empty($dn)) {
      $dn = $tabobject->dn;
    }
    if (!empty($errors)) {
      return $errors;
    }
    return TRUE;
  }
}

$sinapsHandler = new sinapsHandler();

$sinapsHandler->execute();

exit();

/*TODO
 * Ignorer si date validité pas atteinte
 */
