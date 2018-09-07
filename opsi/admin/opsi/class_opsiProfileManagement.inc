<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2018 FusionDirectory

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

class opsiManagement extends simpleManagement
{
  protected $objectTypes  = array('opsiProfile', 'opsiSoftwareList');

  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = TRUE;

  /* Return plugin information for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI'),
      'plDescription' => _('OPSI profile management'),
      'plIcon'        => 'geticon.php?context=applications&icon=opsi&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 25,
      'plManages'     => array('opsiProfile', 'opsiSoftwareList'),

      'plProvidedAcls' => array()
    );
  }

  function configureHeadpage ()
  {
    $this->headpage->registerElementFilter('filterUsage', 'opsiManagement::filterUsage');
    parent::configureHeadpage();
  }

  function parseXML ($file)
  {
    $data = parent::parseXML($file);
    $data['list']['table']['layout'] = '|20px;c|||110px;c|150px;r|';
    $data['list']['table']['column'][4] = $data['list']['table']['column'][3];
    $data['list']['table']['column'][3] = array(
      'label'         => 'Usage',
      'value'         => '%{filter:filterUsage(objectType,dn)}',
      'export'        => 'true',
    );
    return $data;
  }

  static function filterUsage($objectType, $dn)
  {
    if (empty($dn) || (strcasecmp($objectType, 'opsiProfile') != 0)) {
      return '&nbsp;';
    }
    $systems  = objects::count('workstation', NULL, '(&(objectClass=opsiClient)(fdOpsiProfileDn='.$dn.'))', FALSE);
    $groups   = objects::count('ogroup',      NULL, '(&(objectClass=opsiClient)(fdOpsiProfileDn='.$dn.'))', FALSE);

    if (($systems == 0) && ($groups == 0)) {
      return '&nbsp;';
    } else {
      $result = '';
      if ($systems > 0) {
        $typeInfos = objects::infos('workstation');
        $result .= '<img class="center" src="'.htmlentities($typeInfos['icon'], ENT_COMPAT, 'UTF-8').'" title="'.$typeInfos['name'].'" alt="'.$typeInfos['name'].'"/>&nbsp;'.$systems;
      }
      if ($groups > 0) {
        $typeInfos = objects::infos('ogroup');
        $result .= '<img class="center" src="'.htmlentities($typeInfos['icon'], ENT_COMPAT, 'UTF-8').'" title="'.$typeInfos['name'].'" alt="'.$typeInfos['name'].'"/>&nbsp;'.$groups;
      }
      return $result;
    }
  }
}
?>
