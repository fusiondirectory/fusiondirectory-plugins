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

class pureftpdAccount extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Ftp'),
      'plDescription'   => _('Pure ftp account').'&nbsp;('._('Connectivity addon').')',
      'plIcon'          => 'geticon.php?context=applications&icon=pureftpd&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=pureftpd&size=16',
      'plSelfModify'    => TRUE,
      'plPriority'      => 10,
      'plObjectClass'   => ['PureFTPdUser'],
      'plObjectType'    => ['user'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
    // Attributes are grouped by section
      'section1' => [
        'name'  => _('Bandwidth'),
        'attrs' => [
          new IntAttribute(
            _('Upload bandwidth')._(' (kb/sec)'),
            _('Upload bandwidth'),
            'FTPUploadBandwidth',
            FALSE,
            FALSE,
            FALSE,
            ''
          ),
          new IntAttribute(
            _('Download bandwidth')._(' (kb/sec)'),
            _('Download bandwidth in kb/sec'),
            'FTPDownloadBandwidth',
            FALSE,
            FALSE,
            FALSE,
            ''
          )
        ]
      ],
      'section2' => [
        'name'  => _('Ratio'),
        'attrs' => [
          new IntAttribute(
            _('Uploaded files'),
            _('Uploaded files'),
            'FTPUploadRatio',
            FALSE,
            FALSE,
            FALSE,
            ''
          ),
          new IntAttribute(
            _('Downloaded files'),
            _('Downloaded files'),
            'FTPDownloadRatio',
            FALSE,
            FALSE,
            FALSE,
            ''
          )
        ]
      ],
      'section3' => [
        'name' => ('Quota'),
        'attrs' => [
          new IntAttribute(
            _('Files'),
            _('Quota files'),
            'FTPQuotaFiles',
            FALSE,
            FALSE,
            FALSE,
            ''
          ),
          new IntAttribute(
            _('Size')._(' (MB)'),
            _('Files size'),
            'FTPQuotaMBytes',
            FALSE,
            FALSE,
            FALSE,
            ''
          )
        ]
      ],
      'section4' => [
        'name' => _('Miscellaneous'),
        'attrs' => [
          new BooleanAttribute(
            _('Enable FTP access'), _('Enable or disable FTP access'),
            'FTPStatus', FALSE,
            TRUE
          )
        ]
      ]
    ];
  }
}
