<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class sudoManagement extends management
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Sudo'),
      'plTitle'       => _('Sudo management'),
      'plDescription' => _('Manage sudo roles'),
      'plIcon'        => 'geticon.php?context=applications&icon=sudo&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 45,
      'plManages'     => ['sudo'],
    ];
  }

  protected function configureActions ()
  {
    parent::configureActions();
    $this->actions['new']->addAction(
      new Action(
        'new_default',
        _('Default'),
        'geticon.php?context=applications&icon=sudo&size=16',
        0,
        'newEntryDefault',
        ['sudo/sudo/c'],
        TRUE,
        FALSE
      )
    );
  }

  function newEntryDefault (array $action)
  {
    $action['subaction'] = 'SUDO';

    parent::newEntry($action);

    /* This entry will become the default entry */
    $this->tabObject->set_default(TRUE);
  }
}
