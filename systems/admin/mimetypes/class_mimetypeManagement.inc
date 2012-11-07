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

class mimetypeManagement extends management
{
  var $plHeadline     = "Mime types";
  var $plDescription  = "Manage mime types";
  var $plIcon  = "plugins/systems/images/mimetypes.png";

  var $mime_release = ""; // The currently selected release while in release management mode!

  // Tab definition
  protected $tabClass = "mimetabs";
  protected $tabType = "MIMETYPETABS";
  protected $aclCategory = "mimetypes";
  protected $aclPlugin   = "mimetype";
  protected $objectName   = "mimetype";

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    if($this->IsReleaseManagementActivated()){
      $this->mime_release = get_ou('mimetypeRDN').$this->config->current['BASE'];
      $filter_xml = "mimetype-filter-release.xml";
      $list_xml = "mimetype-list-release.xml";
      $this->storagePoints= array("");
    }else{
      $filter_xml = "mimetype-filter.xml";
      $list_xml = "mimetype-list.xml";
      $this->storagePoints= array(get_ou("mimetypeRDN"));
    }

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")){
      $filter= session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path($filter_xml, true));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path($list_xml, true));
    $headpage->setFilter($filter);

    // Add copy&paste and snapshot handler.
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->cpHandler = new CopyPasteHandler($this->config);
    }
    if($this->config->get_cfg_value("enableSnapshots") == "TRUE"){
      $this->snapHandler = new SnapshotHandler($this->config);
    }
    parent::__construct($config, $ui, "mimetypes", $headpage);
  }

  function renderList()
  {
    $filter = $this->getFilter();
    $headpage = $this->getHeadpage();
    if($this->IsReleaseManagementActivated()){
      $filter->setComboBoxOptions("RELEASE",$this->getReleaseList());

      if(isset($_POST['RELEASE'])){
        $this->mime_release = get_post('RELEASE');
      }
      $headpage->setBase($this->mime_release);
    }

    $headpage->update();
    $display = $headpage->render();
    return($this->getHeader().$display);
  }

  function getReleaseList($base = "", $prefix ="")
  {
    $list = array();
    if(empty($base)){
      $base = get_ou('mimetypeRDN').$this->config->current['BASE'];
      $list[$base] = "/";
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->ls("(objectClass=FAIbranch)",$base,array("ou","FAIstate"));
    while($release = $ldap->fetch()){
      $list[$release['dn']] = $prefix.$release['ou'][0];
      $list = array_merge($list,$this->getReleaseList($release['dn'],$prefix."&nbsp; "));
    }
    return($list);
  }


  function IsReleaseManagementActivated()
  {
    /* Check if we should enable the release selection */
    $tmp = $this->config->search("faiManagement", "CLASS",array('menu','tabs'));
    if(!empty($tmp) && class_available("faiManagement")){
      return(true);
    }
    return(false);
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
