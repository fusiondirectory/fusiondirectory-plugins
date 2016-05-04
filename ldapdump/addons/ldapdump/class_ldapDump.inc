<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class ldapDump extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('LDAP'),
      'plDescription' => _('LDAP Dump'),
      'plObjectType'  => array('special'),
      'plPriority'    => 99,

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'      => _('LDAP Dump'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new FakeAttribute('dump')
        ),
        'template'  => get_template_path('ldapdump.tpl', TRUE, dirname(__FILE__))
      ),
    );
  }

  function __construct($unused, $dn = NULL, $object = NULL)
  {
    parent::__construct($unused, $dn, $object);

    $this->dump = NULL;
  }

  function execute()
  {
    global $config;
    if ($this->dump === NULL) {
      $ui = get_userinfo();
      if (preg_match('/r/', $ui->get_category_permissions($this->acl_base, 'all'))) {
        $ldap = $config->get_ldap_link();
        $ldap->cat($this->dn);
        if ($attrs = $ldap->fetch()) {
          $this->dump = array($this->dumpLdapNode($attrs, $config->current['BASE']));
        } else {
          $this->dump = array();
        }
      } else {
        $this->dump = FALSE;
      }
    }
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    return parent::execute();
  }

  function dumpLdapNode($attrs, $parent_dn = '')
  {
    global $config;
    $node = array(
      'dn'        => $attrs['dn'],
      'name'      => preg_replace('/,'.preg_quote($parent_dn).'$/', '', $attrs['dn']),
      'attrs'     => array(),
      'subnodes'  => array(),
    );
    for ($i = 0; $i < $attrs['count']; $i++) {
      $key                  = $attrs[$i];
      $node['attrs'][$key]  = $attrs[$key];
      unset($node['attrs'][$key]['count']);
    }

    $ldap = $config->get_ldap_link();
    $ldap->cd($attrs['dn']);
    $ldap->ls();
    while ($attrs = $ldap->fetch()) {
      $node['subnodes'][$attrs['dn']] = $this->dumpLdapNode($attrs, $node['dn']);
    }

    return $node;
  }

  function check()
  {
  }

  function save()
  {
  }
}

?>
