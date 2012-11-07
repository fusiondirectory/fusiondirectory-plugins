<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class rSyslogServer extends simpleService{

  var $displayHeader    = TRUE; // If we want the button Add/Delete
  var $objectclasses    = array("goSyslogServer");

  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Logs service"),
          "plDescription" => _("Logs service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plPriority"    => 99,
          "plCategory"    => array("server"),

          "plProvidedAcls"=> parent::generatePlProvidedAcls(self::getAttributesInfo())
          ));
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _("Logging service")." ("._("Services").")",
        'attrs' => array (
          new BooleanAttribute (
            _('Server provides a syslog mysql database'),
            _('Server provides a syslog mysql database'),
            'enableSyslog',
            FALSE,
            FALSE
          ),
          new StringAttribute (
            _('Database'),
            _('Database'),
            'gosaLogDB',
            TRUE,
            'Syslog'
          ),
          new StringAttribute (
            _('Database user'),
            _('Database administrator'),
            'goLogAdmin',
            TRUE,
            'rsyslog'
          ),
          new PasswordAttribute (
            _('Password'),
            _('Password for the database administrator'),
            'goLogPassword',
            TRUE,
            ''
          )
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['enableSyslog']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['enableSyslog']->setManagedAttributes (
      array(
        'erase' => array (
          FALSE => array (
            'section1' => array (
              'gosaLogDB',
              'goLogAdmin',
              'goLogPassword',
            )
          )
        )
      )
    );
  }
}
?>
