<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011  FusionDirectory

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

class argonautFuseConfig extends simpleService
{
  var $objectclasses  = array("argonautFuseConfig");
  var $showActions    = FALSE;

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("Argonaut Fuse settings"),
      "plDescription"   => _("Argonaut Fuse settings")." ("._("Services").")",
      "plSelfModify"    => FALSE,
      "plDepends"       => array(),
      "plSection"       => array("admin"),
      "plCategory"      => array("server"),
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
      'main' => array (
        'name'  => _('Basic settings'),
        'attrs' => array (
          new StringAttribute (
            _('Default mode'),
            '',
            'argonautFuseDefaultMode',
            TRUE,
            'install'
          ),
          new StringAttribute (
            _('Log directory'),
            _('Directory in which argonaut-fuse will store its log'),
            'argonautFuseLogDir',
            TRUE,
            '/var/log/argonaut'
          ),
        )
      ),
      'tftp' => array (
        'name'  => _('TFTP'),
        'attrs' => array (
          new StringAttribute (
            _('Pxelinux cfg path'),
            _('Path where argonaut-fuse should store pxelinux.cfg'),
            'argonautFusePxelinuxCfg',
            TRUE,
            '/srv/tftp/pxelinux.cfg'
          ),
          new StringAttribute (
            _('Pxelinux cfg static path'),
            _('Path where argonaut-fuse should store pxelinux.static'),
            'argonautFusePxelinuxCfgStatic',
            TRUE,
            '/srv/tftp/pxelinux.static'
          ),
        )
      ),
      'fai' => array (
        'name'  => _('FAI'),
        'attrs' => array (
          new StringAttribute (
            _('Fai flags'),
            '',
            'argonautFuseFaiFlags',
            FALSE,
            'verbose,sshd,syslogd,createvt,reboot'
          ),
          new StringAttribute (
            _('NFS root'),
            '',
            'argonautFuseNfsRoot',
            TRUE,
            '/srv/fai/nfsroot'
          ),
        )
      ),
      'opsi' => array (
        'name'  => _('OPSI'),
        'attrs' => array (
          new StringAttribute (
            _('Opsi admin'),
            '',
            'argonautFuseOpsiAdmin'
          ),
          new PasswordAttribute (
            _('Opsi password'),
            '',
            'argonautFuseOpsiPassword'
          ),
          new StringAttribute (
            _('Opsi server'),
            '',
            'argonautFuseOpsiServer'
          ),
          new StringAttribute (
            _('Opsi lang'),
            '',
            'argonautFuseOpsiLang'
          ),
        )
      ),
      'ltsp' => array (
        'name'  => _('LTSP'),
        'attrs' => array (
          new StringAttribute (
            _('LTSP server'),
            '',
            'argonautFuseLtspServer'
          )
        )
      )
    );
  }
}
?>
