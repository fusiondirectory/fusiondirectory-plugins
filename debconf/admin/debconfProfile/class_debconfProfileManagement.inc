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

class debconfProfileManagement extends simpleManagement
{
  protected $objectTypes  = array('debconfProfile');

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;

  static function plInfo ()
  {
    return array(
      'plShortName'   => _('Debconf'),
      'plDescription' => _('Debconf profile management'),
      'plIcon'        => 'geticon.php?context=applications&icon=debconf&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 21,
      'plManages'     => array('debconfProfile'),

      'plProvidedAcls' => array()
    );
  }

  function __construct ()
  {
    $this->filterXMLPath  = get_template_path('debconfProfile-filter.xml', TRUE, dirname(__FILE__));
    parent::__construct();
  }

  function parseXML ($file)
  {
    $data = parent::parseXML($file);
    $data['list']['table']['column'][1]['sortAttribute']  = 'ou';
    $data['list']['table']['column'][1]['value']          = '%{filter:link(row,dn,"%s",ou)}';
    return $data;
  }
}
?>
