<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2010 Antoine Gallavardin
  Copyright (C) 2012-2019 FusionDirectory

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

class aliasManagement extends management
{
  /* Default columns */
  public static $columns = array(
    array('ObjectTypeColumn',     array()),
    array('LinkColumn',           array('attributes' => 'nameAttr',             'label' => 'Name')),
    array('LinkColumn',           array('attributes' => 'description',          'label' => 'Description')),
    array('UnixTimestampColumn',  array('attributes' => 'aliasExpirationDate',  'label' => 'Expiration date')),
    array('ActionsColumn',        array('label' => 'Actions')),
  );

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Aliases'),
      'plTitle'       => _('Alias management'),
      'plDescription' => _('Manage aliases'),
      'plIcon'        => 'geticon.php?context=applications&icon=alias&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 26,
      'plCategory'    => array('alias' => array('description'  => _('Mail aliases'),
                                                'objectClass'  => array('mailAliasRedirection','mailAliasDistribution'))),
      'plManages'     => array('mailAliasDistribution','mailAliasRedirection'),

      'plProvidedAcls' => array()
    );
  }
}
