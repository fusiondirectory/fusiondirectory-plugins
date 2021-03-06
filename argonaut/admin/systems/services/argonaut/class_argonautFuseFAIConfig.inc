<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2018  FusionDirectory

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

class argonautFuseFAIConfig extends simplePlugin
{
  protected static $showActions = FALSE;
  protected $displayHeader      = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Argonaut Fuse FAI module settings'),
      'plDescription'   => _('Argonaut Fuse FAI module settings'),
      'plCategory'      => ['server'],
      'plObjectClass'   => ['argonautFuseFAIConfig'],
      'plObjectType'    => ['fusemodule'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return  [
      'fai' => [
        'name'  => _('FAI'),
        'attrs' => [
          new IntAttribute(
            _('FAI version'), _('Version of FAI installed on the server'),
            'argonautFuseFaiVersion', TRUE,
            4, 5, 5
          ),
          new StringAttribute(
            _('FAI flags'), _('Flags to pass to FAI'),
            'argonautFuseFaiFlags', FALSE,
            'verbose,sshd,syslogd,createvt,reboot'
          ),
          new StringAttribute(
            _('NFS root'), '',
            'argonautFuseNfsRoot', TRUE,
            '/srv/fai/nfsroot'
          ),
          new StringAttribute(
            _('FAI 4 command line'), _('Command line for FAI 4 - should be "ip=dhcp root=/dev/nfs boot=live union=aufs"'),
            'argonautFuseFai4Cmdline', TRUE,
            'ip=dhcp root=/dev/nfs boot=live union=aufs'
          ),
          new StringAttribute(
            _('FAI 5 command line'), _('Command line for FAI 5 - should be "ip=dhcp rootovl"'),
            'argonautFuseFai5Cmdline', TRUE,
            'ip=dhcp rootovl'
          ),
          new BooleanAttribute(
            _('Multiple distro mode'), _('This enables a mode for multiple distributions usage which adds the release as a suffix to kernel, initrd and nfsroot in the PXE file'),
            'argonautFuseMultipleReleaseMode'
          ),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    $this->attributesAccess['argonautFuseFaiVersion']->setManagedAttributes(
      [
        'disable' => [
          4 => [
            'argonautFuseFai5Cmdline'
          ],
          5 => [
            'argonautFuseFai4Cmdline'
          ]
        ]
      ]
    );
  }
}
