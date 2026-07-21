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

class supannConsentType extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Consent Type'),
      'plDescription' => _('SupAnn Consent Type'),
      'plObjectClass' => ['fdSupannConsentType'],
      'plFilter'      => '(objectClass=fdSupannConsentType)',
      'plObjectType'  => [
        'supannConsentType' => [
          'name'         => _('SupAnn Consent Type'),
          'description'  => _('SupAnn Consent Type'),
          'ou'           => get_ou('SupannConsentRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannConsentTypeName',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannConsentTypeName'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Consent Type'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannConsentRDN')),
          new StringAttribute(
            _('Consent Type Name'), _('The name of the Supann Consent Type'),
            'fdSupannConsentTypeName', TRUE
          ),
          new StringAttribute(
            _('Consent Type Label'), _('The label of the Supann Consent Type'),
            'fdSupannConsentTypeLabel', TRUE
          ),
          new StringAttribute(
            _('Consent Type Description'), _('The description explaining this Supann Consent Type'),
            'description', FALSE
          ),
        ],
      ],
    ];
  }
}
