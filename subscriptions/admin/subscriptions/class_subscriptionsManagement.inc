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

class subscriptionsManagement extends management
{
  public static $skipTemplates = FALSE;

  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn',          []],
    ['LinkColumn',                ['attributes' => 'nameAttr',                'label' => 'Name']],
    ['LinkColumn',                ['attributes' => 'description',             'label' => 'Description']],
    ['LinkColumn',                ['attributes' => 'fdSubscriptionType',      'label' => 'Type']],
    ['LdapGeneralizedTimeColumn', ['attributes' => 'fdSubscriptionStartDate', 'label' => 'Start date']],
    ['LdapGeneralizedTimeColumn', ['attributes' => 'fdSubscriptionEndDate',   'label' => 'End date']],
    ['ActionsColumn',             ['label' => 'Actions']],
  ];

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Subscriptions'),
      'plTitle'       => _('Subscription management'),
      'plDescription' => _('Manage subscriptions'),
      'plIcon'        => 'geticon.php?context=applications&icon=subscription&size=48',
      'plSection'     => 'accounts',
      'plManages'     => ['subscription'],

      'plProvidedAcls' => []
    ];
  }
}
