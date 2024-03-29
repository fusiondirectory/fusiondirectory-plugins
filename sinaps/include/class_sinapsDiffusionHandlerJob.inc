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
  protected $acqUrl;
  protected $login;
  protected $password;
  protected $uuidPrefix;
  protected $identifiantApplication;
  protected $userBase;
  protected $userTemplate;
  protected $identifiantApplicationSync;
  protected $userRoles;

  protected $userMapping;
  protected $structureMapping;

  protected $request;

  function __construct (sinapsRequest $request)
  {
    global $config;

    $this->request = $request;

    $this->dryRun                     = ($config->get_cfg_value('SinapsDryRun') == 'TRUE');
    $this->acqUrl                     = $config->get_cfg_value('SinapsAcquisitionURL');
    $this->login                      = $config->get_cfg_value('SinapsLogin');
    $this->password                   = $config->get_cfg_value('SinapsPassword');
    $this->uuidPrefix                 = $config->get_cfg_value('SinapsUuidPrefix', 'LDAPUUID');
    $this->identifiantApplication     = $config->get_cfg_value('SinapsIdentifiantApplication');
    $this->userBase                   = $config->get_cfg_value('SinapsUserBase', $config->current['BASE']);
    $this->userTemplate               = $config->get_cfg_value('SinapsUserTemplate');
    $this->identifiantApplicationSync = $config->get_cfg_value('SinapsIdentifiantApplicationSync', []);
    $this->userRoles                  = $config->get_cfg_value('SinapsUserRole', ['PR', 'EXT/PRA']);

    $this->userMapping = [];
    foreach ($config->get_cfg_value('SinapsUserDiffusionMap', []) as $line) {
      list($sinaps, $tab, $field) = explode('|', $line);
      $this->userMapping[$sinaps] = [$tab,$field];
    }
    $this->structureMapping = [];
    foreach ($config->get_cfg_value('SinapsStructureDiffusionMap', []) as $line) {
      list($sinaps, $tab, $field) = explode('|', $line);
      $this->structureMapping[$sinaps] = [$tab,$field];
    }
  }

  function handleRequest ()
  {
    if ($this->request->codeOperation() != 'DIFFUSION') {
      throw new FusionDirectoryException('Invalid operation '.$this->request->codeOperation());
    }

    if ($this->request->codeDomaine() == 'STRUCTURE') {
      $this->handleStructureDiffusion($this->request->getSupannEntiteValues($this->identifiantApplication, $this->identifiantApplicationSync, $this->uuidPrefix, [$this, 'ldapUuidToCodeEntite'], $this->structureMapping));
    } elseif ($this->request->codeDomaine() == 'PERSONNE') {
      $this->handlePersonneDiffusion($this->request->getUserValues($this->identifiantApplication, $this->identifiantApplicationSync, $this->uuidPrefix, [$this, 'ldapUuidToCodeEntite'], $this->userRoles, $this->userMapping));
    } else {
      throw new FusionDirectoryException('Invalid domain '.$this->request->codeDomaine());
    }
  }

  function ldapUuidToCodeEntite ($uuid, $typeNeeded = FALSE)
  {
    /* Translate UUID refs to code entite
     * This is done here to make sure sinapsRequest does not perform LDAP searches by itself
     * */
    if (!preg_match('/^{'.$this->uuidPrefix.'}/', $uuid)) {
      $uuid = '{'.$this->uuidPrefix.'}'.$uuid;
    }
    $entites = objects::ls(['entite','etablissement'], ['supannCodeEntite' => 1, 'supannTypeEntite' => 1], NULL, '(supannRefId='.$uuid.')');
    if (empty($entites)) {
      $error = 'Could not find entite '.$uuid;
      $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 10, $error));
    } elseif (count($entites) > 1) {
      $error = 'Multiple entite matches id '.$uuid;
      $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 11, $error));
    } else {
      $entite = reset($entites);
      if (!isset($entite['supannCodeEntite'])) {
        $error = 'Entite '.key($entites).' has no supannCodeEntite';
        $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 17, $error));
      }
      if ($typeNeeded && !isset($entite['supannTypeEntite'])) {
        $error = 'Entite '.key($entites).' has no supannTypeEntite';
        $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 18, $error));
      }
      return $entite;
    }
  }

  public static function dumpFile ($fileName, $fileContent)
  {
    global $config;
    $dumpFolder = $config->get_cfg_value('SinapsDumpFolder');

    if (empty($dumpFolder)) {
      return;
    }

    $fileName = $dumpFolder.'/'.$fileName;

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

  function handleStructureDiffusion ($values)
  {
    $uuid     = $values['entite']['supannRefId'][0];
    $idObjApp = preg_replace('/^{'.$this->uuidPrefix.'}/', '', $uuid);
    $entites  = objects::ls('entite', ['supannRefId' => '*'], NULL, '(supannRefId='.$uuid.')');
    $message  = 'Entite created';
    if (empty($entites)) {
      $dn = '';
    } elseif (count($entites) > 1) {
      $error = 'Multiple entite matches id '.$uuid;
      $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 12, $error));
    } else {
      $dn = key($entites);
      /* Keep previous ref ids */
      foreach ($entites[$dn]['supannRefId'] as $supannRefId) {
        if (preg_match('/^{([^}]+)}.+$/', $supannRefId, $m)) {
          if (!in_array($m[1], $this->identifiantApplicationSync) && ($m[1] != $this->identifiantApplication)) {
            $values['entite']['supannRefId'][] = $supannRefId;
          }
        }
      }
      $values['entite']['supannRefId'] = array_values(array_unique($values['entite']['supannRefId']));
      $message = 'Entite updated';
    }
    $errors = $this->fillObject('entite', $values, $dn);
    if ($errors !== TRUE) {
      $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 13, strip_tags(implode(', ', $errors))));
    } else {
      $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 0, $message, $idObjApp));
    }
  }

  function handlePersonneDiffusion ($values)
  {
    $uuid     = $values['supannAccount']['supannRefId'][0];
    $idObjApp = preg_replace('/^{'.$this->uuidPrefix.'}/', '', $uuid);
    $users    = objects::ls('user', ['supannRefId' => '*'], NULL, '(supannRefId='.$uuid.')');
    $message  = 'User created';
    if (!empty($users)) {
      if (count($users) > 1) {
        $error = 'Multiple user matches id '.$uuid;
        $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 14, $error));
      } else {
        $dn = key($users);
        /* Keep previous ref ids */
        foreach ($users[$dn]['supannRefId'] as $supannRefId) {
          if (preg_match('/^{([^}]+)}.+$/', $supannRefId, $m)) {
            if (!in_array($m[1], $this->identifiantApplicationSync) && ($m[1] != $this->identifiantApplication)) {
              $values['supannAccount']['supannRefId'][] = $supannRefId;
            }
          }
        }
        $values['supannAccount']['supannRefId'] = array_values(array_unique($values['supannAccount']['supannRefId']));
        if ($values['lock']) {
          $message = 'User deleted';
        } else {
          $message = 'User updated';
        }
      }
    } else {
      if ($values['lock']) {
        $error = 'Cannot delete non-existing user '.$uuid;
        $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 16, $error));
      }
      $dn = '';
    }
    $errors = $this->fillObject('user', $values, $dn);
    if ($errors !== TRUE) {
      $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 15, strip_tags(implode(', ', $errors))));
    } else {
      if ($values['lock']) {
        /* $values['lock'] means this was a deletion, so we must not send an object identifier in the answer */
        $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 0, $message));
      } else {
        $this->sendAcquittementFonctionnel(sinapsRequest::acquittementFonctionnel(200, 0, $message, $idObjApp));
      }
    }
  }

  function fillObject ($type, $values, &$dn)
  {
    global $config;

    $lock = FALSE;
    if (isset($values['lock'])) {
      $lock = $values['lock'];
      unset($values['lock']);
    }

    if ($this->dryRun) {
      static::dumpFile(
        $this->request->identifiantTransaction().'-dryrun.txt',
        "type:$type\ndn:$dn\n".print_r($values, TRUE)
      );
      return TRUE;
    }
    if (empty($dn)) {
      if (($type == 'user') && !empty($this->userBase)) {
        $values['user']['base'] = $this->userBase;
      }
      if (($type == 'user') && !empty($this->userTemplate)) {
        $template = new template($type, $this->userTemplate);
        $error    = $template->deserialize($values);
        if ($error !== TRUE) {
          return [$error];
        }
        $tabobject  = $template->apply();
        $errors     = $tabobject->save();
        if (!empty($errors)) {
          return $errors;
        }
        $dn = $tabobject->dn;

        $errors = $this->lockUser($dn);

        if (!empty($errors)) {
          return $errors;
        }

        return TRUE;
      } else {
        $tabobject = objects::create($type);
      }
    } else {
      $tabobject = objects::open($dn, $type);
    }
    foreach ($values as $tab => $tabvalues) {
      if (!isset($tabobject->by_object[$tab])) {
        return ['This tab does not exists: "'.$tab.'"'];
      }
      if ($tabvalues === FALSE) {
        if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
            $tabobject->by_object[$tab]->isActivatable() &&
            $tabobject->by_object[$tab]->isActive()
          ) {
          list($disabled, , $text) = $tabobject->by_object[$tab]->getDisplayHeaderInfos();
          if ($disabled) {
            return [$text];
          }
          $tabobject->by_object[$tab]->is_account = FALSE;
        }
      } else {
        if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
            $tabobject->by_object[$tab]->isActivatable() &&
            !$tabobject->by_object[$tab]->isActive()
          ) {
          list($disabled, , $text) = $tabobject->by_object[$tab]->getDisplayHeaderInfos();
          if ($disabled) {
            return [$text];
          }
          $tabobject->by_object[$tab]->is_account = TRUE;
        }
        $error = $tabobject->by_object[$tab]->deserializeValues($tabvalues);
        if ($error !== TRUE) {
          return $error;
        }
      }
      $tabobject->current = $tab;
      $tabobject->update();
    }
    $errors = $tabobject->save();
    if (empty($dn)) {
      $dn = $tabobject->dn;
    }
    if (!empty($errors)) {
      return $errors;
    }
    if ($lock) {
      $errors = $this->lockUser($dn);

      if (!empty($errors)) {
        return $errors;
      }
    }
    return TRUE;
  }

  protected function lockUser ($dn)
  {
    global $config;

    // Try to lock the entry
    $ldap   = $config->get_ldap_link();
    $errors = [];
    $ldap->cat($dn, ['userPassword']);
    if ($ldap->count() == 1) {
      // We can't lock empty passwords.
      $val = $ldap->fetch();
      if (!isset($val['userPassword'])) {
        $errors[] = sprintf(_('Failed to get password method for account "%s". It has not been locked!'), $dn);
        return $errors;
      }
      // Detect the password method and try to lock
      $method = passwordMethod::get_method($val['userPassword'][0], $dn);
      $success = TRUE;
      if (!$method->is_locked($dn)) {
        $success = $method->lock_account($dn);
      }

      // Check if everything went fine.
      if (!$success) {
        $hn = $method->get_hash_name();
        if (is_array($hn)) {
          $hn = $hn[0];
        }
        $errors[] = sprintf(_('Password method "%s" failed locking. Account "%s" has not been locked!'), $hn, $dn);
      }
    } else {
      $errors[] = sprintf(_('Could not find account "%s" in LDAP. It has not been locked!'), $dn);
    }

    return $errors;
  }

  function sendAcquittementFonctionnel ($xml)
  {
    static::dumpFile(
      $this->request->identifiantTransaction().'-acquittement.xml',
      $xml
    );
    echo "$xml\n";
    exit();
  }

  public static function sendPostRequest ($url, $user, $password, $data)
  {
    $ch = curl_init();

    curl_setopt($ch, CURLOPT_URL,         $url);
    curl_setopt($ch, CURLOPT_POST,        1);
    curl_setopt($ch, CURLOPT_POSTFIELDS,  $data);
    curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: text/xml']);
    curl_setopt($ch, CURLOPT_USERPWD, "$user:$password");
    curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

    // receive server response ...
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);

    $serverOutput = curl_exec($ch);

    curl_close($ch);

    return $serverOutput;
  }
}
