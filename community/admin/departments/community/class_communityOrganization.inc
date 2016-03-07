<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

class communityOrganization extends simplePlugin
{
  var $objectclasses  = array('fdCommunityOrganization');
  var $displayHeader  = TRUE;

  static function plInfo ()
  {
    return array(
      'plShortName'     => _('Community organization'),
      'plDescription'   => _('Community organization dates and alternate address'),
      'plObjectType'    => array('organization'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'dates' => array(
        'name'  => _('Dates'),
        'attrs' => array(
          new DateAttribute (
            _('Start date'), _('Date of the beginning'),
            'fdContractStartDate', TRUE,
            'YmdHis\Z'
          ),
          new DateAttribute (
            _('End date'), _('Date of the end'),
            'fdContractEndDate', FALSE,
            'YmdHis\Z', ''
          ),
        )
      ),
      'address' => array (
        'name'  => _('Contact'),
        'attrs' => array (
          new StringAttribute(
            _('First name'), _('First name'),
            'fdOrganizationAlternateFirstName', FALSE
          ),
          new StringAttribute(
            _('Last name'), _('Last name'),
            'fdOrganizationAlternateLastName', FALSE
          ),
          new TextAreaAttribute(
            _('Street'), _('Street'),
            'fdOrganizationAlternateStreet', FALSE
          ),
          new StringAttribute(
            _('Zip code'), _('Zip code'),
            'fdOrganizationAlternateZipCode', FALSE
          ),
          new StringAttribute(
            _('City'), _('City'),
            'fdOrganizationAlternateCity', FALSE
          ),
          new StringAttribute(
            _('State'), _('State'),
            'fdOrganizationAlternateState', FALSE
          ),
          new StringAttribute(
            _('Country'), _('Country'),
            'fdOrganizationAlternateCountry', FALSE
          ),
        )
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }
}
?>
