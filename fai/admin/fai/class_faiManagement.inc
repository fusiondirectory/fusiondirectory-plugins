<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2014-2019  FusionDirectory

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

class faiManagement extends management
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('FAI'),
      'plDescription' => _('Manage FAI software packages and deployment recipes'),
      'plIcon'        => 'geticon.php?context=applications&icon=fai&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 20,
      'plCategory'    => [
        'fai' => [
          'description' => _('FAI'),
          'objectClass' => ['FAIclass']
        ]
      ],
      'plManages'     => ['faiVariable', 'faiHook', 'faiScript', 'faiPackage', 'faiPartitionTable', 'faiTemplate', 'faiProfile'],

      'plProvidedAcls' => []
    ];
  }

  protected function setUpListing ()
  {
    $this->listing  = new faiListing($this);
  }

  function execute (): string
  {
    if ($this->listing->noFaiBranches()) {
      return $this->getHeader().
        '<div class="notice">'.
          '<div class="msgtitle">'.
            '<h2>'.
              '<img src="geticon.php?context=status&icon=dialog-warning&size=32" class="center"  alt="{t}Warning{/t}"/>'.
              _('There are no FAI branches').
            '</h2>'.
          '</div><div>'.
            _(' Please add at least one repository service to create those.').
          '</div>'.
        '</div>';
    }
    return parent::execute();
  }

  static function mainInc ($classname = NULL, $objectTypes = FALSE)
  {
    global $ui;

    parent::mainInc($classname, $objectTypes);

    if ($cleanup) {
      $ui->setCurrentBase(NULL);
    }
  }
}
