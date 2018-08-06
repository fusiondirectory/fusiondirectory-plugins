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
  var $objectclasses  = array('argonautFuseFAIConfig');
  var $showActions    = FALSE;
  var $displayHeader  = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Argonaut Fuse FAI module settings'),
      'plDescription'   => _('Argonaut Fuse FAI module settings'),
      'plCategory'      => array('server'),
      'plObjectType'    => array('fusemodule'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array (
      'fai' => array (
        'name'  => _('FAI'),
        'attrs' => array (
          new IntAttribute (
            _('FAI version'), _('Version of FAI installed on the server'),
            'argonautFuseFaiVersion', TRUE,
            4, 5, 5
          ),
          new StringAttribute (
            _('FAI flags'), _('Flags to pass to FAI'),
            'argonautFuseFaiFlags', FALSE,
            'verbose,sshd,syslogd,createvt,reboot'
          ),
          new StringAttribute (
            _('NFS root'), '',
            'argonautFuseNfsRoot', TRUE,
            '/srv/fai/nfsroot'
          ),
          new StringAttribute (
            _('FAI 4 command line'), _('Command line for FAI 4 - should be "ip=dhcp root=/dev/nfs boot=live union=aufs"'),
            'argonautFuseFai4Cmdline', TRUE,
            'ip=dhcp root=/dev/nfs boot=live union=aufs'
          ),
          new StringAttribute (
            _('FAI 5 command line'), _('Command line for FAI 5 - should be "ip=dhcp rootovl"'),
            'argonautFuseFai5Cmdline', TRUE,
            'ip=dhcp rootovl'
          ),
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    $this->attributesAccess['argonautFuseFaiVersion']->setManagedAttributes(
      array(
        'disable' => array (
          4 => array (
            'argonautFuseFai5Cmdline'
          ),
          5 => array (
            'argonautFuseFai4Cmdline'
          )
        )
      )
    );
  }
}
