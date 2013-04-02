<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class rSyslogServer extends simpleService {
  var $objectclasses  = array('goLogDBServer');

  var $displayHeader  = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Logs service'),
      'plDescription' => _('Logs service').' ('._('Services').')',
      'plIcon'          => 'plugins/rsyslog/images/iconMini.png',
      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array (
      'main' => array (
        'name'  => _('Logging service'),
        'attrs' => array (
          new SelectAttribute (
            _('Driver'), _('Driver to use to contact this database.'),
            'goLogDriver', TRUE
          ),
          new StringAttribute (
            _('Database'), _('Database in which syslog stores its informations'),
            'goLogDB', TRUE,
            'Syslog'
          ),
          new StringAttribute (
            _('Database user'), _('Database administrator'),
            'goLogAdmin', TRUE,
            'rsyslog'
          ),
          new PasswordAttribute (
            _('Password'), _('Password for the database administrator'),
            'goLogPassword', TRUE,
            ''
          )
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $drivers = array(
      'fbsql','ibase','mssql','mysql','mysqli','oci8','odbc','pgsql','querysim','sqlite','sqlsrv'
    );
    sort($drivers);
    $this->attributesAccess['goLogDriver']->setChoices(
      //~ array_filter($drivers, function ($d) {return class_exists("MDB2_Driver_$d", FALSE);})
      $drivers
    );
  }
}
?>
