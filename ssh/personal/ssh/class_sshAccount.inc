<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class SSHPublicKeyAttribute extends FileAttribute
{
  /* cache for key extracted informations */
  protected $format = '/^(ssh-(dss|rsa))\s+([a-zA-Z0-9+\/.=]+)\s+([[:print:]]+)$/';

  function getArrayValue ()
  {
    if (preg_match($this->format, $this->value, $match)) {
      $fingerprint = $this->fingerprint(base64_decode($match[3]));

      return array(
        ($match[1] == 'ssh-dss'?'DSA':'RSA'),
        $fingerprint,
        $match[4]
      );
    }
    return parent::getArrayValue();
  }

  function readFile($handle)
  {
    $line = fgets($handle);
    if (preg_match($this->format, $line)) {
      $this->postValue = $line;
    } else {
      msg_dialog::display(_("Upload error"), _("Unknown public key format!"), ERROR_DIALOG);
    }
    @fclose($handle);
  }

  function applyPostValue()
  {
    parent::applyPostValue();
  }

  function fingerprint($data)
  {
    $result = md5($data);
    $result = preg_replace('/(..)/', '\1:', $result);
    return rtrim($result, ':');
  }
}

class sshAccount extends simplePlugin
{
  var $displayHeader = TRUE;
  var $objectclasses = array('ldapPublicKey');

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('SSH Keys'),
        'icon'  => 'images/lists/key.png',
        'class' => array('fullwidth'),
        'attrs' => array(
          new OrderedArrayAttribute(
            new SSHPublicKeyAttribute(
              '', _('SSH public keys for this user'),
              'sshPublicKey'
            ),
            FALSE // no order
          )
        )
      )
    );
  }

  static function plInfo()
  {
    return array(
      'plShortName'     => _('SSH'),
      'plDescription'   => _('Edit user\'s SSH public keys'),
      'plSelfModify'    => TRUE,
      'plDepends'       => array('posixAccount'),
      'plPriority'      => 3,
      'plCategory'      => array('users'),
      'plObjectType'    => array('user'),
      'plIcon'          => 'images/password.png',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function check()
  {
    $message = parent::check();

    $fingerprints = array();
    $values = $this->attributesAccess['sshPublicKey']->getValue();
    foreach ($values as $value) {
      $this->attributesAccess['sshPublicKey']->attribute->setValue($value);
      $arrayValue = $this->attributesAccess['sshPublicKey']->attribute->getArrayValue();
      if (in_array($arrayValue[1],$fingerprints)) {
        $message[] = sprintf(_('Error : there are several keys with fingerprint %s'),$arrayValue[1]);
      } else {
        $fingerprints[] = $arrayValue[1];
      }
    }

    return $message;
  }
}
?>
