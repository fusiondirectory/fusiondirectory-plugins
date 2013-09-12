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

class buildRepository extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('fdBuildRepository');

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Build repository'),
      'plDescription' => _('Build repository'),
      'plObjectType'  => array('buildRepository' => array(
        'name'        => _('Build repository'),
        'filter'      => 'objectClass=fdBuildRepository',
        'aclCategory' => 'repository',
        'ou'          => get_ou('repositoryRDN'),
        'icon'        => 'plugins/repository/images/iconMini.png'
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Build repository'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('repositoryRDN')),
          new HostNameAttribute (
            _('Name'), _('Unique name for this repository'),
            'cn', TRUE
          ),
          new TextAreaAttribute (
            _('description'), _('description'),
            'description', FALSE
          ),
          new HiddenAttribute('fdRepoType', TRUE, 'debian'),
          new SetAttribute (
            new SelectAttribute (
              _('Distribution sections'), _('The distribution sections this repository provides'),
              'fdRepoDistributionSection', TRUE,
              array()
            )
          ),
          new BooleanAttribute (
            _('Private'), _('Is this repository private or public?'),
            'fdRepoPrivate', FALSE
          )
        )
      ),
      'users' => array(
        'name'  => _('Members'),
        'attrs' => array(
          new UsersAttribute (
            _('Admins'), _('Admins of this repository'),
            'fdRepoAdmin', FALSE
          ),
          new UsersAttribute(
            _('Uploaders'), _('Uploaders of this repository'),
            'fdRepoUploader', FALSE
          ),
          new UsersAttribute(
            _('Users'), _('Users of this repository'),
            'fdRepoUser', FALSE
          ),
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);

    $distros = objects::ls('repoDistribution', array('cn' => '1', 'fdRepoSection' => '*'));
    print_r($distros);
    $choices = array();
    foreach ($distros as $infos) {
      foreach ($infos['fdRepoSection'] as $section) {
        $choices[] = $infos['cn'].'/'.$section;
      }
    }
    $this->attributesAccess['fdRepoDistributionSection']->attribute->setChoices(array_values($choices));
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'cn='.$this->attributesAccess['cn']->getValue().','.get_ou('repositoryRDN').$this->base;
  }
}

?>
