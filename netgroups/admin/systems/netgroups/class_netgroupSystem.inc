<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2013-2016  FusionDirectory

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

class netgroupSystem extends netgroupMembership {
  protected static $id_attr = 'cn';

  static function plInfo()
  {
    return array(
      'plShortName'     => _('NIS Netgroup'),
      'plDescription'   => _('NIS Netgroup member'),
      'plSelfModify'    => FALSE,
      'plPriority'      => 3,
      'plObjectType'    => array('workstation','terminal','server','printer'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  protected function triple_mask()
  {
    return "/^\(".$this->old_id.",\-?,\S*\)$/";
  }

  protected function triple_data()
  {
    return '('.$this->id.',,)';
  }
}
?>
