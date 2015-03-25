<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class FAIRepositoryAttribute extends PipeSeparatedCompositeAttribute
{
  function __construct($description, $ldapName, $acl = "", $label = "Composite attribute")
  {
    parent::__construct(
      $description,
      $ldapName,
      array(
        new StringAttribute(
          _('URL'), _('Repository url'),
          'mirrorUrl', TRUE
        ),
        new SelectAttribute(
          _('Parent server'), _('Parent repository server'),
          'mirrorParent', FALSE,
          array(), ''
        ),
        new StringAttribute(
          _('Release'), _('Release used on this repository'),
          'mirrorRelease', TRUE
        ),
        new CommaListAttribute(
          'mirrorSections',
          new SetAttribute(
            new StringAttribute(
              _('Sections'), _('Sections available on this repository'), 'mirrorSections_inner'
            )
          )
        ),
        new SelectAttribute(
          _('Mirror mode'), _('Is this mirror an installation repository? Is its release a custom one?'),
          'mirrorMode', TRUE,
          array('install','update','custom')
        ),
        new BooleanAttribute(
          _('Local mirror'), _('Is this mirror a local or a network mirror?'),
          'mirrorLocal', FALSE,
          FALSE, '',
          'local', 'network'
        ),
        new CommaListAttribute(
          'mirrorArchs',
          new SetAttribute(
            new StringAttribute(
              _('Architectures'), _('Processor architectures available on this repository'),
              'mirrorArchs_inner', TRUE
            )
          )
        ),
        new SelectAttribute(
          _('Distribution'), _('Which distribution is this repository for'),
          'distribution', TRUE,
          array('debian','centos')
        ),
      ),
      $acl,
      $label
    );
    $this->attributes[7]->setSubmitForm(TRUE);
    $this->updateFields();
  }

  function updateFields()
  {
    $dn = NULL;
    if (is_object($this->plugin)) {
      $dn = $this->plugin->dn;
    }
    $this->attributes[1]->setChoices(serviceRepository::getParentServers($this->attributes[7]->getValue(), $dn));
    if ($this->attributes[7]->getValue() == 'debian') {
      $releaseList = array_map(
        function ($a)
        {
          return "custom:$a";
        },
        serviceRepository::getReleaseList($this->attributes[7]->getValue())
      );
      $modeChoices = array_merge(array('install', 'update'), $releaseList);
    } else {
      $modeChoices = array('install', 'update');
    }
    $this->attributes[4]->setChoices($modeChoices);
  }

  function applyPostValue()
  {
    parent::applyPostValue();
    $this->updateFields();
  }

  function setValue ($values)
  {
    $arrayValues = $values;
    if (!is_array($values)) {
      $arrayValues = $this->inputValue($values);
    }
    $this->attributes[7]->setValue($arrayValues[7]);
    $this->updateFields();
    parent::setValue($values);
  }

  function resetToDefault ()
  {
    $this->attributes[7]->resetToDefault();
    $this->updateFields();
    parent::resetToDefault();
  }
}

class serviceRepository extends simpleService
{
  var $objectclasses  = array('FAIrepositoryServer');

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'template'  => get_template_path('fai_repository.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Repositories'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new OrderedArrayAttribute(
            new FAIRepositoryAttribute(
              _('Repositories this server hosts'),
              'FAIrepository'
            ),
            FALSE,  // no order
            array(),
            TRUE    // edit button
          )
        )
      )
    );
  }

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Package repository'),
      'plDescription' => _('Package repository information'),
      'plIcon'        => 'geticon.php?context=mimetypes&icon=package-x-generic&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->attributesAccess['FAIrepository']->setLinearRendering(FALSE);
  }

  static function isOfDistribution($attrs, $distribution)
  {
    if (!isset($attrs['FAIrepository'])) {
      return FALSE;
    }
    if (preg_match('/'.preg_quote($distribution, '/').'$/', $attrs['FAIrepository'][0])) {
      return TRUE;
    } else {
      return FALSE;
    }
  }

  function ldap_save($clean = TRUE)
  {
    parent::ldap_save($clean);

    $repos  = $this->attributesAccess['FAIrepository']->getArrayValues();
    $done   = array();
    foreach ($repos as $repo) {
      $release = $repo[2];
      if (($repo[4] == 'install') && (!isset($done[$release]))) {
        FAI::createBranch($repo[7], $release);
        $done[$release] = TRUE;
      }
    }
  }

  static function getParentServers($distribution, $cn = NULL)
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=FAIrepositoryServer)(FAIrepository=*|'.$distribution.'))', array('dn','cn'));

    $ret = array();
    while ($attr = $ldap->fetch()) {
      if ($attr['dn'] == $dn) {
        continue;
      }
      $ret[$attr['cn'][0]] = $attr['cn'][0];
    }

    asort($ret);
    return $ret;
  }

  static function getReleaseList ($distribution)
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->ls('(objectClass=FAIbranch)', 'ou='.$distribution.','.get_ou('faiBaseRDN').$config->current['BASE'], array('ou'));

    $list = array();
    while ($release = $ldap->fetch()) {
      $list[] = $release['ou'][0];
    }
    return $list;
  }
}
?>
