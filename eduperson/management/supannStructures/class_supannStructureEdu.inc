<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2020  FusionDirectory

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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannStructureEdu extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('EduOrg'),
      'plDescription' => _('Education Organisation'),
      'plObjectType'  => ['entite', 'etablissement'],
      'plObjectClass' => ['eduOrg'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'organisation' => [
        'name'  => _('Organisation'),
        'attrs' => [
          new StringAttribute(
            _('Legal name'), _('The legal name of this establishment'),
            'eduOrgLegalName', FALSE
          ),
          new URLAttribute(
            _('Home page URI'), _('The URI of this establishment website home page'),
            'eduOrgHomePageURI', FALSE
          ),
          new URLAttribute(
            _('Institution URI'), _('The URI of this establishment institution website'),
            'eduOrgSuperiorURI', FALSE
          ),
          new URLAttribute(
            _('White pages URI'), _('The URI of this establishment white pages'),
            'eduOrgWhitePagesURI', FALSE
          ),
        ]
      ],
    ];
  }
}
