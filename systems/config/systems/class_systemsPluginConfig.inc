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

class systemsPluginConfig extends simplePlugin
{
  var $objectclasses = array('fdSystemsPluginConf');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Systems'),
      'plDescription'   => _('Systems plugin configuration'),
      'plObjectType'    => array('configuration'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'rdns' => array(
        'name'  => _('LDAP tree for systems'),
        'attrs' => array(
          new StringAttribute (
            _('Systems RDN'), _('Branch in which systems will be stored'),
            'fdSystemRDN', TRUE,
            'ou=systems'
          ),
          new StringAttribute (
            _('Server RDN'), _('Branch in which servers will be stored'),
            'fdServerRDN', TRUE,
            'ou=servers,ou=systems'
          ),
          new StringAttribute (
            _('Workstations RDN'), _('Branch in which workstations will be stored'),
            'fdWorkstationRDN', TRUE,
            'ou=workstations,ou=systems'
          ),
          new StringAttribute (
            _('Terminal RDN'), _('Branch in which terminals will be stored'),
            'fdTerminalRDN', TRUE,
            'ou=terminals,ou=systems'
          ),
          new StringAttribute (
            _('Printer RDN'), _('Branch in which printers will be stored'),
            'fdPrinterRDN', TRUE,
            'ou=printers,ou=systems'
          ),
          new StringAttribute (
            _('Component RDN'), _('Branch in which network devices will be stored'),
            'fdComponentRDN', TRUE,
            'ou=netdevices,ou=systems'
          ),
          new StringAttribute (
            _('Phone RDN'), _('Branch in which phones will be stored'),
            'fdPhoneRDN', TRUE,
            'ou=phones,ou=systems'
          ),
          new StringAttribute (
            _('Mobile phone RDN'), _('Branch in which mobile phones will be stored'),
            'fdMobilePhoneRDN', TRUE,
            'ou=mobile,ou=systems'
          ),
        )
      ),
      'config' => array(
        'name'  => _('Miscellaneous'),
        'attrs' => array(
          new OrderedArrayAttribute (
            new CompositeAttribute(
              _('Available encodings for share services'),
              'fdEncodings',
              array(
                new StringAttribute (
                  _('Encoding'), _('The encoding code name'),
                  'encoding_code', TRUE, '', '',
                  '/^[^=]+$/'
                ),
                new StringAttribute (
                  _('Label'), _('The encoding displayed name'),
                  'encoding_label', TRUE, '', '',
                  '/^[^=]+$/'
                ),
              ),
              '/^([^=]*)=([^=]*)$/',
              '%s=%s',
              '',
              _('Encodings')
            ),
            FALSE, /* no order */
            array(
              'UTF-8=UTF-8', 'ISO8859-1=ISO8859-1 (Latin 1)',
              'ISO8859-2=ISO8859-2 (Latin 2)', 'ISO8859-3=ISO8859-3 (Latin 3)',
              'ISO8859-4=ISO8859-4 (Latin 4)', 'ISO8859-5=ISO8859-5 (Latin 5)',
              'cp850=CP850 (Europe)'
            ),
            TRUE /* edit enabled */
          ),
        )
      )
    );
  }
}
?>
