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

class TerminalReleaseAttribute extends SelectAttribute
{
  function __construct ($label, $description, $ldapName, $required, $choices = array())
  {
    parent::__construct($label, $description, $ldapName, $required, $choices);
    $this->setSubmitForm(TRUE);
  }

  function setValue ($value)
  {
    if (!in_array($value, $this->choices)) {
      return;
    }
    parent::setValue($value);
    if (isset($this->plugin)) {
      $this->plugin->attributesInfo['main']['attrs']['gotoBootKernel']->setRelease($value);
    }
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    $this->setValue('squeeze');
  }
}

class terminalStartup extends simplePlugin
{
  var $objectclasses = array();

  var $inheritance = array("gosaGroupOfNames" => "member");

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'startup' => array (
        'name' => _('Startup parameters'),
        'attrs' => array (
          new SelectAttribute(
            _('Root server'), _('The root server the terminal should be using'),
            'gotoTerminalPath', FALSE
          ),
          new SelectAttribute(
            _('Swap server'), _('The swap server the terminal should be using'),
            'gotoSwapServer', FALSE
          ),
          new StringAttribute (
            _('Boot kernel'),  _('Linux kernel to be installed and booted'),
            'gotoBootKernel', FALSE
          ),
          new StringAttribute (
            _('Boot kernel parameters'), _('Linux kernel parameters to be ran'),
            'gotoKernelParameters', FALSE
          )
        )
      ),
      'remote_desktop' => array(
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
      ),
    );
  }

  /* Return plugin informations
   * This plugin is deactivated for now
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Startup'),
      'plDescription'   => _('Terminal startup'),
      'plSelfModify'    => FALSE,
      'plPriority'      => 3,
      'plCategory'      => array('terminal'),
      'plObjectType'    => array('terminal', 'ogroup-dynamic'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }*/

  function __construct($unused, $dn = NULL, $object = NULL)
  {
    global $config;
    parent::__construct($unused, $dn, $object);

    $this->attributesAccess['gotoXMethod']->setSubmitForm(TRUE);

    /* Root server */
    $tmp        = $config->data['SERVERS']['NFS'];
    $tmp_output = $tmp;
    if (($key = array_search('default', $tmp)) !== FALSE) {
      if ($this->member_of_group) {
        $tmp[$key] = '';
        $tmp_output[$key] = _('inherited');
      } else {
        unset($tmp[$key]);
        unset($tmp_output[$key]);
      }
    }
    $this->attributesAccess['gotoTerminalPath']->setChoices($tmp, $tmp_output);

    /* Swap server */
    $tmp        = $config->data['SERVERS']['NBD'];
    $tmp_output = $tmp;
    array_unshift($tmp, '!');
    array_unshift($tmp_output, _('Local swap'));
    if (($key = array_search('default', $tmp)) !== FALSE) {
      if ($this->member_of_group) {
        $tmp[$key] = "";
        $tmp_output[$key] = _('inherited');
      } else {
        unset($tmp[$key]);
        unset($tmp_output[$key]);
      }
    }
    $this->attributesAccess['gotoSwapServer']->setChoices($tmp, $tmp_output);
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
      if (in_array(strtoupper($this->attributesAccess['gotoXMethod']), $supported_types)) {
        $available_servers[] = $servername;
      }
    }
    $this->attributesAccess['gotoXdmcpServer']->attribute->setChoices($available_servers);
    return parent::execute();
  }
}

?>
