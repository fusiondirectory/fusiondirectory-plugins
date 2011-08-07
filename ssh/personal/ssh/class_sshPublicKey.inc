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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/


class sshPublicKey
{
  var $config= null;
  var $publicKeys= array();
  var $storedPublicKeys= array();
  var $modified= false;
  var $dn;
  var $dialog= false;
  var $enabled= false;
  var $acl;

  function sshPublicKey(&$config, $dn, $acl)
  {
    /* Configuration is fine, always */
    $this->config= &$config;
    $this->dn= $dn;
    $this->acl= $acl;

    // Load list of public keys
    $data= array();
    $ldap= $this->config->get_ldap_link();
    $ldap->cat($this->dn, array('objectClass'));
    if ($attrs= $ldap->fetch()){
      if(in_array_ics('ldapPublicKey', $attrs['objectClass'])){
        $this->enabled= true;
        $data= $ldap->get_attribute($this->dn, "sshPublicKey", 1);
        if(is_array($data)){
          unset($data['count']);
        } 
      }
    }

    // Analyze keys for type, bits and comment
    foreach ($data as $key) {
      list($type, $data, $comment)= preg_split('/\s/', $key);
      $this->publicKeys[]= array("type" => $type,
                                 "fingerprint" => $this->fingerprint(base64_decode($data)),
                                 "comment" => $comment,
                                 "data" => $key);
    }

    // Save copy for later usage
    $this->storedPublicKeys= $this->publicKeys;
  }


  function setDN($dn)
  {
    $this->dn= $dn;
  }


  function execute()
  {
    global $ui;

    // Check if we need to open a dialog
    if (isset($_POST['edit_sshpublickey'])){
      $this->dialog= true;
    }
    if (isset($_POST['cancel_sshpublickey'])){
      $this->dialog= false;
      if ($this->modified) {
        $this->publicKeys= $this->storedPublicKeys;
      }
      $this->modified= false;
    }

    if (isset($_POST['save_sshpublickey'])){
      $this->dialog= false;
      if ($this->modified) {
        $this->storedPublicKeys= $this->publicKeys;
      }
    }

    // If we do not need the dialog, don't show it
    if (!$this->dialog) {
      return null;
    }

    // Remove action?
    if (isset($_POST['remove_sshpublickey']) && isset($_POST['keylist']) && preg_match('/w/', $this->acl)){
      foreach($_POST['keylist'] as $index){
        if (isset($this->publicKeys[$index])){
          unset($this->publicKeys[$index]);
          $this->modified= true;
        }
      }
      $this->publicKeys= array_values($this->publicKeys);
    }

    // Upload action?
    if (isset($_POST['upload_sshpublickey']) && preg_match('/w/', $this->acl)) {
      if ($_FILES['key']['error'] > 0){
        msg_dialog::display(_("Upload error"), _("Error uploading the key!")." (".$_FILES['key']['error'].")", ERROR_DIALOG);
      } else {

        $lines= file($_FILES['key']['tmp_name']);
        foreach ($lines as $line) {
          if (preg_match('/^(ssh-(dss|rsa))\s+([a-zA-Z0-9+\/.=]+)\s+([[:print:]]+)$/', $line, $match)) {
            $fingerprint= $this->fingerprint(base64_decode($match[3]));

            // Check if we already have it
            $found= false;
            foreach ($this->publicKeys as $key) {
              if ($key['fingerprint'] == $fingerprint) {
                $found= true;
                msg_dialog::display(_("Upload error"), _("This key is already used!"), ERROR_DIALOG);
                break;
              }
            }

            // If not used, just add it
            if (!$found) {
              $this->publicKeys[]= array("type" => $match[1],
                                   "fingerprint" => $fingerprint,
                                   "comment" => $match[4],
                                   "data" => $line);
               $this->modified= true;
            }

          } else {
            msg_dialog::display(_("Upload error"), _("Unknown public key format!"), ERROR_DIALOG);
          }
        }
      }
    }

    // Show the ssh page now
    $smarty= get_smarty();
    $data= array();
    foreach ($this->publicKeys as $index => $info) {
      $data[$index]= sprintf(_("SSH %s key, Fingerprint: %s, Comment: %s"), $info['type']=='ssh-dss'?"DSA":"RSA", $info['fingerprint'], $info['comment']);
    }
    $smarty->assign("keylist", $data);
    $smarty->assign("sshPublicKeyACL", $this->acl);
    return $smarty->fetch (get_template_path('sshPublicKey.tpl', TRUE, dirname(__FILE__)));
  }


  function save()
  {
    if ($this->modified) {
      $attrs= array();
      $ldap= $this->config->get_ldap_link();

      // SSH stuff removed?
      if (count($this->publicKeys) == 0) {
        
        $ldap->cat($this->dn, array("objectClass", "sshPublicKey"));
        $nattrs= $ldap->fetch();
        $attrs['objectClass']= array_remove_entries_ics(array("ldapPublicKey"), $nattrs['objectClass']);
        unset($attrs['objectClass']['count']);
        if (isset($nattrs['sshPublicKey'])){
          $attrs['sshPublicKey']= array();
        }
         
        $ldap->cd($this->dn);
        $ldap->modify($attrs);
        new log("modify","posix/ssh",$this->dn,array_keys($attrs),$ldap->get_error());

      } else {

        // If it was enabled before, we just need to update the
        // attributes, elseways modify objectclasses, too.
        if (!$this->enabled) {
          $ldap->cat($this->dn, array("objectClass"));
          $nattrs= $ldap->fetch();
          $attrs['objectClass']= $nattrs['objectClass'];
          unset($attrs['objectClass']['count']);
          $attrs['objectClass'][]= "ldapPublicKey";
        }
        
        // Save public key
        $attrs['sshPublicKey']= array();
        foreach($this->publicKeys as $key) {
          $attrs['sshPublicKey'][]= $key['data'];
        }

        $ldap->cd($this->dn);
        $ldap->modify($attrs);
        new log("modify","posix/ssh",$this->dn,array_keys($attrs),$ldap->get_error());
      }

      // LDAP error?
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, ERROR_DIALOG));
      }
     
    }
  }


  function fingerprint($data)
  {
    $result= md5($data);
    $result= preg_replace('/(..)/', '\1:', $result);
    return rtrim($result, ':');
  }


}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
