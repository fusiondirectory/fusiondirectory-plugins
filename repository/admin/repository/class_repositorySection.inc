<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2013-2016 FusionDirectory

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

class repositorySection extends simplePlugin
{
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Repository section'),
      'plDescription' => _('Repository section'),
      'plObjectClass' => array('fdBuildRepositorySection'),
      'plObjectType'  => array('repoSection' => array(
        'name'        => _('Repository section'),
        'aclCategory' => 'repository',
        'ou'          => get_ou('repositoryRDN'),
        'icon'        => 'geticon.php?context=applications&icon=repository-section&size=16',
      )),
      'plForeignKeys'  => array(
        'fdBasedOn' => array(
          array('repositorySection',  'cn'),
        )
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Repository section'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('repositoryRDN')),
          new StringAttribute (
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

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique('one');

    $sections = objects::ls('repoSection', 'cn');
    $this->attributesAccess['fdBasedOn']->attribute->setChoices(array_values($sections));
  }
}

?>
