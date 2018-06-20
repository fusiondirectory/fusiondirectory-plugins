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
  protected $acqUrl;
  protected $login;
  protected $password;
  protected $dumpFolder;
  protected $uuidPrefix;
  protected $identifiantApplication;
  protected $tokens;

  protected $request;

  function readLdapConfig()
  {
    global $config;

    $this->ackUrl                 = $config->get_cfg_value('SinapsAcknowledgmentURL');
    $this->acqUrl                 = $config->get_cfg_value('SinapsAcquisitionURL');
    $this->login                  = $config->get_cfg_value('SinapsLogin');
    $this->password               = $config->get_cfg_value('SinapsPassword');
    $this->dumpFolder             = $config->get_cfg_value('SinapsDumpFolder');
    $this->uuidPrefix             = $config->get_cfg_value('SinapsUuidPrefix', 'LDAPUUID');
    $this->identifiantApplication = $config->get_cfg_value('SinapsIdentifiantApplication');
    $this->tokens                 = $config->get_cfg_value('SinapsFDToken', array());

    if ($config->get_cfg_value('SinapsEnabled') != 'TRUE') {
      $this->returnError('SINAPS integration is disabled'."\n");
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

    $this->request = new sinapsRequest($http_raw_post_data);

    if (!isset($_GET['token']) || !in_array($_GET['token'], $this->tokens)) {
      $this->returnError('No token or invalid token was provided'."\n");
    }

    $this->dumpFile(
      $this->request->identifiantTransaction().'.xml',
      $http_raw_post_data
    );

    if (($this->request->codeOperation() == 'DIFFUSION') && ($this->request->codeDomaine() == 'STRUCTURE')) {
      $this->outputAcquittementTechnique($this->request->acquittementTechnique(200, 'Diffusion de structure reçue'));
      $this->handleStructureDiffusion($this->request->getSupannEntiteValues($this));
    } elseif (($this->request->codeOperation() == 'DIFFUSION') && ($this->request->codeDomaine() == 'PERSONNE')) {
      $this->outputAcquittementTechnique($this->request->acquittementTechnique(200, 'Diffusion de personne reçue'));
      $this->handlePersonneDiffusion($this->request->getUserValues($this));
    } else {
      $this->returnError('Cannot handle '.$this->request->codeDomaine().' '.$this->request->codeOperation().' '.$this->request->operationVersion()."\n");
    }
  }

  function dumpFile($fileName, $fileContent)
  {
    if (empty($this->dumpFolder)) {
      return;
    }

    $fileName = $this->dumpFolder.'/'.$fileName;

    if (!is_dir($this->dumpFolder)) {
      if (!mkdir($this->dumpFolder, 0777, TRUE)) {
        $this->returnError('Failed to create dump folder '.$this->dumpFolder, FALSE);
      }
    }
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
      $this->returnError($errormsg, FALSE);
    }
  }

  function returnError($errorText, $dump = TRUE)
  {
    if (!is_object($this->request)) {
      // If we were called too soon, for instance from readLdapConfig
      $http_raw_post_data = file_get_contents('php://input');

      if (!$http_raw_post_data) {
        exit();
      }

      $this->request = new sinapsRequest($http_raw_post_data);
    }
    $acquittement = $this->request->acquittementTechnique(500, $errorText);
    echo "$acquittement\n";
    if ($dump) {
      $this->dumpFile(
        $this->request->identifiantTransaction().'-answer-error.xml',
        $acquittement
      );
    }
    error_log('Error: '.$errorText);
    exit();
  }

  function outputAcquittementTechnique($acquittement)
  {
    echo "$acquittement\n";
    $this->dumpFile(
      $this->request->identifiantTransaction().'-answer.xml',
      $acquittement
    );
  }

  protected static function sendPostRequest($url, $user, $password, $data)
  {
    $ch = curl_init();

    curl_setopt($ch, CURLOPT_URL,         $url);
    curl_setopt($ch, CURLOPT_POST,        1);
    curl_setopt($ch, CURLOPT_POSTFIELDS,  $data);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: text/xml'));
    curl_setopt($ch, CURLOPT_USERPWD, "$user:$password");
    curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

    // receive server response ...
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);

    $serverOutput = curl_exec($ch);

    curl_close($ch);

    return $serverOutput;
  }


  function sendAcquittementFonctionnel($xml)
  {
    $answer = static::sendPostRequest($this->ackUrl, $this->login, $this->password, $xml);
    if ($answer !== FALSE) {
      $this->dumpFile(
        $this->request->identifiantTransaction().'-acquittement.xml',
        $xml
      );
      $this->dumpFile(
        $this->request->identifiantTransaction().'-acquittement-answer.xml',
        $answer
      );
    } else {
      $errormsg = 'Unable to connect to '.$this->ackUrl;
      //TODO: returnError should not be used once acquittementTechnique was already sent it’s too late
      $this->returnError($errormsg);
    }
    exit();
  }

  function handleStructureDiffusion($values)
  {
    $uuid     = $values['entite']['supannRefId'][0];
    $idObjApp = preg_replace('/^{'.$this->uuidPrefix.'}/', '', $uuid);
    $entites  = objects::ls('entite', 'ou', NULL, '(supannRefId='.$uuid.')');
    $message  = 'Entite created';
    if (!empty($entites)) {
      if (count($entites) > 1) {
        $error = 'Multiple entite matches id '.$uuid;
        $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, $error, $idObjApp));
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
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, strip_tags(implode(', ', $error)), $idObjApp));
    } else {
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(200, 0, $message, $idObjApp));
    }
  }

  function handlePersonneDiffusion($values)
  {
    $uuid     = $values['supannAccount']['supannRefId'][0];
    $idObjApp = preg_replace('/^{'.$this->uuidPrefix.'}/', '', $uuid);
    $entites  = objects::ls('user', 'ou', NULL, '(supannRefId='.$uuid.')');
    $message  = 'User created';
    if (!empty($entites)) {
      if (count($entites) > 1) {
        $error = 'Multiple user matches id '.$uuid;
        $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, $error, $idObjApp));
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
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, strip_tags(implode(', ', $error)), $idObjApp));
    } else {
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(200, 0, $message, $idObjApp));
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
