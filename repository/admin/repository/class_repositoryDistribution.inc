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

class repositoryDistribution extends simplePlugin
{
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Repository distribution'),
      'plDescription' => _('Repository distribution'),
      'plObjectClass' => array('fdBuildRepositoryDistribution'),
      'plObjectType'  => array('repoDistribution' => array(
        'name'        => _('Repository distribution'),
        'aclCategory' => 'repository',
        'ou'          => get_ou('repositoryRDN'),
        'icon'        => 'geticon.php?context=applications&icon=repository-distribution&size=16'
      )),
      'plForeignKeys'  => array(
        'fdBasedOn' => array(
          array('repositoryDistribution', 'cn'),
        ),
        'fdRepoSection' => array(
          array('repositorySection', 'cn'),
        ),
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Repository distribution'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('repositoryRDN')),
          new StringAttribute (
            _('Name'), _('Unique name for this distribution'),
            'cn', TRUE
          ),
          new TextAreaAttribute (
            _('Description'), _('Description of this distribution'),
            'description', FALSE
          ),
          new SetAttribute (
            new SelectAttribute (
              _('Section'), _('The sections this distribution contains'),
              'fdRepoSection', TRUE,
              array()
            )
          ),
          new SetAttribute (
            new SelectAttribute (
              _('Based on'), _('The distributions this one is based on'),
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

    $this->attributesAccess['cn']->setUnique(TRUE);

    $distros = objects::ls('repoDistribution', 'cn');
    $this->attributesAccess['fdBasedOn']->attribute->setChoices(array_values($distros));
    $sections = objects::ls('repoSection', 'cn');
    $this->attributesAccess['fdRepoSection']->attribute->setChoices(array_values($sections));
  }
}

?>
