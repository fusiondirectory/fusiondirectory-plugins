<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2014-2019 FusionDirectory

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

class ejbcaManagement extends management
{
  public static $skipSnapshots  = TRUE;
  protected $skipCpHandler      = TRUE;

  public $neededAttrs = array('userCertificate;binary' => 1);

  /* Default columns - no description for certificates */
  public static $columns = array(
    array('ObjectTypeColumn', array()),
    array('LinkColumn',       array('attributes' => 'nameAttr',    'label' => 'Name')),
    array('ActionsColumn',    array('label' => 'Actions')),
  );

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('EJBCA'),
      'plTitle'       => _('EJBCA certificates management'),
      'plDescription' => _('Manage EJBCA certificates'),
      'plIcon'        => 'geticon.php?context=applications&icon=ejbca&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 50,
      'plManages'     => array('ejbcaCertificate'),
    );
  }

  protected function configureActions()
  {
    parent::configureActions();

    /* Remove create menu */
    foreach ($this->actions['new']->listActions() as $actionName) {
      unset($this->actionHandlers[$actionName]);
    }
    unset($this->actions['new']);

    /* Add download action */
    $this->registerAction(
      new Action(
        'download',
        _('Download'),
        'geticon.php?context=actions&icon=download&size=16',
        1,
        'downloadEntry',
        array('ejbcaCertificate/ejbcaCertificate/r'),
        FALSE,
        TRUE
      )
    );
  }

  function downloadEntry(array $action)
  {
    $entry = $this->listing->getEntry($action['targets'][0]);
    session::set('binary', $entry['userCertificate;binary']);
    session::set('binarytype', 'octet-stream');
    session::set('binaryfile', $entry['cn'].'.der');
    header('location: getbin.php');
    exit();
  }
}
