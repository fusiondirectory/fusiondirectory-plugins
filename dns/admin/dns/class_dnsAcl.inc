<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2018  FusionDirectory

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

class dnsAcl extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('fdDNSAcl');

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS acl'),
      'plDescription' => _('DNS acl'),
      'plObjectType'  => array('dnsAcl' => array(
        'name'        => _('DNS acl'),
        'filter'      => '(objectClass=fdDNSAcl)',
        'ou'          => get_ou('dnsRDN'),
        'icon'        => 'geticon.php?context=categories&icon=acl&size=16'
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Acl'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('dnsRDN')),
          new HostNameAttribute (
            _('ACL name'), _('Name of this acl'),
            'cn', TRUE
          ),
          new SetAttribute (
            new StringAttribute (
              _('Address match list'), _('The ip address match list for this acl'),
              'fdDNSAclMatchList', FALSE
            )
          )
        )
      )
    );
  }
}
