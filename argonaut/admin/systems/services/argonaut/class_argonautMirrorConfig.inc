<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class argonautMirrorConfig extends simpleService
{
  protected static $showActions = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Argonaut Mirror settings'),
      'plDescription'   => _('Argonaut Mirror settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=argonaut-mirror&size=16',
      'plObjectClass'   => array('argonautMirrorConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array (
    'section1' => array (
        'name'  => _('Argonaut mirror settings'),
        'attrs' => array (
          new StringAttribute (
            _('Local Debian mirror directory'),
            '',
            'argonautMirrorDir', FALSE,
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
            'argonautCrawlerPackagesFolder', FALSE,
            '/var/cache/argonaut/packages'
          )
        )
      ),
      'section3' => array (
        'name' => _('Argonaut Repository configuration'),
        'attrs' => array (
          new StringAttribute (
            _('Proxy'),
            _('Specifies the http proxy (like Squid) to use for http and hftp method'),
            'argonautLdap2repProxy', FALSE
          ),
          new StringAttribute (
            _('Log directory'), _('Directory in which logs should be stored'),
            'argonautLdap2repLogDir', FALSE,
            '/var/log/argonaut/'
          ),
          new BooleanAttribute (
            _('Ignore errors'),
            _('Causes debmirror to ignore missing or broken deb and source files but still be pedantic about checking meta files'),
            'argonautLdap2repErrors', FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Source'), _('Include source in the mirror'),
            'argonautLdap2repSource', FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('GPG Check'), _('Fail if the Release.gpg file is missing'),
            'argonautLdap2repGPGCheck', FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('Contents'),
            _('Additionally download Contents.<arch>.gz files (Note that these files can be relatively big and can change frequently)'),
            'argonautLdap2repContents', FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('Verbose'), _('Wether to activate verbose mode'),
            'argonautLdap2repVerbose', FALSE,
            FALSE
          ),
        )
      )
    );
  }
}
?>
