<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class serviceSieve extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Sieve'),
      'plDescription'   => _('Sieve').' ('._('Services').')',
      'plIcon'          => 'geticon.php?context=applications&icon=sieve&size=16',
      'plObjectClass'   => ['fdSieveServer'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    return  [
      'sieve' => [
        'name'  => _('Sieve'),
        'attrs' => [
          new CompositeAttribute(
            _('Sieve connect URL'),
            'fdSieveConnect',
            [
              new StringAttribute(
                _('Hostname'), _('Hostname of the Sieve server'),
                'sieveConnectURL', TRUE
              ),
              new IntAttribute(
                _('Port'), _('Port number on which Sieve server should be contacted'),
                'sieveConnectPort', TRUE,
                0 /*min*/, FALSE /*max*/, 4190
              ),
              new SelectAttribute(
                _('Option'), _('Options for contacting Sieve server'),
                'sieveConnectOptions', TRUE,
                ['notls', 'tls', 'ssl'], 'notls'
              ),
            ],
            '/^{(.*):(\\d+)\\/([^\\/]+)}$/',
            '{%s:%d/%s}'
          ),
          new StringAttribute(
            _('Admin user'), _('Sieve server admin user'),
            'fdSieveAdmin', TRUE
          ),
          new StringAttribute(
            _('Password'), _('Admin user password'),
            'fdSievePassword', TRUE
          ),
        ],
      ],
    ];
  }
}
