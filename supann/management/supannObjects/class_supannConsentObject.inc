<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannConsentObject extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Consent Object'),
      'plDescription' => _('SupAnn Consent Object'),
      'plObjectClass' => ['fdSupannConsentObject'],
      'plFilter'      => '(objectClass=fdSupannConsentObject)',
      'plObjectType'  => [
        'supannConsentObject' => [
          'name'         => _('SupAnn Consent Object'),
          'description'  => _('SupAnn Consent Object'),
          'ou'           => get_ou('SupannConsentRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannConsentObjectName',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannConsentObjectName'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Consent Object'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannConsentRDN')),
          new StringAttribute(
            _('Consent Object Name'), _('The name of the Supann Consent Object'),
            'fdSupannConsentObjectName', TRUE
          ),
          new StringAttribute(
            _('Consent Object Label'), _('The label of the Supann Consent Object'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('Consent Object Description'), _('The description explaining this Supann Consent Object'),
            'description', FALSE
          ),
        ],
      ],
    ];
  }
}
