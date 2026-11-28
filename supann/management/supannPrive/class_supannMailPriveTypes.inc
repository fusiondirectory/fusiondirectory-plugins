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

class supannMailPriveType extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn Mail Prive types'),
      'plDescription' => _('SupAnn Mail Prive types'),
      'plObjectClass' => ['fdSupannPriveMail'],
      'plFilter'      => '(objectClass=fdSupannPriveMail)',
      'plObjectType'  => [
        'supannMailPriveType' => [
          'name'         => _('SupAnn Mail Prive types'),
          'description'  => _('SupAnn Mail Prive types'),
          'ou'           => get_ou('SupannPriveMailRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannName',
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
        'name'  => _('Supann Mail Prive'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('SupannPriveMailRDN')),
          new StringAttribute(
            _('Name'), _('The name of this Supann Mail Prive'),
            'fdSupannName', TRUE,
          ),
          new StringAttribute(
            _('Label'), _('The label of this Supann Mail Prive'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('Description'), _('The description explaining this Supann Mail Prive'),
            'description', FALSE
          ),
        ],
      ],
    ];
  }
}
