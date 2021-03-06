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

class terminalStartup extends simplePlugin
{
  var $inheritance = ['gosaGroupOfNames' => 'member'];

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Startup'),
      'plDescription'   => _('Terminal startup'),
      'plSmallIcon'     => 'geticon.php?context=devices&icon=terminal&size=16',
      'plPriority'      => 2,
      'plObjectClass'   => ['fdTerminalStartup'],
      'plObjectType'    => ['terminal', 'ogroup-dynamic'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    return [
      'startup' => [
        'name' => _('Startup parameters'),
        'attrs' => [
          new SelectAttribute(
            _('Root server'), _('The root server the terminal should be using'),
            'gotoTerminalPath', FALSE
          ),
          new SelectAttribute(
            _('Swap server'), _('The swap server the terminal should be using'),
            'gotoSwapServer', FALSE
          ),
        ]
      ],
      'remote_desktop' => [
        'name'  => _('Remote desktop'),
        'icon'  => 'geticon.php?context=devices&icon=server&size=16',
        'attrs' => [
          new SelectAttribute(
            _('Connect method'), _('Connect method'),
            'gotoXMethod', FALSE,
            ['XDMCP', 'LDM', 'SHELL', 'TELNET', 'RDP'],
            '',
            [_('XDMCP'), _('LDM'), _('Shell'), _('Telnet'), _('Windows RDP')]
          ),
          new SelectAttribute(
            _('Terminal server'), _('Terminal server'),
            'gotoXdmcpServer', FALSE,
            []
          )
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['gotoXMethod']->setSubmitForm('updateServerList');

    /* Get NFS and NBD server lists */
    $nfs = [];
    $nbd = [];
    $shareServers = objects::ls('server', ['cn' => 1, 'goExportEntry' => '*'], NULL, '(&(objectClass=goShareServer)(goExportEntry=*))');
    foreach ($shareServers as $server) {
      foreach ($server['goExportEntry'] as $entry) {
        $parts = explode('|', $entry);
        if ($parts[2] == 'NFS') {
          $nfs[] = $server['cn'].':'.$parts[4];
        } elseif ($parts[2] == 'NBD') {
          $nbd[] = $server['cn'].':'.$parts[4];
        }
      }
    }

    /* Root server */
    $tmp        = $nfs;
    $tmp_output = $nfs;
    if ($this->member_of_group) {
      array_unshift($tmp, '');
      array_unshift($tmp_output, _('inherited'));
    }
    $this->attributesAccess['gotoTerminalPath']->setChoices($tmp, $tmp_output);

    /* Swap server */
    $tmp        = $nbd;
    $tmp_output = $nbd;
    array_unshift($tmp, '!');
    array_unshift($tmp_output, _('Local swap'));
    if ($this->member_of_group) {
      array_unshift($tmp, '');
      array_unshift($tmp_output, _('inherited'));
    }
    $this->attributesAccess['gotoSwapServer']->setChoices($tmp, $tmp_output);

    $this->updateServerList();
  }

  /*! \brief Create a list of useable servers for the currently selected protocol */
  function updateServerList ()
  {
    if (empty($this->gotoXMethod)) {
      $servers = [];
    } else {
      $servers = objects::ls('server', NULL, NULL, '(&(objectClass=goTerminalServer)(gotoSessionType='.ldap_escape_f(strtoupper($this->gotoXMethod)).'))');
    }
    $this->attributesAccess['gotoXdmcpServer']->setChoices($servers);
  }
}
