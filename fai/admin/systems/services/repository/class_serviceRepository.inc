<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2012  FusionDirectory

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

class ColonListAttribute extends CompositeAttribute
{
  function __construct($label, $description, $ldapName, $required = FALSE)
  {
    parent::__construct(
      $description, $ldapName."_composite",
      array(
        new SetAttribute(
          new StringAttribute($label, $description, $ldapName)
        )
      ), '', ''
    );
  }

  function readValues($value)
  {
    if ($value == '') {
      return array(array());
    } else {
      return array(explode(',', $value));
    }
  }

  function writeValues($values)
  {
    return join(',', $values[0]);
  }
}

class FAIrepositoryAttribute extends CompositeAttribute
{
  function readValues($value)
  {
    return explode('|', $value);
  }

  function writeValues($values)
  {
    return join('|', $values);
  }
}

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
            new FAIrepositoryAttribute(
              _('Repositories this server hosts'),
              'FAIrepository',
              array(
                new StringAttribute(
                  _('URL'), _(''),
                  'mirrorUrl', TRUE
                ),
                new SelectAttribute(
                  _('Parent server'), _(''),
                  'mirrorParent', FALSE,
                  array(), ''
                ),
                new StringAttribute(
                  _('Release'), _(''), 'mirrorRelease', TRUE
                ),
                new ColonListAttribute(
                  _('Sections'), '', 'mirrorSections'
                ),
                new SelectAttribute(
                  _('Mirror mode'), _(''),
                  'mirrorMode', TRUE,
                  array('install','update','custom')
                ),
                new BooleanAttribute(
                  _('Local mirror'), _(''),
                  'mirrorLocal', FALSE,
                  FALSE, '',
                  'local', 'network'
                ),
                new ColonListAttribute(
                  _('Architectures needed'), _(''), 'mirrorArchs'
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
