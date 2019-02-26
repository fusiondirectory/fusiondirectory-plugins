<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017 FusionDirectory

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

class deploymentTimeframe extends simplePlugin
{
  var $displayHeader  = TRUE;

  var $inheritance = ['gosaGroupOfNames' => 'member'];

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Deployment time frame'),
      'plDescription'   => _('Edit deployment time frame'),
      'plObjectClass'   => ['argonautDeploymentOptions'],
      'plObjectType'    => ['workstation','server','terminal','ogroup-dynamic'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Time frames'),
        'attrs' => [
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Time frames in which deployment is authorized'),
              'argonautDeploymentTimeframe',
              [
                new TimeHiAttribute(
                  '', _('Opening time for this frame as HH:MM'),
                  'begin', TRUE
                ),
                new TimeHiAttribute(
                  _('->'), _('Closing time for this frame as HH:MM'),
                  'end', TRUE
                )
              ],
              '-',
              '',
              _('Time frames')
            ),
            FALSE, [], TRUE
          )
        ]
      ],
    ];
  }
}
