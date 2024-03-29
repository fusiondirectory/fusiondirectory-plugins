<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2017  FusionDirectory

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

class serviceRenaterPartage extends simpleMailMethodService
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Renater Partage'),
      'plDescription'   => _('Renater Partage'),
      'plIcon'          => 'geticon.php?context=applications&icon=renater-partage&size=16',
      'plObjectClass'   => ['fdRenaterPartageServer'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return  [
      'main' => [
        'name'  => _('Settings'),
        'class' => ['fullwidth'],
        'attrs' => [
          new URLAttribute(
            _('URI'), _('URI to contact the Renater Partage API'),
            'fdRenaterPartageServerUri', TRUE
          ),
          new StringAttribute(
            _('User Agent'), _('User Agent to use to contact the Renater Partage API'),
            'fdRenaterPartageServerUserAgent', TRUE,
            'FusionDirectory'
          ),
        ]
      ],
      'domains' => [
        'template'  => get_template_path('renater_domains.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Domains'),
        'class'     => ['fullwidth'],
        'attrs'     => [
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Domains handled by this Renater Partage server'),
              'fdRenaterPartageServerMailDomain',
              [
                new StringAttribute(
                  _('Domain'), _('Domain handled by this server'),
                  'renaterPartageDomain', TRUE
                ),
                new StringAttribute(
                  _('Key'), _('Key for this domain'),
                  'renaterPartageDomainKey', TRUE
                ),
                new StringAttribute(
                  _('Classes of service'), _('Possible classes of service for this domain and their ids, separated by commas. Format is cos1Name|cos1Id,cos2Name|cos2Id.'),
                  'renaterPartageDomainCOS', TRUE,
                  '', '',
                  '/^[^,|]+\|[^,|]+(,[^,|]+\|[^,|]+)*$/',
                  'cos1|cos1id,cos2|cos2id'
                )
              ],
              ':',
              '',
              _('Domains')
            ),
            FALSE, [], TRUE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $parent = NULL)
  {
    parent::__construct($dn, $parent);
    $this->attributesAccess['fdRenaterPartageServerMailDomain']->setLinearRendering(FALSE);
  }
}
