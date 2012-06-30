<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class pureftpdAccount extends simplePlugin
{
  /* Definitions */
  var $displayHeader = TRUE;
  var $plHeadline= "FTP";
  var $plDescription= "This does something";
  var $objectclasses= array("PureFTPdUser");


  /* Return plugin informations for acl handling
  #FIME There possibly some attributes that can be combined to one acl. */
  static function plInfo()
  {
    return (array(
          "plShortName"     => _("Ftp"),
          "plDescription"   => _("Pure ftp account")."&nbsp;("._("Connectivity addon").")",
          "plSelfModify"    => TRUE,
          "plDepends"       => array("user"),
          "plPriority"      => 22,                                 // Position in tabs
          "plSection"       => array("personal" => _("My account")),
          "plCategory"      => array("users"),
          "plProvidedAcls"    => parent::generatePlProvidedAcls(self::getAttributesInfo())
          ));
  }

  static function getAttributesInfo ()
  {
    return array(
    // Attributes are grouped by section
      'section1' => array (
        'name'  => _('Bandwidth'),
        'attrs' => array (
          new IntAttribute (
            _('Upload bandwidth'.'(kb/sec)'),
            _('Upload bandwidth'),
            'FTPUploadBandwidth',
            FALSE,
            FALSE,
            FALSE,
            ''
          ),
          new IntAttribute (
            _('Download bandwidth'),
            _('Download bandwidth in kb/sec'),
            'FTPDownloadBandwidth',
            FALSE,
            FALSE,
            FALSE,
            ''
          )
        )
      ),
      'section2' => array (
        'name'  => _('Ratio'),
        'attrs' => array(
          new IntAttribute (
            _('Uploaded / downloaded files'),
            _('Uploaded files'),
            'FTPUploadRatio',
            FALSE,
            FALSE,
            FALSE,
            ''
          ),
          new IntAttribute (
            _('/'),
            _('Downloaded files'),
            'FTPDownloadRatio',
            FALSE,
            FALSE,
            FALSE,
            ''
          )
        )
      ),
      'section3' => array (
        'name' => ('Quota'),
        'attrs' => array (
          new IntAttribute (
            _('Files'),
            _('Quota files'),
            'FTPQuotaFiles',
            FALSE,
            FALSE,
            FALSE,
            ''
          ),
          new IntAttribute (
            _('Size'),
            _('Files size'),
            'FTPQuotaMBytes',
            FALSE,
            FALSE,
            FALSE,
            ''
          )
        )
      ),
      'section' => array (
        'name' => ('Miscellaneous'),
        'attrs' => array (
          new BooleanAttribute (
            _('Temporary disable FTP access'),
            _('Temporary disable FTP access'),
            'FTPStatus',
            FALSE,
            FALSE
          )
        )
      )
    );
  }
}

?>
