<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015  FusionDirectory

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

class dnsManagement extends simpleManagement
{
  protected $objectTypes  = array('dnsZone');

  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = TRUE;
  protected $baseMode               = TRUE;

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'relativeDomainName');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS'),
      'plDescription' => _('DNS Management'),
      'plIcon'        => 'geticon.php?context=applications&icon=dns&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 1,
      'plManages'     => array('dnsZone'),

      'plProvidedAcls'  => array()
    );
  }

  function __construct($config, $ui)
  {
    //~ $this->listXMLPath    = get_template_path('dns-list.xml', TRUE, dirname(__FILE__));
    parent::__construct($config, $ui);
  }
}
?>
