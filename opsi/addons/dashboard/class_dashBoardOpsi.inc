<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2017 FusionDirectory project

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

class dashboardOpsi extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI'),
      'plDescription' => _('Statistics and information about OPSI'),
      'plObjectType'  => array('dashboard'),

      'plProvidedAcls'  => array()
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'stats' => array(
        'name'  => _('Statistics'),
        'attrs' => array(new FakeAttribute('stats')),
        'template' => get_template_path('opsi_stats.tpl', TRUE, dirname(__FILE__)),
      ),
      'profiles' => array(
        'name'  => _('Profiles'),
        'attrs' => array(new FakeAttribute('profiles')),
        'template' => get_template_path('opsi_profiles.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->stats    = $this->main_stats();
    $this->profiles = $this->profile_stats();
  }

  function main_stats ()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Statistics */
    $stats = array(
      array(
        'name'    => _('OPSI servers'),
        'type'    => 'server',
        'filter'  => '(objectClass=opsiServer)',
        'img'   => 'geticon.php?context=devices&icon=server&size=16'
      ),
      array(
        'name'  => _('OPSI clients'),
        'type'    => 'workstation',
        'filter'  => '(objectClass=opsiClient)',
        'img'     => 'geticon.php?context=devices&icon=computer&size=16'
      ),
      array(
        'name'  => _('OPSI groups'),
        'type'    => 'ogroup',
        'filter'  => '(objectClass=opsiClient)',
        'img'     => 'geticon.php?context=types&icon=resource-group&size=16'
      ),
    );

    foreach ($stats as &$stat) {
      try {
        $stat['nb'] = count(objects::ls($stat['type'], NULL, NULL, $stat['filter'], TRUE));
      } catch (FusionDirectoryException $e) {
        $stat['nb'] = 0;
        msg_dialog::display(
          _('Error'),
          sprintf(_('Statistics for OPSI could not be computed because of the following error: %s'), $e->getMessage()),
          ERROR_DIALOG
        );
      }
    }
    unset($stat);

    return $stats;
  }

  function profile_stats ()
  {
    $profiles = objects::ls('opsiProfile', array('cn' => 1, 'fdOpsiServerDn' => 1, 'fdOpsiNetbootProduct' => 1, 'fdOpsiSoftwareList' => '*'), NULL, '', TRUE, 'subtree');

    $id   = 'profileStats';
    $div  = new divSelectBox('rows'.$id);
    $smarty = get_smarty();
    $div->SetHeight(90);
    $div->SetHeaders(array(_('Profile'), _('Systems'), _('Groups')));
    foreach ($profiles as $dn => $profile) {
      $fields = array(
        array('string' => $profile['cn']),
        array('string' => objects::count('workstation', NULL, '(&(objectClass=opsiClient)(fdOpsiProfileDn='.$dn.'))', FALSE)),
        array('string' => objects::count('ogroup', NULL, '(&(objectClass=opsiClient)(fdOpsiProfileDn='.$dn.'))', FALSE)),
      );
      $div->AddEntry($fields);
    }
    return $div->DrawList();
  }
}
?>
