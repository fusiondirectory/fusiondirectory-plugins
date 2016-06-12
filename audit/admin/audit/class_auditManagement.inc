<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016 FusionDirectory

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

class auditManagement extends simpleManagement
{
  protected $objectTypes  = array('auditEvent');

  protected $autoFilterAttributes = array(
    'dn','fdAuditDateTime','fdAuditAction','fdAuditAuthorDN','fdAuditObjectType',
    'fdAuditObject','fdAuditAttributes','fdAuditResult');

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;
  protected $autoActions            = FALSE;
  protected $multiSelect            = FALSE;
  protected $skipCpHandler          = TRUE;
  protected $skipSnapHandler        = TRUE;

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Audit'),
      'plDescription' => _('Audit events display'),
      'plIcon'        => 'geticon.php?context=applications&icon=audit&size=48',
      'plSection'     => 'reporting',
      'plManages'     => array('auditEvent'),

      'plProvidedAcls' => array()
    );
  }

  function configureHeadpage ()
  {
    $this->headpage->registerElementFilter('filterDateTime', 'auditManagement::filterDateTime');
    parent::configureHeadpage();
    unset($this->headpage->xmlData['actionmenu']['action'][0]); // create
    unset($this->headpage->xmlData['actionmenu']['action'][1]); // edit
    unset($this->headpage->xmlData['actionmenu']['action'][2]); // remove
    $this->headpage->sortDirection[1] = TRUE;
    /* Make sure remove actions can’t be triggered */
    unset($this->actions['remove']);
    unset($this->actions['removeConfirmed']);
  }

  function parseXML ($file)
  {
    $data = parent::parseXML($file);
    $data['list']['table']['layout'] = '|20px;c||||||50px;r|';
    $columns = array (
      array(
        'label'         => _('Time'),
        'sortAttribute' => 'fdAuditDateTime',
        'sortType'      => 'string',
        'value'         => '%{filter:filterDateTime(pid,row,dn,fdAuditDateTime)}',
        'export'        => 'true',
      ),
      array(
        'label'         => _('Author'),
        'sortAttribute' => 'fdAuditAuthorDN',
        'sortType'      => 'string',
        'value'         => '%{filter:link(row,dn,"%s",fdAuditAuthorDN)}',
        'export'        => 'true',
      ),
      array(
        'label'         => _('Action'),
        'sortAttribute' => 'fdAuditAction',
        'sortType'      => 'string',
        'value'         => '%{filter:link(row,dn,"%s",fdAuditAction)}',
        'export'        => 'true',
      ),
      array(
        'label'         => _('Type'),
        'sortAttribute' => 'fdAuditObjectType',
        'sortType'      => 'string',
        'value'         => '%{filter:link(row,dn,"%s",fdAuditObjectType)}',
        'export'        => 'true',
      ),
      array(
        'label'         => _('Target'),
        'sortAttribute' => 'fdAuditObject',
        'sortType'      => 'string',
        'value'         => '%{filter:link(row,dn,"%s",fdAuditObject)}',
        'export'        => 'true',
      ),
    );
    array_splice($data['list']['table']['column'], 1, count($data['list']['table']['column']), $columns);
    return $data;
  }

  static function filterDateTime()
  {
    $pid    = func_get_arg(0);
    $row    = func_get_arg(1);
    $dn     = func_get_arg(2);
    $date   = func_get_arg(3);
    if (!is_array($date)) {
      return '';
    }
    $dateObject = LdapGeneralizedTime::fromString($date[0]);
    if (is_object($dateObject)) {
      $dateObject->setTimezone(timezone::getDefaultTimeZone());
      return '<a href="?plug='.$_GET['plug'].'&amp;PID='.$pid.'&amp;act=listing_edit_'.$row.'" title="'.$dn.'">'.$dateObject->format('Y-m-d, H:i:s').'</a>';
    }
    return '';
  }
}
?>
