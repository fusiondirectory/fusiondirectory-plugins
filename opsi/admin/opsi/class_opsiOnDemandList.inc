<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016 FusionDirectory

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

class opsiOnDemandList extends opsiSoftwareList
{
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI ondemand list'),
      'plDescription' => _('OPSI on demand software list'),
      'plIcon'        => 'geticon.php?context=applications&icon=opsi-on-demand&size=16',
      'plObjectClass' => array('opsiOnDemandList'),
      'plObjectType'  => array('opsiOnDemandList' => array(
        'name'    => _('OPSI ondemand list'),
        'ou'      => get_ou('opsiRDN'),
        'icon'    => 'geticon.php?context=applications&icon=opsi-on-demand&size=16',
      )),
      'plForeignKeys'   => array(
        'fdOpsiServerDn' => 'serverGeneric',
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Infos'),
        'attrs' => array(
          new SubmittingOPSIServerAttribute(
            _('OPSI server'), _('OPSI server to use for deployment'),
            'fdOpsiServerDn', TRUE
          ),
          new HostNameAttribute(
            _('Name'), _('Name of this OPSI profile'),
            'cn', TRUE
          ),
          new BooleanAttribute(
            _('Show details'), _('Show further information to the user'),
            'fdOpsiOnDemandShowDetails', FALSE
          ),
        )
      ),
      'products' => array(
        'name'  => _('Softwares'),
        'attrs' => array(
          new SetAttribute(
            new SelectAttribute(
              _('Localboot products'), _('The localboot products to put in this list'),
              'fdOpsiLocalbootProduct', FALSE
            )
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    simplePlugin::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique(TRUE);
  }

  public function server_changed ($opsi_args = NULL)
  {
    if ($opsi_args === NULL) {
      $opsi_args = array('id','name');
    }
    return parent::server_changed($opsi_args);
  }
}
?>
