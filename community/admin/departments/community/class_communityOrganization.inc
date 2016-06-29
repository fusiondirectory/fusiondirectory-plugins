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
    global $config;
    return array(
      'membership' => array(
        'name'  => _('Membership'),
        'attrs' => array(
          new SelectAttribute (
            _('Membership type'), _('Membership type of this organization'),
            'fdCommunityMembershipType', FALSE,
            $config->get_cfg_value('communityMembershipTypeChoices', array())
          )
        )
      ),
      'dates' => array(
        'name'  => _('Dates'),
        'attrs' => array(
          new GeneralizedTimeDateAttribute (
            _('Start date'), _('Date of the beginning'),
            'fdCommunityStartDate', TRUE
          ),
          new GeneralizedTimeDateAttribute (
            _('End date'), _('Date of the end'),
            'fdCommunityEndDate', FALSE,
            ''
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
          new PostalAddressAttribute(
            _('Address'), _('A postal address'),
            'fdOrganizationAlternateAddress', FALSE
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
}
?>
