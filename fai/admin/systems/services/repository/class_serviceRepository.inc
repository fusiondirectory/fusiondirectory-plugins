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
        new StringAttribute(
          _('Path pattern'), _('How this repository is organized (only for RPM repositories)'),
          'pathMask', FALSE,
          '%RELEASE%/%SECTION%/%ARCH%/'
        ),
      ),
      $acl,
      $label
    );
    $this->attributes[7]->setSubmitForm(TRUE);
  }

  function setParent(&$plugin)
  {
    $this->updateFields();
    parent::setParent($plugin);
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
      $this->attributes[8]->setVisible(FALSE);
    } else {
      $modeChoices = array('install', 'update');
      $this->attributes[8]->setVisible(TRUE);
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

  function readValues($value)
  {
    $values = parent::readValues($value);
    if (!isset($values[8])) {
      $values[8] = '%RELEASE%/%SECTION%/%ARCH%/';
    }
    return $values;
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
  protected static $showActions = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Package repository'),
      'plDescription' => _('Package repository information'),
      'plIcon'        => 'geticon.php?context=mimetypes&icon=package-x-generic&size=16',
      'plObjectClass' => array('FAIrepositoryServer'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

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
            // no order
            FALSE,
            array(),
            // edit button
            TRUE
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $parent = NULL)
  {
    parent::__construct($dn, $parent);

    $this->attributesAccess['FAIrepository']->setLinearRendering(FALSE);
  }

  static function isOfDistribution($attrs, $distribution)
  {
    if (!isset($attrs['FAIrepository'])) {
      return FALSE;
    }
    return preg_match('/'.preg_quote($distribution, '/').'$/', $attrs['FAIrepository'][0]);
  }

  function ldap_save()
  {
    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    $repos  = $this->attributesAccess['FAIrepository']->getArrayValues();
    $done   = array();
    foreach ($repos as $repo) {
      $release = $repo[2];
      if (($repo[4] == 'install') && (!isset($done[$release]))) {
        static::createBranch($repo[7], $release);
        $done[$release] = TRUE;
      }
    }

    return $errors;
  }

  static function createBranch ($distribution, $release)
  {
    global $config;

    // Check if FAI branch is here
    $fai  = "ou=$distribution,".get_ou('faiBaseRDN').$config->current['BASE'];
    $ldap = $config->get_ldap_link();
    $ldap->cat($fai, array('dn'));
    if (!$ldap->count()) {
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees($fai);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $fai, LDAP_ADD, get_class()), LDAP_ERROR);
        return;
      }
    }

    // Check if FAI release branch is here
    $dn = "ou=$release,$fai";
    $ldap->cat($dn, array('dn'));
    if (!$ldap->count()) {
      $ldap->cd($dn);
      $ldap->add(array('objectClass' => array('organizationalUnit','FAIbranch'), 'ou' => $release));
      if ($ldap->success()) {
        // Add classes OUs
        foreach (array('Script', 'Hook', 'Template', 'Variable', 'Profile', 'Package', 'Partition') as $type) {
          $ldap->cd($dn);
          $ldap->create_missing_trees(get_ou('fai'.$type.'RDN').$dn);
          if (!$ldap->success()) {
            msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), get_ou('fai'.$type.'RDN').$dn, LDAP_ADD, get_class()), LDAP_ERROR);
          }
        }
      } else {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
      }
    }
  }

  /*!
   * \brief Get all configured repository servers
   *
   * \return array All configured repository servers
  */
  static function getServers()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(FAIrepository=*)(objectClass=FAIrepositoryServer))', array('FAIrepository'));

    $repos = array();
    while ($entry = $ldap->fetch()) {
      if (isset($entry['FAIrepository'])) {
        // Add an entry for each Repository configured for server
        unset($entry['FAIrepository']['count']);
        foreach ($entry['FAIrepository'] as $repo) {
          $tmp = explode("|", $repo);
          $infos['URL']           = $tmp[0];
          $infos['PARENT_SERVER'] = $tmp[1];
          $infos['FAI_RELEASE']   = $tmp[2];
          $infos['SECTIONS']      = (empty($tmp[3]) ? array() : explode(',', $tmp[3]));
          $infos['INSTALL']       = $tmp[4];
          $infos['LOCAL']         = $tmp[5];
          $infos['ARCHS']         = (empty($tmp[6]) ? array() : explode(',', $tmp[6]));
          $infos['DIST']          = (isset($tmp[7]) ? $tmp[7] : 'debian');
          $repos[] = $infos;
        }
      }
    }
    return $repos;
  }

  /*!
   * \brief Get all configured customs releases based on a given release
   *
   * \param string $release The release concerned
   *
   * \return array All configured customs releases based on the given release
  */
  static function getCustomReleases($release)
  {
    $list     = array();
    $servers  = static::getServers();

    foreach ($servers as $server) {
      if (strtolower($server['INSTALL']) == strtolower("custom:$release")) {
        $list[$server['FAI_RELEASE']] = $server['FAI_RELEASE'];
      }
    }

    return array_values($list);
  }

  static function getParentServers($distribution, $dn = NULL)
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=FAIrepositoryServer)(FAIrepository=*|'.$distribution.'|*))', array('dn','cn'));

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
    $ldap->cd('ou='.$distribution.','.get_ou('faiBaseRDN').$config->current['BASE']);
    $ldap->search('(objectClass=FAIbranch)', array('ou'), 'one');

    $list = array();
    while ($release = $ldap->fetch()) {
      $list[] = $release['ou'][0];
    }
    return $list;
  }
}
?>
