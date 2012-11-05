<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012  FusionDirectory

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
  var $objectclasses  = array("argonautFuseFAIConfig");
  var $showActions    = FALSE;
  var $displayHeader     = TRUE;

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("Argonaut Fuse FAI module settings"),
      "plDescription"   => _("Argonaut Fuse FAI module settings")." ("._("Services").")",
      "plCategory"      => array("server"),
      "plObjectType"    => array("fusemodule"),
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
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
      )
    );
  }

  function __construct(&$config, $dn, $object = NULL, $attributesInfo = NULL)
  {
    parent::__construct($config, $dn, $object, $attributesInfo);
  }
}
