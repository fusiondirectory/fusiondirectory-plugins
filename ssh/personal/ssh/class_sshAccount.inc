<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class SSHPublicKeyAttribute extends FileTextAreaAttribute
{
  /* cache for key extracted information */
  protected $format = '/^([-[:alnum:]=",.]+\s+)?(disabled-)?(ssh-(dss|rsa|ed25519)|ecdsa-sha2-nistp\d{3})\s+([[:alnum:]+\/.=]+)(\s+([[:graph:][:space:]]+))?\s*$/';

  function getArrayValue (): array
  {
    if (preg_match($this->format, $this->value, $match)) {
      $fingerprint  = $this->fingerprint(base64_decode($match[5]));
      $comment      = (isset($match[7]) ? $match[7] : '');
      switch ($match[3]) {
        case 'ssh-dss':
          $type = 'DSA';
          break;
        case 'ssh-rsa':
          $type = 'RSA';
          break;
        case 'ssh-ed25519':
          $type = 'Ed25519';
          break;
        default:
          $type = $match[3];
          break;
      }
      return [
        $type,
        $fingerprint,
        $comment
      ];
    }
    return parent::getArrayValue();
  }

  function readFile ($handle)
  {
    $line = fgets($handle);
    if (preg_match($this->format, $line)) {
      $this->postValue = trim($line);
    } else {
      $error = new SimplePluginError($this, htmlescape(_('Unknown public key format!')));
      $error->display();
    }
    @fclose($handle);
  }

  function fingerprint ($data)
  {
    $result = md5($data);
    $result = preg_replace('/(..)/', '\1:', $result);
    return rtrim($result, ':');
  }
}

class sshAccount extends simplePlugin implements UserTabLockingAction
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('SSH'),
      'plDescription'   => _('Edit user\'s SSH public keys'),
      'plSelfModify'    => TRUE,
      'plDepends'       => ['posixAccount'],
      'plPriority'      => 6,
      'plObjectClass'   => ['ldapPublicKey'],
      'plObjectType'    => ['user'],
      'plIcon'          => 'geticon.php?context=applications&icon=ssh&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=ssh&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('SSH Keys'),
        'icon'  => 'geticon.php?context=status&icon=dialog-password&size=16',
        'class' => ['fullwidth'],
        'attrs' => [
          new OrderedArrayAttribute(
            new SSHPublicKeyAttribute(
              '', _('SSH public keys for this user'),
              'sshPublicKey'
            ),
            // no order
            FALSE,
            [],
            // edit enabled
            TRUE
          )
        ]
      ]
    ];
  }

  function check (): array
  {
    $errors = parent::check();

    $fingerprints = [];
    $values = $this->attributesAccess['sshPublicKey']->getValue();
    foreach ($values as $value) {
      $this->attributesAccess['sshPublicKey']->attribute->setValue($value);
      $arrayValue = $this->attributesAccess['sshPublicKey']->attribute->getArrayValue();
      if (in_array($arrayValue[1], $fingerprints)) {
        $errors[] = new SimplePluginCheckError(
          $this->attributesAccess['sshPublicKey'],
          htmlescape(sprintf(_('Error : there are several keys with fingerprint %s'), $arrayValue[1]))
        );
      } else {
        $fingerprints[] = $arrayValue[1];
      }
    }

    return $errors;
  }

  public function fillLockingLDAPAttrs (string $mode, array &$modify)
  {
    if (!isset($this->attrs['sshPublicKey'])) {
      return;
    }
    $modify['sshPublicKey'] = [];
    for ($i = 0; $i < $this->attrs['sshPublicKey']['count']; ++$i) {
      if ($mode == 'LOCK') {
        $modify['sshPublicKey'][] = preg_replace('/^/', 'disabled-', $this->attrs['sshPublicKey'][$i]);
      } else {
        $modify['sshPublicKey'][] = preg_replace('/^disabled-/', '', $this->attrs['sshPublicKey'][$i]);
      }
    }
  }
}
