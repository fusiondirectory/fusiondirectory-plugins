<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class printGeneric extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses  = array('fdPrinter', 'ipHost', 'ieee802Device');

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Printer'),
      'plDescription' => _('Printer information'),
      'plPriority'    => 10,
      'plObjectType'  => array('printer' => 
        array(
          'name'        => _('Printer'),
          'description' => _('Printer'),
          'filter'      => 'objectClass=fdPrinter',
          'icon'        => 'geticon.php?context=devices&icon=printer&size=16',
          'ou'          => get_ou('printerRDN'),
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('printerRDN')),
          new HostNameAttribute (
            _('Name'), _('The name of the printer'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('A short description of the printer'),
            'description', FALSE
          ),
        )
      ),
      'details' => array(
        'name'  => _('Details'),
        'attrs' => array(
          new StringAttribute (
            _('Printer location'), _('The location of the printer'),
            'l', FALSE
          ),
          new StringAttribute (
            _('Printer URL'), _('The URL of the printer'),
            'labeledURI', TRUE
          ),
        )
      ),
      'network' => array(
        'name'      => _('Network settings'),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
        'attrs'     => array(
          new IPAttribute(
            _('IP address'), _('IP address this system uses'),
            'ipHostNumber', TRUE
          ),
          new MacAddressAttribute(
            _('Mac address'), _('Mac address of this system'),
            'macAddress', FALSE
          ),
        ),
      ),
    );
  }
}
?>
