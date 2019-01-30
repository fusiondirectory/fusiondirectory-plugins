<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2012-2016 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class sympaAlias extends simplePlugin
{
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Sympa list alias'),
      'plDescription' => _('Sympa list alias'),
      'plObjectClass' => array('sympaAlias'),
      'plObjectType'  => array('sympaAlias' => array(
        'name'        => _('Sympa list alias'),
        'icon'        => 'geticon.php?context=applications&icon=sympa&size=16',
        'ou'          => get_ou('sympaRDN'),
      )),
      'plSearchAttrs' => array('description'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('Sympa list alias'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('sympaRDN')),
          new StringAttribute (_('Name'), _('Name to identify this alias'), 'cn', TRUE),
          new TextAreaAttribute (
            _('Description'), _('Description of this alias'),
            'description', FALSE
          ),
          new SetAttribute(
            new MailAttribute (
              _('Email address'), _('Email address'),
              'mail', TRUE
            )
          ),
          new SelectAttribute (
            _('Sympa server'), _('Sympa server for this alias'),
            'sympaServerURL', TRUE,
            array()
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=sympaServer)', array('sympaServerURL'));
    $server = array();
    while ($attrs = $ldap->fetch()) {
        $server[] = $attrs['sympaServerURL'][0];
    }
    $this->attributesInfo['main']['attrs']['sympaServerURL']->setChoices($server);
  }
}

?>
