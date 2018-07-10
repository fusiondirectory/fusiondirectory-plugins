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

class sinapsDiffusionHandlerJob
{
  protected $dryRun;
  protected $ackUrl;
  protected $acqUrl;
  protected $login;
  protected $password;
  protected $dumpFolder;
  protected $uuidPrefix;
  protected $identifiantApplication;

  protected $request;

  function __construct(sinapsRequest $request)
  {
    global $config;

    $this->request = $request;

    $this->dryRun                 = ($config->get_cfg_value('SinapsDryRun') == 'TRUE');
    $this->ackUrl                 = $config->get_cfg_value('SinapsAcknowledgmentURL');
    $this->acqUrl                 = $config->get_cfg_value('SinapsAcquisitionURL');
    $this->login                  = $config->get_cfg_value('SinapsLogin');
    $this->password               = $config->get_cfg_value('SinapsPassword');
    $this->dumpFolder             = $config->get_cfg_value('SinapsDumpFolder');
    $this->uuidPrefix             = $config->get_cfg_value('SinapsUuidPrefix', 'LDAPUUID');
    $this->identifiantApplication = $config->get_cfg_value('SinapsIdentifiantApplication');
  }

  function handleRequest()
  {
    if ($this->request->codeOperation() != 'DIFFUSION') {
      throw new FusionDirectoryException('Invalid operation '.$this->request->codeOperation());
    }

    if ($this->request->codeDomaine() == 'STRUCTURE') {
      $this->handleStructureDiffusion($this->request->getSupannEntiteValues($this->identifiantApplication, $this->uuidPrefix, array($this, 'ldapUuidToCodeEntite')));
    } elseif ($this->request->codeDomaine() == 'PERSONNE') {
      $this->handlePersonneDiffusion($this->request->getUserValues($this->identifiantApplication, $this->uuidPrefix, array($this, 'ldapUuidToCodeEntite')));
    } else {
      throw new FusionDirectoryException('Invalid domain '.$this->request->codeDomaine());
    }
  }

  function ldapUuidToCodeEntite($uuid)
  {
    /* Translate UUID refs to code entite
     * This is done here to make sure sinapsRequest does not perform LDAP searches by itself
     * */
    if (!preg_match('/^{'.$this->uuidPrefix.'}/', $uuid)) {
      $uuid = '{'.$this->uuidPrefix.'}'.$uuid;
    }
    $entites = objects::ls('entite', array('supannCodeEntite' => 1, 'supannTypeEntite' => 1), NULL, '(supannRefId='.$uuid.')');
    if (empty($entites)) {
      $error = 'Could not find parent '.$uuid;
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, $error));
    } elseif (count($entites) > 1) {
      $error = 'Multiple entite matches id '.$uuid;
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, $error));
    } else {
      return reset($entites);
    }
  }

  function dumpFile($fileName, $fileContent)
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
      error_log('Could not dump file: '.$errormsg);
    }
  }

  function handleStructureDiffusion($values)
  {
    $uuid     = $values['entite']['supannRefId'][0];
    $idObjApp = preg_replace('/^{'.$this->uuidPrefix.'}/', '', $uuid);
    $entites  = objects::ls('entite', array('supannRefId' => '*'), NULL, '(supannRefId='.$uuid.')');
    $message  = 'Entite created';
    if (empty($entites)) {
      $dn = '';
    } elseif (count($entites) > 1) {
      $error = 'Multiple entite matches id '.$uuid;
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, $error));
    } else {
      $dn = key($entites);
      /* Keep previous ref ids */
      $values['entite']['supannRefId'] = array_values(array_unique(array_merge($values['entite']['supannRefId'], $entites[$dn]['supannRefId'])));
      $message = 'Entite updated';
    }
    $error = $this->fillObject('entite', $values, $dn);
    if ($error !== TRUE) {
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, strip_tags(implode(', ', $error))));
    } else {
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(200, 0, $message, $idObjApp));
    }
  }

  function handlePersonneDiffusion($values)
  {
    $uuid     = $values['supannAccount']['supannRefId'][0];
    $idObjApp = preg_replace('/^{'.$this->uuidPrefix.'}/', '', $uuid);
    $users    = objects::ls('user', 'ou', NULL, '(supannRefId='.$uuid.')');
    $message  = 'User created';
    if (!empty($users)) {
      if (count($users) > 1) {
        $error = 'Multiple user matches id '.$uuid;
        $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, $error));
      } else {
        $dn = key($users);
        $message = 'User updated';
        //TODO: Ne pas écraser les champs multivalué?
      }
    } else {
      $dn = '';
    }
    $error = $this->fillObject('user', $values, $dn);
    if ($error !== TRUE) {
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(500, 10, strip_tags(implode(', ', $error))));
    } else {
      $this->sendAcquittementFonctionnel($this->request->acquittementFonctionnel(200, 0, $message, $idObjApp));
    }
  }

  function fillObject($type, $values, &$dn)
  {
    if ($this->dryRun) {
      $this->dumpFile(
        $this->request->identifiantTransaction().'-dryrun.txt',
        "type:$type\ndn:$dn\n".print_r($values, TRUE)
      );
      return TRUE;
    }
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

  function sendAcquittementFonctionnel($xml)
  {
    $this->dumpFile(
      $this->request->identifiantTransaction().'-acquittement.xml',
      $xml
    );
    echo "$xml\n";
    exit();
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
}
