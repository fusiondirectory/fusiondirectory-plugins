<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2013 FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class repositorySection extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('fdBuildRepositorySection');

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Repository section'),
      'plDescription' => _('Repository section'),
      'plObjectType'  => array('repoSection' => array(
        'name'        => _('Repository section'),
        'filter'      => 'objectClass=fdBuildRepositorySection',
        'aclCategory' => 'repository',
        'ou'          => get_ou('repositoryRDN'),
        'icon'        => 'plugins/repository/images/iconSection.png'
      )),
      'plForeignKeys'  => array(
        'fdBasedOn' => array(
          array('repositorySection',  'cn'),
        )
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Repository section'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('repositoryRDN')),
          new HostNameAttribute (
            _('Name'), _('Unique name for this section'),
            'cn', TRUE
          ),
          new TextAreaAttribute (
            _('Description'), _('Description of this section'),
            'description', FALSE
          ),
          new SetAttribute (
            new SelectAttribute (
              _('Based on'), _('The sections this one is based on'),
              'fdBasedOn', FALSE,
              array()
            )
          ),
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);

    $sections = objects::ls('repoSection', 'cn');
    $this->attributesAccess['fdBasedOn']->attribute->setChoices(array_values($sections));
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'cn='.$this->attributesAccess['cn']->getValue().','.get_ou('repositoryRDN').$this->base;
  }
}

?>
