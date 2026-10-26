<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2026  FusionDirectory

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

class supannTypeDiplome extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn TypeDiplome'),
      'plDescription' => _('SupAnn TypeDiplome'),
      'plObjectClass' => ['fdSupannTypeDiplome'],
      'plFilter'      => '(objectClass=fdSupannTypeDiplome)',
      'plObjectType'  => [
        'supannTypeDiplome' => [
          'name'         => _('SupAnn TypeDiplome'),
          'description'  => _('SupAnn TypeDiplome'),
          'ou'           => get_ou('SupannTypeDiplomeRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannLabel',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannName', 'fdSupannLabel'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann TypeDiplome'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannTypeDiplomeRDN')),
          new StringAttribute(
            _('TypeDiplome Name'), _('The name of this Supann TypeDiplome'),
            'fdSupannName', TRUE, '', '', '/^{.+?}.+$/', '{PREFIX}VALUE'
          ),
          new StringAttribute(
            _('TypeDiplome Label'), _('The label of this Supann TypeDiplome'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('TypeDiplome Description'), _('The description explaining this Supann TypeDiplome'),
            'description', FALSE
          ),
        ],
      ],
    ];
  }
}
