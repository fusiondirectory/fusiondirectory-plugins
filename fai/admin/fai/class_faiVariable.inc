<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016  FusionDirectory

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

class faiVariable extends faiSimplePluginClass
{
  var $mainTab = TRUE;

  var $objectclasses = array("top","FAIclass","FAIvariable");

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Variable'),
      'plDescription' => _('FAI variable'),
      'plObjectType'  => array(
        'faiVariable' => array(
          'name'        => _('FAI variable'),
          'filter'      => 'objectClass=FAIvariable',
          'aclCategory' => 'fai',
          'ou'          => get_ou('faiVariableRDN'),
          'icon'        => 'geticon.php?context=applications&icon=fai-variable&size=16'
        )
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute (
            _('Class name'), _('Variables class name'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('Short description of the class'),
            'description', FALSE
          ),
        )
      ),
      'vars' => array(
        'name'  => _('Variables'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new SubNodesAttribute (
            '', _('Variables in this class'),
            'variables', array('FAIvariableEntry','FAIclass','top'),
            array(
              new StringAttribute (
                _('Name'), _('The name of the variable'),
                'cn', TRUE
              ),
              new StringAttribute (
                _('Description'), _('A short description of the variable'),
                'description', FALSE
              ),
              new StringAttribute (
                _('Content'), _('The content of the variable'),
                'FAIvariableContent', TRUE
              ),
            ),
            FALSE, /* no order */
            array(),
            TRUE /* edit enabled */
          ),
        )
      )
    );
  }

  function __construct ($unused, $dn = NULL, $object = NULL)
  {
    parent::__construct($unused, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
    $this->attributesAccess['variables']->setLinearRendering(FALSE);
  }

  function ldap_save ($cleanup = TRUE)
  {
    global $config;
    parent::ldap_save($cleanup);
    $this->attributesAccess['variables']->postLdapSave($config->get_ldap_link());
  }
}
?>
