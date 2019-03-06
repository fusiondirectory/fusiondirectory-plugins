<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2015-2019 FusionDirectory

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

class auditManagement extends management
{
  protected $skipCpHandler      = TRUE;
  public static $skipSnapshots  = TRUE;

  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn',           []],
    ['LdapGeneralizedTimeColumn',  ['attributes' => 'fdAuditDateTime',    'label' => 'Time']],
    ['LinkColumn',                 ['attributes' => 'fdAuditAuthorDN',    'label' => 'Author']],
    ['LinkColumn',                 ['attributes' => 'fdAuditAction',      'label' => 'Action']],
    ['LinkColumn',                 ['attributes' => 'fdAuditObjectType',  'label' => 'Type']],
    ['LinkColumn',                 ['attributes' => 'fdAuditObject',      'label' => 'Target']],
  ];

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Audit'),
      'plTitle'       => _('Audit events'),
      'plDescription' => _('Browse audit events'),
      'plIcon'        => 'geticon.php?context=applications&icon=audit&size=48',
      'plSection'     => 'reporting',
      'plManages'     => ['auditEvent'],
    ];
  }

  function __construct ()
  {
    parent::__construct();

    $this->filter->addElement(new DateFilterElement($this->filter, 'fdAuditDateTime', _('Date')));
  }

  protected function setUpListing ()
  {
    /* Disable base mode and multi select */
    $this->listing  = new managementListing($this, FALSE, FALSE);
  }

  protected function configureActions ()
  {
    parent::configureActions();

    /* Remove create, remove */
    $removeActions = ['new', 'remove'];
    foreach ($removeActions as $action) {
      foreach ($this->actions[$action]->listActions() as $actionName) {
        unset($this->actionHandlers[$actionName]);
      }
      unset($this->actions[$action]);
    }
    unset($this->actions['removeConfirmed']);

    $this->actions['edit']->setInMenu(FALSE);
  }
}
