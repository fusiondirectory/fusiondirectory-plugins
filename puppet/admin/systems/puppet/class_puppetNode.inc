<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2018  FusionDirectory

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

class puppetNode extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array("puppetClient");

  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'section1' => array(
        'name'  => _("Puppet node settings"),
        'attrs' => array(
          new SetAttribute (
            new StringAttribute (_("Puppet class"), _("Puppet Node Class"), "puppetClass")
          ),
          new SelectAttribute (_("Parent node"), _("Puppet Parent Node"),       "parentNode", FALSE),
          new SelectAttribute (_("Environment"), _("Puppet Node Environment"),  "environment", TRUE),
          new SetAttribute (
            new CompositeAttribute (
              _("A variable setting for puppet"),
              'puppetVar',
              array(
                new StringAttribute ('',   _('Name of the variable'),   'puppetVar'),
                new StringAttribute ('=',  _('Value of the variable'),  'puppetVarValue'),
              ),
               // sscanf format
              '%[^ =]=%[^ ]',
              // sprintf format
              '%s=%s',
              // acl, empty string means we don't care (handle it automagically)
              '',
              'Puppet Vars'
            )
          ),
        )
      ),
    );
  }

  // We also need this function that return some information about the plugin
  static function plInfo ()
  {
    return array(
      "plShortName"       => _("Puppet"),
      "plDescription"     => _("Support for puppet schema in order to edit puppet classes and puppet vars"),
      "plSelfModify"      => FALSE,
      "plObjectType"      => array("server", "workstation"),

      // But simplePlugin can generate the ACL list for us
      "plProvidedAcls"    => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=puppetClient)', array('cn'));
    $nodes = array();
    while ($attrs = $ldap->fetch()) {
      $nodes[] = $attrs['cn'][0];
    }
    $this->attributesAccess['parentNode']->setChoices($nodes);
    $ldap->search('(objectClass=puppetServer)', array('puppetAvailableEnvironment'));
    $envs = array();
    if ($attrs = $ldap->fetch()) {
      $envs = $attrs['puppetAvailableEnvironment'];
      unset($envs['count']);
    }
    $this->attributesAccess['environment']->setChoices($envs);
  }

  public function getDisplayHeaderInfos()
  {
    list($disabled,$buttonText,$text) = parent::getDisplayHeaderInfos();
    if (!$this->is_account && !$disabled && empty($this->attributesAccess['environment']->getChoices())) {
      $disabled = TRUE;
      $text     = _('You need to add the puppet service to a server to be able to use this tab');
    }
    return array($disabled,$buttonText,$text);
  }
}

?>
