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
      'plShortName'     => _('Subscriptions'),
      'plDescription'   => _('FusionDirectory subscriptions plugin configuration'),
      'plObjectClass'   => ['fdSubscriptionsPluginConf'],
      'plObjectType'    => ['configuration'],
      'plIcon'        => 'geticon.php?context=applications&icon=subscriptions&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=subscriptions&size=16',
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Subscriptions'),
        'class' => ['fullwidth'],
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
          new OrderedArrayAttribute(
            new CompositeAttribute(
              _('Apt repositories'),
              'fdSubscriptionAptRepositories',
              [
                new StringAttribute(
                  _('Path'), _('Repository path'),
                  'fdSubscriptionAptRepositories_path', TRUE
                ),
                new StringAttribute(
                  _('Distribution'), _('Distribution name'),
                  'fdSubscriptionAptRepositories_distro', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this repository'),
                  'fdSubscriptionAptRepositories_label', TRUE
                ),
              ],
              '/^([^ ]+) ([^ ;]+);(.+)$/',
              '%s %s;%s',
              '',
              _('Apt repositories')
            ),
            FALSE,
            [],
            TRUE
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Rpm repositories'),
              'fdSubscriptionRpmRepositories',
              [
                new StringAttribute(
                  _('Path'), _('Repository path'),
                  'fdSubscriptionRpmRepositories_path', TRUE
                ),
                new StringAttribute(
                  _('Label'), _('Label shown for this repository'),
                  'fdSubscriptionRpmRepositories_label', TRUE
                ),
              ],
              ';',
                  '',
                  _('Rpm repositories')
            ),
            FALSE,
            [],
            TRUE
          ),
          new StringAttribute(
            _('Repositories host'), _('Host for the repositories, to use in auth.conf'),
            'fdSubscriptionHost', FALSE
          ),
        ],
      ],
    ];
  }
}
