<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2017  FusionDirectory

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

class mailMethodRenaterPartage extends mailMethod
{
  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail   = FALSE;

  /* Allow modification of the mail server attribute existing mail accounts */
  protected $modifyableServer = FALSE;

  protected $enableQuota            = TRUE;
  protected $enableVacationRange    = FALSE;
  protected $enableFolderTypes      = FALSE;

  public function updateMailbox()
  {
    mailMethod::updateMailbox();

    return TRUE;
  }

  public function deleteMailbox()
  {
    mailMethod::deleteMailbox();

    return TRUE;
  }

  static public function get_server_list()
  {
    global $config;
    $serverList = array();

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search ('(&(objectClass=fdRenaterPartageServer)(fdRenaterPartageServerMailDomain=*))',
                  array('cn', 'fdRenaterPartageServerUri', 'fdRenaterPartageServerMailDomain'));
    while ($attrs = $ldap->fetch()) {
      unset($attrs['fdRenaterPartageServerMailDomain']['count']);
      foreach($attrs['fdRenaterPartageServerMailDomain'] as $domainAndKey) {
        list($domain, $key) = explode(':', $domainAndKey, 2);
        $serverList[$attrs['cn'][0].'-'.$domain] = array(
          'server_dn' => $attrs['dn'],
          'uri'       => $attrs['fdRenaterPartageServerUri'][0],
          'domain'    => $domain,
          'key'       => $key,
        );
      }
    }

    return $serverList;
  }
}
?>
