<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2010-2012 Antoine Gallavardin
  Copyright (C) 2013-2020 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class SupannCursusAnneeAttribute extends SupannPrefixedSelectAttribute
{
  function __construct ($label, $description, $ldapName, $required, $acl = "")
  {
    $attributes = [
      new SelectAttribute('', '', $ldapName.'_prefix', $required),
      new SelectAttribute('', '', $ldapName.'_content', FALSE)
    ];
    CompositeAttribute::__construct($description, $ldapName, $attributes, '/^{SUPANN}(%askme%|.)(%askme%|\\d*)$/', '{SUPANN}%s%s', $acl, $label);
    $this->setLinearRendering(TRUE);

    $this->attributes[0]->setChoices(
      ['L','M','D','X','B'],
      [_('Licence'),_('Master'),_('Ph.D.'),_('Another class of degree'),_('Post-graduate year')]
    );
    $yearLabels = [
      _('1st year'),_('2nd year'),_('3rd year'),
      _('4th year'),_('5th year'),_('6th year'),
      _('7th year'),_('8th year'),_('9th year'),
    ];
    $this->prefixedChoices = [
      'L' => [range(1, 3),array_slice($yearLabels, 0, 3)],
      'M' => [range(1, 2),array_slice($yearLabels, 0, 2)],
      'D' => [range(1, 9),array_slice($yearLabels, 0, 9)],
      'X' => [range(1, 9),array_slice($yearLabels, 0, 9)],
      'B' => [range(0, 20),range(0, 20)],
    ];
    $this->attributes[0]->setSubmitForm(TRUE);
    $this->supannUpdateSelect();
    $this->setRequired($required);
  }

  protected function supannUpdateSelect ()
  {
    $prefix = $this->attributes[0]->getValue();
    $this->attributes[1]->setChoices($this->prefixedChoices[$prefix][0] ?? [], $this->prefixedChoices[$prefix][1] ?? []);
  }
}
