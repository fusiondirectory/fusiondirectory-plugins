<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2017  FusionDirectory

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

require_once 'PEAR.php';
require_once dirname(__FILE__) . '/Sieve.php';

class mailMethodSieve extends mailMethod
{
  protected $sieve_handle = NULL;

  static public function serverMatch ($server)
  {
    return isset(static::get_server_list()[$server]);
  }

  protected function init ()
  {
    parent::init();
  }

  public function connect (): bool
  {
    parent::connect();
    $this->connected = FALSE;

    $cfg = $this->getServerConfig();
    if ($cfg === FALSE) {
      return FALSE;
    }

    // Parse connect string: {hostname:port/options}
    if (preg_match('/^\{([^:]+):(\d+)\/([^}]+)\}$/', $cfg['connect'], $matches)) {
      $host     = $matches[1];
      $port     = $matches[2];
      $options  = $matches[3];
    } else {
      $this->error = 'Invalid connect string';
      return FALSE;
    }

    // Create Net_Sieve instance
    $this->sieve_handle = new Net_Sieve();
    
    // Connect to server
    $result = $this->sieve_handle->connect($host, $port);
    if (PEAR::isError($result)) {
      $this->error = 'Could not connect to Sieve server: ' . $result->getMessage();
      return FALSE;
    }
    
    // Login using admin credentials and proxy authentication to user if needed
    $result = $this->sieve_handle->login($cfg['admin'], $cfg['password'], $options, $this->getUAttribValue());
    if (PEAR::isError($result)) {
      $this->error = 'Sieve authentication failed: ' . $result->getMessage();
      return FALSE;
    }

    $this->connected = TRUE;
    return TRUE;
  }

  public function disconnect ()
  {
    parent::disconnect();
    if ($this->sieve_handle) {
      $this->sieve_handle->disconnect();
    }
  }

  public function is_connected ()
  {
    $ret = parent::is_connected();
    return ($ret && $this->sieve_handle && !PEAR::isError($this->sieve_handle));
  }

  public function saveSieveSettings ()
  {
    $this->reset_error();

    if (!$this->is_connected()) {
      $this->error = 'Not connected to sieve server';
      return FALSE;
    }

    // Get sieve settings from parent (mailAccount)
    $spamfilter = $this->parent->attributesAccess['flag_spamfilter']->getValue() ?? FALSE;
    $spamlevel = $this->parent->attributesAccess['spamlevel']->getValue() ?? 5;
    $spambox = $this->parent->attributesAccess['spambox']->getValue() ?? 'INBOX.Spam';
    $sizefilter = $this->parent->attributesAccess['flag_sizefilter']->getValue() ?? FALSE;
    $maxsize = $this->parent->attributesAccess['maxsize']->getValue() ?? 10;
    $vacation = $this->parent->attributesAccess['flag_vacation']->getValue() ?? FALSE;
    $vacmsg = $this->parent->attributesAccess['gosaVacationMessage']->getValue() ?? '';
    $addrlist = $this->parent->attributesAccess['vacation_addresses']->getValue() ?? [];
    $dropownmail = $this->parent->attributesAccess['flag_ownmailbox']->getValue() ?? FALSE;

    // Generate sieve script
    $script = $this->generateSieveScript([
      'spamfilter' => $spamfilter ? 'true' : 'false',
      'spamlevel' => $spamlevel,
      'spambox' => $spambox,
      'sizefilter' => $sizefilter ? 'true' : 'false',
      'maxsize' => $maxsize,
      'vacation' => $vacation ? 'true' : 'false',
      'vacmsg' => addslashes($vacmsg),
      'addrlist' => implode('","', $addrlist),
      'dropownmail' => $dropownmail ? 'true' : 'false',
    ]);

    // Upload script using Net_Sieve
    $result = $this->sieve_handle->installScript("fusiondirectory", $script, true);
    if (PEAR::isError($result)) {
      $this->error = sprintf(_('Cannot store or activate SIEVE script: %s'), $result->getMessage());
      return FALSE;
    }

    return TRUE;
  }

  protected function generateSieveScript ($vars)
  {
    $script = "###FUSIONDIRECTORY\nrequire [\"fileinto\", \"reject\", \"vacation\"];\n\n";

    if ($vars['spamfilter'] === 'true') {
      $script .= "# Sort mails with higher spam level\n";
      $script .= "if header :contains \"X-Spam-Level\" \"{$vars['spamlevel']}\" {\n";
      $script .= "  fileinto \"{$vars['spambox']}\";\n";
      $script .= "}\n\n";
    }

    if ($vars['sizefilter'] === 'true') {
      $script .= "# Reject mails with bigger size\n";
      $script .= "if size :over {$vars['maxsize']}M {\n";
      $script .= "  reject text:\n";
      $script .= "{t}Dear sender,\n\n";
      $script .= "the mail you sent to our mailsystem has been rejected due\n";
      $script .= "to a user configured maximum mail size ({$vars['maxsize']} MB).\n\n";
      $script .= "Either ask the user to remove the sizelimit, or send smaller pieces.\n\n";
      $script .= "Thank you,\n";
      $script .= " the mailserver{/t}\n";
      $script .= ".\n";
      $script .= "  ;\n";
      $script .= "  discard;\n";
      $script .= "}\n\n";
    }

    if ($vars['vacation'] === 'true') {
      $script .= "# Vacation message\n";
      $script .= "vacation :addresses [\"{$vars['addrlist']}\"] \"{$vars['vacmsg']}\";\n\n";
    }

    if ($vars['dropownmail'] === 'true') {
      $script .= "# Do not deliver to own mailbox\n";
      $script .= "discard;\n\n";
    }

    return $script;
  }

  static public function get_server_list ()
  {
    global $config;
    $serverList = [];

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=fdSieveServer)',
                  [
                    'cn','fdSieveConnect','fdSieveAdmin','fdSievePassword'
                  ]);
    while ($attrs = $ldap->fetch()) {
      $serverList[$attrs['cn'][0]] = [
        'server_dn'     => $attrs['dn'],
        'connect'       => $attrs['fdSieveConnect'][0],
        'admin'         => $attrs['fdSieveAdmin'][0],
        'password'      => $attrs['fdSievePassword'][0]
      ];
    }

    return $serverList;
  }
}
