<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014  FusionDirectory

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

class serviceCentosRepository extends simpleService
{
  var $conflicts      = array('serviceDebianRepository');
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
                new SelectAttribute(
                  _('Release'), _('Centos release used on this repository'),
                  'mirrorRelease', TRUE,
                  array(5,6,7)
                ),
                new HiddenAttribute(
                  'mirrorSections'
                ),
                new SelectAttribute(
                  _('Mirror mode'), _('Is this mirror an installation repository? Is its release a custom one?'),
                  'mirrorMode', TRUE,
                  array('os', 'updates', 'centosplus', 'extras', 'contrib')
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
                    new SelectAttribute(
                      _('Architectures'), _('Processor architectures available on this repository'),
                      'mirrorArchs_inner', TRUE,
                      array('i386', 'x86_64')
                    )
                  )
                ),
                new HiddenAttribute(
                  'distribution', TRUE, 'centos'
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

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Centos repository'),
      'plDescription' => _('Centos repository').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=mimetypes&icon=package-x-generic&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->attributesAccess['FAIrepository']->attribute->attributes[1]->setChoices(serviceDebianRepository::getParentServers('centos', $this->cn));
    $this->attributesAccess['FAIrepository']->setLinearRendering(FALSE);
  }

  function is_this_account($attrs)
  {
    if (parent::is_this_account()) {
      return serviceDebianRepository::isOfDistribution($attrs, 'centos');
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
      if (!isset($done[$release])) {
        FAI::createBranch('centos', $release);
        $done[$release] = TRUE;
      }
    }
  }
}
?>
