<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2020-2021 FusionDirectory

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

class subscriptionsConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Subscriptions configuration'),
      'plDescription'   => _('FusionDirectory subscriptions plugin configuration'),
      'plObjectClass'   => ['fdSubscriptionsPluginConf'],
      'plCategory'      => ['configuration'],
      'plObjectType'    => ['smallConfig'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Subscriptions'),
        'attrs' => [
          new StringAttribute(
            _('Subscription RDN'), _('Branch in which subscriptions will be stored'),
            'fdSubscriptionRDN', TRUE,
            'ou=subscriptions'
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Subscription types and their labels'),
              'fdSubscriptionTypes',
              [
                new StringAttribute(
                  _('Type'), _('Subscription type'),
                  'fdSubscriptionTypes_type', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this type'),
                  'fdSubscriptionTypes_label', TRUE
                ),
              ],
              ';',
              '',
              _('Subscription types')
            ),
            FALSE,
            [],
            TRUE
          ),
          new StringAttribute(
            _('Apt repository host'), _('Host for the apt repository, to use in auth.conf'),
            'fdSubscriptionAptHost', FALSE
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Rpm repostiories'),
              'fdSubscriptionRpmRepositories',
              [
                new StringAttribute(
                  _('Repository'), _('Repository URL without scheme, to put after @ in baseurl'),
                  'fdSubscriptionRpmRepositories_repo', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this repository'),
                  'fdSubscriptionRpmRepositories_label', TRUE
                ),
              ],
              ';',
              '',
              _('Rpm repostiories')
            ),
            FALSE,
            [],
            TRUE
          ),
        ],
      ],
    ];
  }
}
