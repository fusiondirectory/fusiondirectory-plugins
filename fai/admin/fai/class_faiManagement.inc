<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016  FusionDirectory

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

class faiListing extends listing
{
  private $builded = FALSE;

  function __construct($data)
  {
    parent::__construct($data);
    // Instanciate base selector
    $this->baseSelector = new faiBaseSelector($this->bases, $this->base);
    $this->builded      = TRUE;
  }
  /*!
   * \brief Refresh the bases list
   */
  function refreshBasesList()
  {
    if (!$this->builded) {
      return;
    }
    // Fill internal bases list
    $this->bases = faiManagement::getReleaseList();

    // Populate base selector if already present
    if ($this->baseSelector && $this->baseMode) {
      $this->baseSelector->setBases($this->bases);
      $this->baseSelector->setBase($this->base);
      $this->baseSelector->update(TRUE);
    }
  }
}

class faiBaseSelector extends baseSelector
{
  protected $select;

  function __construct($bases, $base = '')
  {
    parent::__construct($bases, $base);
    $this->select = new SelectAttribute(
      '', '',
      'bs_rebase_'.$this->pid, TRUE,
      array_keys($this->pathMapping), '', array_values($this->pathMapping)
    );
    $this->select->setSubmitForm(TRUE);
  }

  function setBases($bases)
  {
    $this->pathMapping = $bases;

    if (is_object($this->select)) {
      $this->select->setChoices(array_keys($this->pathMapping), array_values($this->pathMapping));
    }
  }

  function update($force = FALSE)
  {
    if (!is_object($this->select)) {
      return TRUE;
    }
    if (isset($_REQUEST['BPID']) && $_REQUEST['BPID'] == $this->pid) {
      $this->select->loadPostValue();
      $this->select->applyPostValue();
    }
    /* Skip if there's no change */
    if ($this->tree && ($this->getBase() == $this->select->getValue()) && !$force) {
      $this->lastState = TRUE;
      return TRUE;
    }
    if ($this->getBase() != $this->select->getValue()) {
      $this->setBase($this->select->getValue());
    }
    $smarty = get_smarty();
    $smarty->assign($this->select->getAcl().'ACL', 'rwcdm');
    $this->tree = $smarty->fetch('eval:'.$this->select->renderFormInput());
    $this->tree .= '<input type="hidden" name="BPID" id="BPID" value="'.$this->pid.'"/>';

    $this->lastState = TRUE;
    return TRUE;
  }
}

class faiManagement extends simpleManagement
{
  // Tab definition
  protected $objectTypes  = array('faiVariable', 'faiHook', 'faiScript', 'faiPackage', 'faiPartitionTable', 'faiTemplate', 'faiProfile');

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  //~ protected $baseMode               = FALSE;

  protected $headpageClass = 'faiListing';

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('FAI'),
      'plDescription' => _('Manage FAI software packages and deployment recipes'),
      'plIcon'        => 'geticon.php?context=applications&icon=fai&size=48',
      'plSection'     => 'systems',
      'plPriority'    => 20,
      'plCategory'    => array('fai' => array('description'  => _('FAI'),
                                              'objectClass'  => array('FAIclass'))),
      'plManages'     => array('faiVariable', 'faiHook', 'faiScript', 'faiPackage', 'faiPartitionTable', 'faiTemplate', 'faiProfile'),

      'plProvidedAcls' => array()
    );
  }

  function execute ()
  {
    if (empty($this->headpage->bases)) {
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

  /*! \brief   Returns a list of all releases for useable for drop down boxes.
   */
  static function getReleaseList()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=FAIbranch)', array('ou','FAIstate'));

    $list = array();

    while ($release = $ldap->fetch()) {
      preg_match('/[^,],ou=([^,]+),/', $release['dn'], $m);
      $list[$release['dn']] = $m[1].'/'.$release['ou'][0];
    }

    return $list;
  }
}
?>
