<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class mimetypeManagement extends management
{
  var $mime_release = ""; // The currently selected release while in release management mode!

  // Tab definition
  protected $tabClass     = "mimetabs";
  protected $tabType      = "MIMETYPETABS";
  protected $aclCategory  = "mimetypes";
  protected $aclPlugin    = "mimetype";
  protected $objectName   = "mimetype";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Mime types"),
      "plDescription" => _("Mime type management"),
      "plIcon"        => "plugins/systems/images/iconMimetypes.png",
      "plSection"     => "admin",
      "plPriority"    => 22,
      "plCategory"    => array("mimetypes"),

      "plProvidedAcls" => array()
    );
  }

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    $filter_xml = "mimetype-filter.xml";
    $list_xml   = "mimetype-list.xml";
    $this->storagePoints= array(get_ou("mimetypeRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path($filter_xml, TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path($list_xml, TRUE));
    $headpage->setFilter($filter);

    parent::__construct($config, $ui, "mimetypes", $headpage);
  }
}
?>
