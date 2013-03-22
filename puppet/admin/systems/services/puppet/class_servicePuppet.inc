<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2013  FusionDirectory

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

class servicePuppet extends simpleService {
  var $objectclasses = array('puppetServer');

  /* Construction */
  function __construct(&$config, $dn, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('Puppet server'),
        'attrs' => array(
          new SetAttribute (
            new StringAttribute (
              _('Environments'),
              _('Available environments for puppet nodes'),
              'puppetAvailableEnvironment',
              TRUE
            ),
            array('production')
          ),
        )
      ),
    );
  }

  // We also need this function that return some information about the plugin
  static function plInfo ()
  {
    return array(
      "plShortName"       => _("Puppet server"),
      "plDescription"     => _("This service allows you to use a puppet server"),

      "plProvidedAcls"    => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}

?>
