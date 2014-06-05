<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013 FusionDirectory

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

class faiConfig extends simplePlugin
{
  var $objectclasses  = array('fdFaiPluginConf');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('FAI configuration'),
      'plDescription'   => _('FusionDirectory fai plugin configuration'),
      'plSelfModify'    => FALSE,
      'plObjectType'    => array('configuration'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    $attrs = array(
      'main' => array(
        'name'  => _('LDAP tree'),
        'attrs' => array(
          new StringAttribute (
            _('Fai base RDN'), _('Branch in which fai branches will be stored'),
            'fdFaiBaseRDN', TRUE,
            'ou=fai,ou=configs,ou=systems'
          ),
        )
      ),
    );
    $rdns = array('Script', 'Hook', 'Template', 'Variable', 'Profile', 'Package', 'PartitionTable');
    foreach ($rdns as $rdn) {
      if ($rdn == 'PartitionTable') {
        $default = 'ou=disk';
      } else {
        $default = 'ou='.strtolower($rdn).'s';
      }
      $attrs['main']['attrs'][] = new StringAttribute (
        sprintf(_('Fai %s RDN'), _(strtolower($rdn))),
        sprintf(_('Relative branch in which fai %s will be stored'), _(strtolower($rdn).'s')),
        'fdFai'.$rdn.'RDN', TRUE,
        $default
      );
    }
    return $attrs;
  }
}
?>
