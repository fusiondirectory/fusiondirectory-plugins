<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016 FusionDirectory

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

class ejbcaCertificates extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('EJBCA'),
      'plDescription'   => _('Assign EJBCA certificates to a user'),
      'plFilter'        => '(objectClass=fdEjbcaCertificates)',
      'plSelfModify'    => TRUE,
      'plObjectClass'   => ['fdEjbcaCertificates'],
      'plObjectType'    => ['user','server','workstation','terminal','printer','component','phone','mobilePhone'],
      'plIcon'          => 'geticon.php?context=applications&icon=ejbca&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=ejbca&size=16',
      'plForeignKeys'   => [
        'fdEjbcaCertDN' => 'ejbcaCertificate',
      ],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('EJBCA certs'),
        'class' => ['fullwidth'],
        'attrs' => [
          new EjbcaCertsAttribute(
            '', _('Certificates associated to this object'),
            'fdEjbcaCertDN', TRUE
          )
        ]
      ]
    ];
  }
}
