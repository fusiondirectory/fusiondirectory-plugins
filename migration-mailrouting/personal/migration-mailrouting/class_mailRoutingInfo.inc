<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2019  FusionDirectory

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

class mailRoutingInfo extends simplePlugin
{
  var $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Mail routing'),
      'plDescription'   => _('Mail routing information'),
      'plSelfModify'    => TRUE,
      'plObjectClass'   => ['inetLocalMailRecipient'],
      'plObjectType'    => ['user'],
      'plDepends'       => ['mailAccount'],
      'plIcon'          => 'geticon.php?context=applications&icon=mail&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=mail&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;
    return [
      'main' => [
        'name'  => _('Routing'),
        'attrs' => [
          new SetAttribute(
            new StringAttribute(
              _('Local addresses'), _('RFC 822 email addresses of this recipient'),
              'mailLocalAddress', FALSE
            )
          ),
          new StringAttribute(
            _('Mail host'), _('FQDN of the MTA that is the final SMTP destination of messages to this recipient'),
            'mailHost', FALSE
          ),
          new MailAttribute(
            _('Routing address'), _('RFC 822 routing address of this recipient'),
            'mailRoutingAddress', FALSE
          ),
        ]
      ],
    ];
  }
}
