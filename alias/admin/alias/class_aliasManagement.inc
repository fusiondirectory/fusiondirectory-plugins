<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2010 Antoine Gallavardin
  Copyright (C) 2012-2016 FusionDirectory

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

class aliasManagement extends simpleManagement
{
  // Tab definition
  protected $objectTypes  = array('mailAliasDistribution','mailAliasRedirection','sympaAlias');

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'aliasExpirationDate');

  /* Return plugin information for acl handling  */
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Aliases'),
      'plDescription' => _('Alias management'),
      'plIcon'        => 'geticon.php?context=applications&icon=alias&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 26,
      'plCategory'    => array('alias' => array('description'  => _('Mail aliases'),
                                                'objectClass'  => array('mailAliasRedirection','mailAliasDistribution','sympaAlias'))),
      'plManages'     => array('mailAliasDistribution','mailAliasRedirection','sympaAlias'),

      'plProvidedAcls' => array()
    );
  }

  function configureHeadpage ()
  {
    $this->headpage->registerElementFilter('filterDate', 'aliasManagement::filterDate');
    parent::configureHeadpage();
  }

  function parseXML ($file)
  {
    $data = parent::parseXML($file);
    $data['list']['table']['layout'] = '|20px;c|||110px;c|100px;r|';
    $data['list']['table']['column'][4] = $data['list']['table']['column'][3];
    $data['list']['table']['column'][3] = array(
      'label'         => 'Expiration date',
      'sortAttribute' => 'aliasExpirationDate',
      'sortType'      => 'integer',
      'value'         => '%{filter:filterDate(aliasExpirationDate)}',
      'export'        => 'true',
    );
    return $data;
  }

  static function filterDate()
  {
    if (func_num_args() == 0) {
      return '&nbsp;';
    }
    $date = func_get_arg(0);
    if (!is_array($date)) {
      return '&nbsp;';
    }
    $dateObject = DateTime::createFromFormat('U', $date[0], new DateTimeZone('UTC'));
    if (is_object($dateObject)) {
      return $dateObject->format('d.m.Y');
    }
    return '&nbsp;';
  }
}
?>
