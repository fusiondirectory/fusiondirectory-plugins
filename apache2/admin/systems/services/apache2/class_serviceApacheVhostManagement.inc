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

class serviceApacheVhostManagement extends management
{
  var $plHeadline     = "Vhosts";
  var $plDescription  = "Apache vhosts management";
  var $plIcon         = "plugins/systems/images/application.png";

  // Tab definition
  protected $tabClass     = "apachevhosttabs";
  protected $tabType      = "APACHEVHOSTTABS";
  protected $aclCategory  = "serviceApacheVhostEdit";
  protected $aclPlugin    = "serviceApacheVhostEdit";
  protected $objectName   = "serviceApacheVhostEdit";

  function __construct ($config, $ui, $storagePoint = NULL)
  {
    $this->config = $config;
    $this->ui     = $ui;

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$storagePoint,"Storage point");
    session::set('serviceApacheVhostEditDN',$storagePoint.','.$this->config->current['BASE']);
    if (!empty($storagePoint)) {
      $this->storagePoints = array($storagePoint);
    } else {
      trigger_error("Error : did not received any storage point");
    }

    // Build filter
    $filter = new filter(get_template_path("serviceApacheVhostEdit-filter.xml", TRUE, dirname(__FILE__)));
    $filter->setObjectStorage($this->storagePoints);
    $this->setFilter($filter);
    print_r($filter);

    // Build headpage
    $headpage = new listing(get_template_path("serviceApacheVhostEdit-list.xml", TRUE, dirname(__FILE__)));
    $headpage->setFilter($filter);

    parent::__construct($config, $ui, "serviceApacheVhostEdit", $headpage);

    /* We don't want the normal footer, we have our own at the end of the tpl file,
     * that allow us to differenciate plugin saving from service saving */
    $this->skipFooter = TRUE;
  }

  function execute ()
  {
    $this->skipFooter = TRUE;
    return parent::execute();
  }

  function count ()
  {
    $this->getHeadpage()->update();
    return count($this->getHeadpage()->getEntries());
  }

  /*!
   * \brief  Initiates the removal for the given entries
   *           and displays a confirmation dialog.
   *
   * \param  String  $action  The name of the action which was the used as trigger.
   *
   * \param  Array   $target  A list of object dns, which should be affected by this method.
   *
   * \param  Array   $all     A combination of both 'action' and 'target'.
   */
  protected function removeEntryRequested($action = "",$target = array(),$all = array())
  {
    $ret = parent::removeEntryRequested($action, $target, $all);

    // We've at least one entry to delete.
    if (!empty($ret)) {
      $smarty = get_smarty();
      return $smarty->fetch(get_template_path('remove.tpl', TRUE, dirname(__FILE__)));
    }
  }
}
?>
