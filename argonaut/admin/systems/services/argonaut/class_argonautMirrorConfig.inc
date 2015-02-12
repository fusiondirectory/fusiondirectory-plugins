<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2015  FusionDirectory

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

class argonautMirrorConfig extends simpleService
{
  var $objectclasses  = array("argonautMirrorConfig");
  var $showActions    = FALSE;

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("Argonaut Mirror settings"),
      "plDescription"   => _("Argonaut Mirror settings")." ("._("Services").")",
      "plIcon"          => "plugins/argonaut/images/iconMiniMirrorConfig.png",

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
    'section1' => array (
        'name'  => _('Argonaut mirror settings'),
        'attrs' => array (
          new StringAttribute (
            _('Local debian mirror directory'),
            '',
            'argonautMirrorDir',
            FALSE,
            '/srv/www/debian'
          )
        )
      ),
      'section2' => array (
        'name' => _('Argonaut Debconf Crawler configuration'),
        'attrs' => array (
          new StringAttribute (
            _('Local packages folder'),
            _('Folder in which the crawler will store packages during analysis'),
            'argonautCrawlerPackagesFolder',
            FALSE,
            '/var/cache/argonaut/packages'
          )
        )
      ),
      'section3' => array (
        'name' => _('Argonaut Repository configuration'),
        'attrs' => array (
          new StringAttribute (
            _('Proxy'),
            '',
            'argonautLdap2repProxy',
            FALSE
          ),
          new StringAttribute (
            _('Log directory'), '',
            'argonautLdap2repLogDir', FALSE,
            '/var/log/argonaut/'
          ),
          new BooleanAttribute (
            _('Errors'),
            '',
            'argonautLdap2repErrors',
            FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('Source'),
            '',
            'argonautLdap2repSource',
            FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('GPG Check'),
            '',
            'argonautLdap2repGPGCheck',
            FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Contents'),
            '',
            'argonautLdap2repContents',
            FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Verbose'),
            _('Wether to activate verbose mode'),
            'argonautLdap2repVerbose',
            FALSE,
            FALSE
          ),
        )
      )
    );
  }
}
?>
