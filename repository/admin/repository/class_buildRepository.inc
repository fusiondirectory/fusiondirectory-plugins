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
      'plForeignKeys'  => array(
        'fdRepoAdmin'     => 'user',
        'fdRepoUploader'  => 'user',
        'fdRepoUser'      => 'user',
        'fdRepoDistributionSection'   => array(
          array('repositoryDistribution', 'cn', '(fdRepoDistributionSection=%oldvalue%|*)'),
          array('repositorySection',      'cn', '(fdRepoDistributionSection=*|%oldvalue%)'),
        ),
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    global $config;

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
            _('Description'), _('Description of this repository'),
            'description', FALSE
          ),
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
          ),
          new SelectAttribute (
            _('Type'), _('Repository type'),
            'fdRepoType', TRUE,
            $config->get_cfg_value('repositoryTypes', array('debian'))
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
    $choices = array();
    foreach ($distros as $infos) {
      foreach ($infos['fdRepoSection'] as $section) {
        $choices[] = $infos['cn'].'|'.$section;
      }
    }
    $this->attributesAccess['fdRepoDistributionSection']->attribute->setChoices(array_values($choices));
  }

  function foreignKeyCheck ($field, $fieldvalue, $source)
  {
    if ($field == 'fdRepoDistributionSection') {
      $values = $this->fdRepoDistributionSection;
      foreach ($values as $value) {
        if (($source['CLASS'] == 'repositoryDistribution') && preg_match('/^'.preg_quote($fieldvalue, '/').'\|/', $value)) {
          return TRUE;
        }
        if (($source['CLASS'] == 'repositorySection') && preg_match('/\|'.preg_quote($fieldvalue, '/').'$/', $value)) {
          return TRUE;
        }
      }
    } else {
      return parent::foreignKeyCheck ($field, $fieldvalue, $source);
    }
  }

  function foreignKeyUpdate($field, $oldvalue, $newvalue, $source)
  {
    if ($field == 'fdRepoDistributionSection') {
      $values = $this->fdRepoDistributionSection;
      foreach ($values as $key => $value) {
        if (($source['CLASS'] == 'repositoryDistribution') && preg_match('/^'.preg_quote($oldvalue, '/').'\|/', $value)) {
          if ($newvalue === NULL) {
            unset($values[$key]);
          } else {
            $values[$key] = preg_replace('/^'.preg_quote($oldvalue, '/').'/', $newvalue, $value);
          }
        }
        if (($source['CLASS'] == 'repositorySection') && preg_match('/\|'.preg_quote($oldvalue, '/').'$/', $value)) {
          if ($newvalue === NULL) {
            unset($values[$key]);
          } else {
            $values[$key] = preg_replace('/'.preg_quote($oldvalue, '/').'$/', $newvalue, $value);
          }
        }
      }
      $this->fdRepoDistributionSection = $values;
    } else {
      return parent::foreignKeyUpdate($field, $oldvalue, $newvalue, $source);
    }
  }
}

?>
