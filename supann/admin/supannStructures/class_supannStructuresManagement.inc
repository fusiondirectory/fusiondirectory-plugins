<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannStructuresManagement extends simpleManagement
{
  protected $objectTypes = array('etablissement', 'entite');

  protected $autoFilterAttributes = array('dn', 'ou', 'o', 'description', 'supannCodeEntite', 'supannCodeEntiteParent');

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;

  /* Return plugin information for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('SupAnn structures'),
      'plDescription' => _('SupAnn structures management'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 40,
      'plManages'     => array('etablissement', 'entite'),

      'plProvidedAcls' => array()
    );
  }

  function parseXML ($file)
  {
    $data = parent::parseXML($file);
    $data['list']['table']['layout'] = '|20px;c|||||150px;r|';
    $columns = array (
      array(
        'label'         => 'supannCodeEntite',
        'sortAttribute' => 'supannCodeEntite',
        'sortType'      => 'string',
        'value'         => '%{filter:link(row,dn,"%s",supannCodeEntite)}',
        'export'        => 'true',
      ),
      array(
        'label'         => 'supannCodeEntiteParent',
        'sortAttribute' => 'supannCodeEntiteParent',
        'sortType'      => 'string',
        'value'         => '%{filter:link(row,dn,"%s",supannCodeEntiteParent)}',
        'export'        => 'true',
      ),
    );
    array_splice($data['list']['table']['column'], 2, 0, $columns);
    return $data;
  }
}

?>
