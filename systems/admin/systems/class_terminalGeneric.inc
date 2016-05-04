<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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

class terminalGeneric extends workstationGeneric
{
  var $objectclasses = array('top', 'gotoTerminal', 'GOhard');

  var $inheritance = array('gotoWorkstationTemplate' => 'member', 'gotoTerminalTemplate' => 'member');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Terminal'),
      'plDescription' => _('Terminal information'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('terminal' =>
        array(
          'name'        => _('Terminal'),
          'description' => _('Terminal'),
          'filter'      => 'objectClass=gotoTerminal',
          'icon'        => 'geticon.php?context=devices&icon=terminal&size=16',
          'ou'          => get_ou('terminalRDN'),
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

<<<<<<< HEAD
  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ($word = NULL, $rdn = NULL)
  {
    if ($word === NULL) {
      $word = _('terminal');
    }
    if ($rdn === NULL) {
      $rdn = get_ou('terminalRDN');
    }
    $attributesInfo = parent::getAttributesInfo($word, $rdn);
    $attributesInfo['servers'] = array(
      'name'  => _('Servers'),
      'icon'  => 'geticon.php?context=devices&icon=server&size=16',
      'attrs' => array(
        new SelectAttribute(
          _('Root server'),
          sprintf(_('The root server the terminal should be using'), $word),
          'gotoTerminalPath',
          FALSE
        ),
        new SelectAttribute(
          _('Swap server'),
          sprintf(_('The swap server the terminal should be using'), $word),
          'gotoSwapServer',
          FALSE
        )
      )
    );
    $attributesInfo['remote_desktop'] = array(
      'name'  => _('Remote desktop'),
      'icon'  => 'geticon.php?context=devices&icon=server&size=16',
      'attrs' => array(
        new SelectAttribute(
          _('Connect method'), _('Connect method'),
          'gotoXMethod', FALSE,
          array('xdmcp', 'ldm', 'shell', 'telnet', 'rdp'),
          '',
          array(_('XDMCP'), _('LDM'), _('Shell'), _('Telnet'), _('Windows RDP'))
        ),
        new SetAttribute(
          new SelectAttribute(
            _('Terminal server'), _('Terminal server'),
            'gotoXdmcpServer', FALSE,
            array()
          )
        )
      )
    );
    $attributesInfo['remote_desktop']['attrs'][0]->setSubmitForm(TRUE);
    return $attributesInfo;
  }

  function __construct ($dn = NULL, $object = NULL)
  {
    global $config;
    parent::__construct($dn, $object);

    /* Root server */
    $tmp        = $config->data['SERVERS']['NFS'];
    $tmp_output = $tmp;
    if (($key = array_search("default", $tmp)) !== FALSE) {
      if ($this->member_of_group) {
        $tmp[$key] = "";
        $tmp_output[$key] = _('inherited');
      } else {
        unset($tmp[$key]);
        unset($tmp_output[$key]);
      }
    }
    $this->attributesInfo['servers']['attrs']['gotoTerminalPath']->setChoices($tmp, $tmp_output);

    /* Swap server */
    $tmp        = $config->data['SERVERS']['NBD'];
    $tmp_output = $tmp;
    array_unshift($tmp, '!');
    array_unshift($tmp_output, _('Local swap'));
    if (($key = array_search("default", $tmp)) !== FALSE) {
      if ($this->member_of_group) {
        $tmp[$key] = "";
        $tmp_output[$key] = _('inherited');
      } else {
        unset($tmp[$key]);
        unset($tmp_output[$key]);
      }
    }
    $this->attributesInfo['servers']['attrs']['gotoSwapServer']->setChoices($tmp, $tmp_output);
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'cn='.ldap_escape_dn($this->cn).','.get_ou('terminalRDN').$this->base;
  }

  function execute()
  {
    global $config;
    /* Create a list of useable servers for the currently selected
     *  connection type.
     */
    $xdmcp_types        = $config->data['SERVERS']['TERMINAL_SESSION_TYPES'];
    $available_servers  = array();
    foreach ($xdmcp_types as $servername => $supported_types) {
      if (in_array(strtoupper($this->attributesInfo['remote_desktop']['attrs']['gotoXMethod']), $supported_types)) {
        $available_servers[] = $servername;
      }
    }
    $this->attributesInfo['remote_desktop']['attrs']['gotoXdmcpServer']->attribute->setChoices($available_servers);
    return parent::execute();
  }

}

?>
