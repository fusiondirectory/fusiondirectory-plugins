<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

/* TODO add a select for archs ? */

class serviceDebianRepository extends simpleService
{
  var $objectclasses = array("FAIrepositoryServer");

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'template'  => get_template_path('fai_repository.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Repositories'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Repositories this server hosts'),
              'FAIrepository',
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
                  _('Release'), _('Debian release used on this repository'), 'mirrorRelease', TRUE
                ),
                new CommaListAttribute(
                  'mirrorSections',
                  new SetAttribute(
                    new StringAttribute(
                      _('Sections'), _('Debian sections available on this repository'), 'mirrorSections_inner'
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
                      _('Architectures'), _('Processor architectures available on this repository'), 'mirrorArchs_inner'
                    )
                  )
                ),
              ),
              '', '' // no formats
            ),
            FALSE,  // no order
            array(),
            TRUE    // edit button
          )
        )
      )
    );
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Debian repository'),
      'plDescription' => _('Debian repository').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=mimetypes&icon=package-x-generic&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->attributesAccess['FAIrepository']->attribute->attributes[1]->setChoices($this->getParentServers());
    $releaseList = array_map(
      function ($a)
      {
        return "custom:$a";
      },
      $this->getReleaseList()
    );
    $releaseChoices = array_merge(array('install', 'update'), $releaseList);
    $this->attributesAccess['FAIrepository']->attribute->attributes[4]->setChoices($releaseChoices);
    $this->attributesAccess['FAIrepository']->setLinearRendering(FALSE);
  }

  function ldap_save($clean = TRUE)
  {
    parent::ldap_save($clean);

    $repos  = $this->attributesAccess['FAIrepository']->getArrayValues();
    $done   = array();
    foreach ($repos as $repo) {
      $release = $repo[2];
      if (($repo[4] == 'install') && (!isset($done[$release]))) {
        self::createBranch($release);
        $done[$release] = TRUE;
      }
    }
  }

  static function createBranch ($release)
  {
    global $config;

    // Check if FAI branch is here
    $fai  = get_ou('faiBaseRDN').$config->current['BASE'];
    $ldap = $config->get_ldap_link();
    $ldap->cat($fai, array('dn'));
    if (!$ldap->count()) {
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees($fai);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $fai, LDAP_ADD, get_class()), LDAP_ERROR);
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

  function getReleaseList ()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->ls("(objectClass=FAIbranch)", get_ou("faiBaseRDN").$this->config->current['BASE'], array("ou"));

    $list = array();
    while ($release = $ldap->fetch()) {
      $list[] = $release['ou'][0];
    }
    return $list;
  }

  function getParentServers()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=FAIrepositoryServer)", array("cn"));

    $ret  = array();
    while ($attr = $ldap->fetch()) {
      if ($attr['cn'][0] == $this->cn) {
        continue;
      }
      $ret[$attr['cn'][0]] = $attr['cn'][0];
    }

    $ret = array_merge($ret, $this->getHookElements());

    asort($ret);
    return $ret;
  }

  /* this funtions calls a defined hook
      and parses all additional serverdata
   */
  function getHookElements()
  {
    $ret = array();
    $cmd = $this->config->search("serviceRepository", "repositoryBranchHook", array('tabs'));
    if (!empty($cmd)) {
      $res  = shell_exec($cmd);
      $res2 = trim($res);
      if (!$res || empty($res2)) {
        msg_dialog::display(_("Error"), msgPool::cmdexecfailed( "repositoryBranchHook",
                                                                $cmd,
                                                                _("Repository service")),
                                                                ERROR_DIALOG);
      } else {
        $tmp = preg_split("/\n/", $res);
        foreach ($tmp as $hook) {
          /* skip empty */
          if (empty($hook)) {
            continue;
          }

          if (preg_match("/;/", $hook)) {
            $hookinfo = explode(";", $hook);
            $ret[$hookinfo[0]] = $hookinfo[0];
          } else {
            $ret[$hook] = $hook;
          }
        }
      }
    }
    return $ret;
  }
}
?>
