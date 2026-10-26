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

class supannEtablissement extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn Etablissement'),
      'plDescription' => _('SupAnn Etablissement'),
      'plObjectClass' => ['fdSupannEtablissement'],
      'plFilter'      => '(objectClass=fdSupannEtablissement)',
      'plObjectType'  => [
        'supannEtablissement' => [
          'name'         => _('SupAnn Etablissement'),
          'description'  => _('SupAnn Etablissement'),
          'ou'           => get_ou('SupannEtablissementRDN'),
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
        'name'  => _('Supann Etablissement'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannEtablissementRDN')),
          new StringAttribute(
            _('Etablissement Name'), _('The name of this Supann Etablissement'),
            'fdSupannName', TRUE, '', '', '/^{.+?}.+$/', '{PREFIX}VALUE'
          ),
          new StringAttribute(
            _('Etablissement Label'), _('The label of this Supann Etablissement'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('Etablissement Description'), _('The description explaining this Supann Etablissement'),
            'description', FALSE
          ),
        ],
      ],
    ];
  }
}
